# 🚗 Autonomiczny Pojazd z STM32

Projekt semestralny z przedmiotu **Systemy Wbudowane i Mikrokontrolery**  
Autor: Kacper Adam Woszczyło  
Numer indeksu: 21324  
Data rozpoczęcia: 28.04.2025   
Repozytorium zawiera kod, dokumentację oraz materiały projektowe.

---

## 📌 Opis projektu

Projekt zakłada stworzenie modelu pojazdu autonomicznego opartego na mikrokontrolerze STM32, który w trybie półautomatycznym realizuje polecenia operatora, a w pełni automatycznym samodzielnie wybiera trasę i reaguje na otoczenie. Dzięki zestawowi czujników ultradźwiękowych i optycznych pojazd potrafi wykrywać i omijać przeszkody oraz precyzyjnie podążać za namalowaną linią trasy. Sterowanie napędem realizowane jest poprzez generowanie sygnałów PWM, a całość logiki działania opiera się na wbudowanych timerach i (opcjonalnie) systemie zarządzania zadaniami w czasie rzeczywistym. Użytkownik komunikuje się z pojazdem poprzez interfejs UART (np. moduł Bluetooth HC-05), co umożliwia zdalne wydawanie komend, odczyt stanu sensorów oraz zmianę trybu jazdy. Mam pomysł, żeby na płytce STM32F3Discovery zaimplementować diodowy system informacyjny: podczas skrętu w prawo migotałyby diody zamontowane po prawej stronie platformy, a przy skręcie w lewo – analogiczne diody po stronie lewej. 

---

## 🛠️ Zastosowane technologie i narzędzia

- **Mikrokontroler:** STM32F3DISCOVERY
- **IDE:** STM32CubeIDE
- **Programowanie:** C (HAL / LL)
- **Sensory:**
  - HC-SR04 (ultradźwiękowy)
  - Sensory optyczne TCRT5000 (IR)
- **Zasilanie:** Koszyk na 6 baterii typu AA (R6) - 1x6
- **Sterownik silników:** L298N
- **Komunikacja:** UART przez Bluetooth HC-05

---

## ⚙️ Funkcjonalności

- ✅ Napęd sterowany przez PWM z użyciem Timerów
- ✅ Obsługa sensorów ultradźwiękowych (pomiar odległości)
- ✅ Odczyt wartości z sensorów IR (linia / przeszkody) przy użyciu ADC
- ✅ Detekcja kolizji i unikanie przeszkód
- ✅ Sterowanie ruchem przez UART (komendy tekstowe)
- ✅ Zasilanie bateryjne – pełna autonomia
- ✅ Regularne wersjonowanie kodu (min. 1 commit/tydzień)

---

## 📁 Struktura repozytorium


---

## 🔌 Komendy UART

| Komenda | Opis                    |
|--------:|-------------------------|
| `START` | Uruchamia pojazd       |
| `STOP`  | Zatrzymuje pojazd      |
| `LEFT`  | Skręt w lewo           |
| `RIGHT` | Skręt w prawo          |
| `DIST?` | Zwraca odczyt z HC-SR04 |

---

## 🧪 Scenariusze testowe

- [x] Detekcja przeszkody z przodu (sensor HC-SR04)
- [x] Reakcja na białą/czarną linię (IR)
- [x] Komunikacja przez Bluetooth
- [x] Test zasilania bateryjnego
- [x] Sterowanie ruchem w czasie rzeczywistym

---

## 📸 Demo i zdjęcia

- Zdjęcia pojazdu: [`/Media/Zdjęcia`](./Media/Zdjęcia)
- Nagranie testów: [`/Media/Filmy`](./Media/Filmy)

---

## 📄 Dokumentacja

Pełna dokumentacja projektu znajduje się w folderze [`Dokumentacja/`](./Dokumentacja/), w tym:
- Raporty z postępów prac nad pojazdem
- Schematy układów
- Lista komponentów

---

## 📅 Harmonogram pracy

- Tydzień 1–2: Koncepcja i lista komponentów  
- 
- 
- 
- 

---

## 🧠 Wnioski

_(Tutaj uzupełnij po zakończeniu projektu)_

---

## 📬 Kontakt

W razie pytań:
- Email: 21324@student.ans-elblag.pl
- GitHub: https://github.com/kacprow21324

---

**Licencja:** MIT  
