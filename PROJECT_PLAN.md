# Plan projektu: Odwrócone wahadło (CartPole) — silnik C + RL w Pythonie

> Dokument referencyjny do samodzielnej implementacji. Każdy etap jest zamknięty i weryfikowalny.
> Stack: **C + Raylib** (gra) · **ctypes / `.so`** (most) · **Python + Gymnasium + własny PPO/DQN w PyTorch** (RL) · **Makefile** (build).

---

## 1. Cel i zasada naczelna

Wirtualne odwrócone wahadło na wózku (CartPole), działające w dwóch trybach z **jednego** silnika fizyki napisanego w C:

1. **Tryb gry (GUI):** klawiszami przesuwasz wózek w poziomie → wahadło się obraca. Interaktywna mini-gra.
2. **Tryb headless (bez animacji):** ten sam silnik skompilowany jako biblioteka `.so`, wołany z Pythona przez `ctypes`, do szybkiego treningu RL bez renderowania.

**Zasada naczelna (nie łam jej):** fizyka jest jednym źródłem prawdy i jest całkowicie oddzielona od renderowania. GUI i headless wołają DOKŁADNIE tę samą `physics_step()`. To, czego nauczy się agent, odpowiada temu, co widać w grze.

**Mechanizm „wyłączania animacji":** nie flaga w runtime, lecz **osobny cel budowania**. `libcartpole.so` nie linkuje Raylib ani kodu renderu — trening fizycznie nie ma dostępu do grafiki, więc nic go nie spowalnia.

---

## 2. Struktura katalogów

```
Inverted_Pendulum/
├── Makefile
├── PROJECT_PLAN.md            # ten plik
├── c_engine/
│   ├── include/
│   │   └── cartpole.h         # struct CartPole, stałe, deklaracje API
│   └── src/
│       ├── physics.c          # RDZEŃ: physics_step, reset, EOM (Euler + RK4)
│       ├── game.c             # tryb GUI: main() + pętla Raylib (TYLKO render + input)
│       ├── api.c              # cienka warstwa C ABI dla ctypes
│       └── test_physics.c     # mały main() do testów fizyki bez GUI
├── python_rl/
│   ├── cartpole_env.py        # CartPoleCustomEnv(gym.Env) — ładuje .so przez ctypes
│   ├── networks.py            # sieci aktora/krytyka (PyTorch)
│   ├── ppo.py                 # własna implementacja PPO (lub dqn.py)
│   ├── train.py              # pętla treningowa, zapis modelu
│   ├── play_policy.py         # podgląd wytrenowanej polityki
│   └── smoke_test.py          # test mostu ctypes vs C
├── build/                     # artefakty: cartpole_game, libcartpole.so, test_physics
└── models/                    # zapisane wagi sieci
```

**Granice warstw (krytyczne):**
- `physics.c` — ZERO Raylib, zero `printf` w pętli kroku. Czysta matematyka.
- `game.c` — zawiera Raylib, NIE liczy fizyki sam (woła `physics.c`).
- `api.c` — tylko adaptery typów dla ctypes, bez logiki.

---

## 3. Model fizyczny

### 3.1 Wektor stanu
```
S = [ x, x_dot, theta, theta_dot ]
```
- `x` — pozycja wózka [m]
- `x_dot` — prędkość wózka [m/s]
- `theta` — kąt wahadła od pionu [rad] (0 = idealnie w górze)
- `theta_dot` — prędkość kątowa [rad/s]

### 3.2 Parametry układu (stałe w struct)
| Symbol | Znaczenie | Wartość startowa |
|---|---|---|
| `M` | masa wózka | 1.0 kg |
| `m` | masa wahadła | 0.1 kg |
| `l` | połowa długości wahadła (do środka masy) | 0.5 m |
| `g` | grawitacja | 9.81 m/s² |
| `dt` | krok czasu | 0.02 s |
| `force_mag` | siła przykładana akcją | 10.0 N |
| (opcjonalnie) `b` | tarcie wózka | 0.0 |

### 3.3 Równania ruchu (Lagrange, klasyczny CartPole)
Wejście: siła pozioma `F`. Oznaczenia: `s = sin(theta)`, `c = cos(theta)`.

