# 08 - 4-Digit 7-Segment Display (GPIO Multiplexing) Demo

## 📌 Concept for Students
A 4-digit 7-segment display consists of 4 separate digits, each containing 8 LEDs (7 segment bars $A\text{--}G$ + 1 Decimal Point $DP$). 

Driving all segments independently would require $4 \times 8 = 32$ GPIO pins! 

To save pins, all corresponding segments ($A\text{ to }DP$) across all 4 digits are wired together in parallel. We control which digit lights up using **4 Digit Select pins** via a technique called **Time-Division Multiplexing (Persistence of Vision - POV)**:
1. Turn ON Digit 1 $\rightarrow$ Send Digit 1's segment data $\rightarrow$ Delay 2ms.
2. Turn ON Digit 2 $\rightarrow$ Send Digit 2's segment data $\rightarrow$ Delay 2ms.
3. Turn ON Digit 3 $\rightarrow$ Send Digit 3's segment data $\rightarrow$ Delay 2ms.
4. Turn ON Digit 4 $\rightarrow$ Send Digit 4's segment data $\rightarrow$ Delay 2ms.

Cycling through all 4 digits in $\approx 8\text{ms}$ creates a refresh rate of **$\approx 125\text{Hz}$**—much faster than the human eye can discern, giving the illusion that all 4 digits are steadily lit!

---

## 🔌 Hardware Connections (Common Cathode)

| Display Pin | STM32G431 Pin | Resistor | Description |
| :--- | :--- | :--- | :--- |
| **Segment A** | **PA4** | $220\Omega$ series | Top bar |
| **Segment B** | **PA5** | $220\Omega$ series | Top right bar |
| **Segment C** | **PA6** | $220\Omega$ series | Bottom right bar |
| **Segment D** | **PA7** | $220\Omega$ series | Bottom bar |
| **Segment E** | **PA8** | $220\Omega$ series | Bottom left bar |
| **Segment F** | **PA9** | $220\Omega$ series | Top left bar |
| **Segment G** | **PA10**| $220\Omega$ series | Middle bar |
| **Segment DP**| **PA11**| $220\Omega$ series | Decimal Point |
| **Digit 1 (DIG1)** | **PB12** | Direct | Common Cathode 1 (Active-LOW) |
| **Digit 2 (DIG2)** | **PB13** | Direct | Common Cathode 2 (Active-LOW) |
| **Digit 3 (DIG3)** | **PB14** | Direct | Common Cathode 3 (Active-LOW) |
| **Digit 4 (DIG4)** | **PB15** | Direct | Common Cathode 4 (Active-LOW) |

---

## 📺 Demonstration Output in Code
1. **Pattern 1:** Displays `"1234"` (Number test).
2. **Pattern 2:** Displays `"dE-0"` (Custom text: "Demo").
3. **Pattern 3:** Real-time live 4-digit counter incrementing from `0000` to `9999`.

---

## 🛠️ STM32CubeIDE Setup Steps
1. In `.ioc`:
   - Set **PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11** as `GPIO_Output` (Segments).
   - Set **PB12, PB13, PB14, PB15** as `GPIO_Output` (Digits).
2. Copy `main.c` to `Core/Src/main.c`.
3. Build, flash, and observe smooth multiplexing.
