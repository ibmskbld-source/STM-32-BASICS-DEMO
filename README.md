# 🎓 STM32G431 Classroom Embedded Systems Demo Suite

Complete, modular, standalone STM32 HAL C code demos for teaching microcontroller peripherals on the **STM32G431** development board.

---

## 📂 Demo Projects Index (01 to 08)

| Folder | Module | Peripheral(s) | Demo Behavior |
| :--- | :--- | :--- | :--- |
| [`01_LED_Blink`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/01_LED_Blink) | **"Hello World" LED Blink** | GPIO Output (`PA5` / `PB8`) | Foundational GPIO demo: blinks onboard user LED LD2 at 1 Hz. |
| [`02_Relay_Module`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/02_Relay_Module) | **1-Channel Relay** | GPIO Output (`PA0`) | Switches relay and synchronized LED ON/OFF every 2 seconds. |
| [`03_LDR_Automatic_Light`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/03_LDR_Automatic_Light) | **LDR Automatic Streetlight** | ADC1 (`PA1`) + GPIO Relay (`PA0`) + LED (`PA5`/`PB8`) | Automatic streetlight: **Dark $\rightarrow$ Relay & LED ON**, **Bright $\rightarrow$ Relay & LED OFF** with 8-sample filtering and anti-flicker hysteresis. |
| [`04_I2C_LCD_Display`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/04_I2C_LCD_Display) | **I2C Alphanumeric LCD** | I2C1 (`PB8`, `PB9`) | Fundamental display demo: welcome screens, course info, and live uptime seconds timer. |
| [`05_LM35_Temperature_with_LCD`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/05_LM35_Temperature_with_LCD) | **LM35 Sensor + LCD** | ADC1 (`PA2`) + I2C1 (`PB8/PB9`) | Reads temperature ($10\text{mV}/^\circ\text{C}$) and displays live `Temp: 26.8°C` and `Status: NORMAL` on I2C LCD. |
| [`06_Matrix_Keypad_with_LCD`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/06_Matrix_Keypad_with_LCD) | **4×4 Keypad + LCD** | GPIO Matrix (`PB0..PB7`) + I2C1 | Interactive keypad typing: displays last pressed key & live typed text on I2C LCD (`*` clears screen). |
| [`07_Parallel_LCD_16x4`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/07_Parallel_LCD_16x4) | **16×4 Parallel LCD** | 4-Bit GPIO (`PC0..PC5`) | Displays 4 lines of classroom info, HD44780 status, and live second counter using direct GPIO. |
| [`08_Seven_Segment_4Digit`](file:///c:/Users/dudet/Downloads/STM32-COLLEGFE/08_Seven_Segment_4Digit) | **4-Digit 7-Segment** | GPIO Multiplexing (`PA4..PA11`, `PB12..PB15`) | Persistence of Vision (POV) demo showing numbers `1234`, text `dE-0`, and live 4-digit counting stopwatch. |

---

## 📌 Master Pin Allocation Map

| Peripheral / Signal | STM32G431 Pin | Mode / Function | Used By |
| :--- | :--- | :--- | :--- |
| **Relay Control (IN)** | `PA0` | GPIO Output Push-Pull | Demo 02, 03 |
| **LDR Analog Input** | `PA1` | ADC1 Channel 2 (`ADC1_IN2`) | Demo 03 |
| **LM35 Analog Input** | `PA2` | ADC1 Channel 3 (`ADC1_IN3`) | Demo 05 |
| **Indicator / Auto LED** | `PA5` / `PB8` | GPIO Output (Onboard LD2) | Demo 01, 02, 03 |
| **7-Segment Lines (A..G, DP)**| `PA4..PA11` | GPIO Output Push-Pull | Demo 08 |
| **Keypad Rows (R1..R4)** | `PB0..PB3` | GPIO Output Push-Pull | Demo 06 |
| **Keypad Columns (C1..C4)** | `PB4..PB7` | GPIO Input with Internal `PULLUP` | Demo 06 |
| **I2C SCL** | `PB8` | `I2C1_SCL` (Alternate Function 4) | Demo 04, 05, 06 |
| **I2C SDA** | `PB9` | `I2C1_SDA` (Alternate Function 4) | Demo 04, 05, 06 |
| **7-Segment Digits (DIG1..DIG4)**| `PB12..PB15`| GPIO Output (Active-Low) | Demo 08 |
| **Parallel LCD Control (RS, EN)**| `PC0, PC1` | GPIO Output Push-Pull | Demo 07 |
| **Parallel LCD Data (D4..D7)** | `PC2..PC5` | GPIO Output Push-Pull | Demo 07 |

---

## 🚀 How to Flash Each Project in STM32CubeIDE (1-by-1)

1. Open **STM32CubeIDE**.
2. Go to **File $\rightarrow$ New $\rightarrow$ STM32 Project**.
3. Target Selection: Select **STM32G431KB** (or your board's MCU).
4. In the Project Explorer, open **`Core/Src/main.c`**.
5. Copy and paste the complete content of the desired demo's `main.c` into `Core/Src/main.c`.
6. Connect your STM32G431 board via USB.
7. Click the green **Run** button (or `Ctrl + F11`) to build and flash!
