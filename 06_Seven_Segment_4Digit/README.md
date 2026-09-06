# 08 - Single Digit 7-Segment Display Counter Demo (STM32G431)

## 📌 Concept for Students
A single digit 7-segment display (such as **D5611A/B** or **KNOT542R**) consists of 8 LEDs inside a single plastic package:
* 7 Segment LEDs: **a, b, c, d, e, f, g**
* 1 Decimal Point: **dp**
* 2 Common pins: **COM** (Pin 3 and Pin 8, internally tied together).

---

## 🔍 Display Pinout (10-Pin Package)

Facing the **front** of the display with the decimal point at the bottom right:

```text
         10    9    8    7    6
       +-------------------------+
       |  |    |    |    |    |  |
       |      ---- a ----        |
       |     |           |       |
       |     f           b       |
       |     |           |       |
       |      ---- g ----        |
       |     |           |       |
       |     e           c       |
       |     |           |       |
       |      ---- d ----   (dp) |
       |  |    |    |    |    |  |
       +-------------------------+
          1    2    3    4    5
```

---

## 🔌 Hardware Connections (SieCore 32 G431 Board)

### 1. Segment Pins:
| Display Pin | Segment | Board Pin | Physical Location on Header P1 |
| :--- | :--- | :--- | :--- |
| **Pin 7 (Top)** | **a** | **A0** (`PA0`) | Header P1, Right column, Pin 6 |
| **Pin 6 (Top)** | **b** | **A1** (`PA1`) | Header P1, Left column, Pin 7 |
| **Pin 4 (Bottom)** | **c** | **A2** (`PA2`) | Header P1, Right column, Pin 8 |
| **Pin 2 (Bottom)** | **d** | **A3** (`PA3`) | Header P1, Left column, Pin 9 |
| **Pin 1 (Bottom)** | **e** | **A4** (`PA4`) | Header P1, Right column, Pin 10 |
| **Pin 9 (Top)** | **f** | **A5** (`PA5`) | Header P1, Left column, Pin 11 |
| **Pin 10 (Top)**| **g** | **A6** (`PA6`) | Header P1, Right column, Pin 12 |
| **Pin 5 (Bottom)** | **dp**| **A7** (`PA7`) | Header P1, Left column, Pin 13 |

### 2. Common Pin (COM):
* **Pin 3 or Pin 8** $\rightarrow$ Connect to **`GND`** (Header P1 bottom pin) for Common Cathode (5611A / KNOT542R).
* *(If your display is Common Anode 5611B: connect Pin 3/8 to `+3.3V` and connect `B0` to `GND`).*

---

## 🎮 Counter Behavior
1. **Power-On Test (1 second):** Lights up all segments (`8.`) so you can visually verify every segment is connected.
2. **0 to 9 Decimal Counter:** Increments automatically every 1.0 second:
   ```text
   0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 0 ...
   ```
3. **Heartbeat:** The decimal point (`dp`) blinks every second.