```
temp        = ( F + m * l * theta_dot^2 * s ) / (M + m)
theta_ddot  = ( g * s - c * temp ) / ( l * (4/3 - m * c^2 / (M + m)) )
x_ddot      = temp - m * l * theta_ddot * c / (M + m)
```

Pochodna stanu:
```
dS/dt = [ x_dot, x_ddot, theta_dot, theta_ddot ]
```
Zaimplementuj to jako funkcję `void derivatives(const CartPole* cp, const float s[4], float F, float out[4])` — czysta, bez efektów ubocznych. Używają jej zarówno Euler, jak i RK4.

### 3.4 Integracja numeryczna
**Euler (Etap 1, do weryfikacji):**
```
S_next = S + dt * dS/dt
```
**RK4 (Etap 2, domyślny dla RL):**
```
k1 = f(S)
k2 = f(S + dt/2 * k1)
k3 = f(S + dt/2 * k2)
k4 = f(S + dt   * k3)
S_next = S + dt/6 * (k1 + 2*k2 + 2*k3 + k4)
```
Przełącznik metody trzymaj jako pole/enum w struct lub `#define`. RK4 nie „pompuje" energii przy większym `dt` — to zobaczysz w teście.

---

## 4. Kontrakt API w C (`cartpole.h`)

Eksportuj stabilne C ABI (`extern "C"` niepotrzebne w czystym C; pilnuj braku name-manglingu). Proponowane sygnatury:

```c
typedef struct CartPole CartPole;          // nieprzezroczysty wskaźnik dla ctypes

CartPole* cp_create(void);                 // alokuje, ustawia parametry domyślne
void      cp_destroy(CartPole* cp);

void      cp_reset(CartPole* cp, unsigned int seed);   // mały losowy odchył startowy
void      cp_step(CartPole* cp, float force, float dt); // jeden krok integracji
void      cp_get_state(CartPole* cp, float* out4);      // kopiuje [x, x_dot, theta, theta_dot]
int       cp_is_done(CartPole* cp);        // 1 gdy |theta| > próg lub |x| > granica
```

Warunki końca (`cp_is_done`): `|theta| > 12° (≈0.21 rad)` lub `|x| > 2.4 m` (jak klasyczny CartPole — dostosujesz).

---

## 5. Most C ↔ Python (ctypes)

Kompilacja biblioteki:
```
gcc -O2 -fPIC -shared c_engine/src/physics.c c_engine/src/api.c \
    -Ic_engine/include -o build/libcartpole.so -lm
```

W Pythonie (`cartpole_env.py`):
```python
import ctypes
lib = ctypes.CDLL("build/libcartpole.so")

lib.cp_create.restype  = ctypes.c_void_p
lib.cp_step.argtypes   = [ctypes.c_void_p, ctypes.c_float, ctypes.c_float]
lib.cp_get_state.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_float)]
lib.cp_is_done.restype = ctypes.c_int
# ... reszta argtypes/restype

buf = (ctypes.c_float * 4)()
lib.cp_get_state(handle, buf)
state = list(buf)
```
**Zawsze ustawiaj `argtypes`/`restype`** — inaczej ctypes źle interpretuje wskaźniki na 64-bit.

---

## 6. Środowisko RL (Gymnasium)

`CartPoleCustomEnv(gym.Env)`:
- `action_space`: **Discrete(2)** na start (0 = siła w lewo, 1 = w prawo, stała `force_mag`). Prostsze, pasuje do DQN.
- `observation_space`: `Box(low, high, shape=(4,), dtype=float32)` — znormalizowany wektor stanu.
- `reset(seed)` → `cp_reset`, zwraca `(obs, info)`.
- `step(action)` → mapuje akcję na `force`, woła `cp_step`, czyta stan, liczy `reward` i `terminated`, zwraca `(obs, reward, terminated, truncated, info)`.

**Funkcja nagrody (start):** `+1` za każdy krok, w którym wahadło stoi (klasyczny CartPole). Później (Etap 7) możesz przejść na `cos(theta) - 0.01*|x| - 0.001*|force|`, by karać dryf wózka i gwałtowne ruchy.

**`truncated`:** po np. 500 krokach (limit długości epizodu).

---

## 7. Makefile — cele

