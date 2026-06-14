# Umiejętność: Modelowanie Fizyki i Architektura C

## Cel Umiejętności
Odpowiadanie na pytania i generowanie kodu związanego z matematycznym modelem odwróconego wahadła na wózku oraz strukturą aplikacji w C.

## Kluczowe Koncepcje do Wdrożenia
1. **Wektor Stanu:** Reprezentacja układu jako $S = [x, \dot{x}, \theta, \dot{\theta}]$ (pozycja wózka, prędkość wózka, kąt wahadła, prędkość kątowa).
2. **Równania Ruchu (Equations of Motion):** Wykorzystanie równań Lagrange'a drugiego rodzaju z uwzględnieniem mas $M$ (wózek), $m$ (wahadło), długości $l$ oraz tarcia.
3. **Integracja Numeryczna:** Implementacja metody Eulera (prostsza) oraz Rungego-Kutty 4. rzędu (RK4 - stabilniejsza dla RL).
4. **Tryb Headless:** Architektura kodu w C umożliwiająca wyłączenie pętli renderowania (np. Raylib/SDL) za pomocą flagi kompilacji (`#ifdef HEADLESS`) lub zmiennej konfiguracyjnej, aby krok fizyki (`physics_step`) wykonywał się czysto w pamięci na potrzeby Pythona.
