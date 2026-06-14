# Umiejętność: Integracja C-Python i Środowisko Gymnasium

## Cel Umiejętności
Pomoc w budowaniu mostu technologicznego pomiędzy skryptem treningowym w Pythonie a silnikiem fizycznym napisanym w C.

## Kluczowe Koncepcje do Wdrożenia
1. **Kompilacja do Biblioteki Współdzielonej:** Tworzenie plików `.so` (Linux) lub `.dll` (Windows) z kodu C za pomocą `gcc -shared -fPIC`.
2. **Interfejs Ctypes:** Mapowanie typów danych z C (`float*`, `struct`) na typy Pythona oraz poprawne zarządzanie pamięcią.
3. **Wrapper Gymnasium:** Konstrukcja klasy `CartPoleCustomEnv(gym.Env)` w Pythonie:
   - `reset()`: Wywołuje funkcję C czyszczącą stan układu do małego, losowego wychylenia.
   - `step(action)`: Przekazuje akcję (siłę horyzontalną) do funkcji C, pobiera nowy stan, oblicza funkcję nagrody (Reward Function) i sprawdza warunki końca (Done).
4. **Projektowanie Nagrody:** Balansowanie nagrody tak, aby agent dążył do pionu, minimalizując jednocześnie gwałtowne ruchy wózka.