| Cel | Co robi | Linkuje Raylib? |
|---|---|---|
| `make game` | buduje `build/cartpole_game` (GUI) z `physics.c` + `game.c` | TAK (`-lraylib -lm -lpthread -ldl`) |
| `make lib` | buduje `build/libcartpole.so` (headless) z `physics.c` + `api.c` | NIE |
| `make test` | buduje `build/test_physics` z `physics.c` + `test_physics.c` | NIE |
| `make clean` | usuwa `build/` | — |

Flagi: `-O2 -Wall -Wextra -Ic_engine/include`. Dla `lib` dodatkowo `-fPIC -shared`.

---

## 8. Roadmapa — etapy

**Etap 0 — Szkielet.** Utwórz katalogi, `cartpole.h` (struct + stałe + deklaracje API), pusty Makefile z celami. Kompiluje się „na pusto".

**Etap 1 — Fizyka (Euler).** Zaimplementuj `derivatives()`, `cp_reset()`, `cp_step()` Eulerem. `test_physics.c`: puść wahadło z `theta=0.05` bez siły → powinno opadać; bez nic wózek stoi. Wypisz stan co krok.

**Etap 2 — RK4.** Dodaj RK4 i przełącznik. Test: przy `dt=0.05` Euler zwiększa energię (amplituda rośnie), RK4 jest stabilny.

**Etap 3 — Gra (Raylib).** `game.c`: rysuj tor, wózek (prostokąt), wahadło (linia/kapsuła). Strzałki ←/→ lub A/D przykładają `±force_mag`. `SetTargetFPS(60)`. Pierwszy raz „grasz".

**Etap 4 — Biblioteka + ctypes.** `api.c`, `make lib`, `smoke_test.py`: ten sam ciąg akcji w C (`test_physics`) i w Pythonie (ctypes) → identyczny stan (z dokładnością float).

**Etap 5 — Środowisko Gymnasium.** `cartpole_env.py`. Sprawdź `gymnasium.utils.env_checker.check_env(env)` i przejazd losowego agenta.

**Etap 6 — Algorytm RL (własny).** `networks.py` (MLP aktor/krytyk) + `ppo.py` (zbieranie trajektorii w headless, GAE, clipped objective) lub `dqn.py` (replay buffer, target net, epsilon-greedy). `train.py` loguje średnią nagrodę i zapisuje model do `models/`.

**Etap 7 — Strojenie i podgląd.** Dostrój reward/hiperparametry. `play_policy.py` ładuje model i steruje wersją GUI, by zobaczyć agenta w akcji.

---

## 9. Weryfikacja (jak sprawdzać postęp)

1. **Fizyka:** `make game`, pograj ręcznie — wahadło reaguje sensownie. Euler vs RK4 przy `dt=0.05`: Euler „pompuje" energię, RK4 nie.
2. **Most:** `make lib` + `smoke_test.py` — stan z C i z Pythona identyczny po N krokach.
3. **Środowisko:** `check_env` przechodzi bez ostrzeżeń; losowy agent kończy epizod przy upadku.
4. **Trening:** `python python_rl/train.py` — średnia nagroda rośnie; agent po treningu utrzymuje wahadło dłużej niż losowy.
5. **Wydajność:** headless robi znacząco więcej kroków/s niż GUI (zmierz `time` na 100k krokach) — dowód, że „wyłączanie animacji" działa.

---

## 10. Zależności do zainstalowania

- **C / grafika:** `gcc`, `make`, `raylib` (np. `apt install libraylib-dev` lub build ze źródeł).
- **Python:** `python3`, `numpy`, `gymnasium`, `torch` (PyTorch). Zalecane wirtualne środowisko (`python -m venv .venv`).

---

## 11. Otwarte kwestie (do decyzji w trakcie, nie blokują startu)

- Akcje **dyskretne (DQN)** vs **ciągłe (PPO + Box)** — zacznij dyskretnie.
- Dokładna postać funkcji nagrody — dostroisz w Etapie 7.
- Czy `play_policy.py` steruje binarką GUI, czy robisz osobny lekki podgląd (np. zapis klatek).
- Tarcie wózka/wahadła — start bez, dodasz jeśli zechcesz realizmu.

---

*W razie utknięcia na którymkolwiek etapie — poproś o pomoc punktowo (np. „pokaż jak liczyć GAE w ppo.py" albo „mój RK4 dryfuje, zobacz physics.c"). Implementację piszesz sam; ja pomagam wskazówkami, wzorami i review.*
