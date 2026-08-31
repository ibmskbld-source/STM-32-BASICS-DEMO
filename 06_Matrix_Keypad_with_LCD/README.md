# 06 - 4x4 Matrix Keypad with I2C LCD (STM32G431)

## 📌 Concept for Students
Connecting 16 individual push buttons directly to a microcontroller would consume 16 GPIO pins! 

A **Matrix Keypad** arranges the switches in a grid of **4 Rows and 4 Columns** ($4 + 4 = 8$ pins total):
* **4 Rows (R1..R4)** are configured as **GPIO Outputs**.
* **4 Columns (C1..C4)** are configured as **GPIO Inputs with Internal Pull-Ups** (normally reading `HIGH / 1`).

---

## 🔍 How Matrix Scanning Works
1. All Row outputs are kept `HIGH` by default.
2. The STM32 drives **Row 1 LOW (`0V`)** while keeping Rows 2, 3, and 4 `HIGH`.
3. It reads the 4 Columns:
   - If **Column 1** is `LOW` $\rightarrow$ Button at `(Row 1, Col 1)` is pressed (**'1'**).
   - If **Column 2** is `LOW` $\rightarrow$ Button at `(Row 1, Col 2)` is pressed (**'2'**).
   - If **Column 3** is `LOW` $\rightarrow$ Button at `(Row 1, Col 3)` is pressed (**'3'**).
   - If **Column 4** is `LOW` $\rightarrow$ Button at `(Row 1, Col 4)` is pressed (**'A'**).
4. Row 1 is restored to `HIGH`, and the process repeats for Rows 2, 3, and 4 continuously!

---

## 🔌 Hardware Connections

| Component | Pin Label | STM32G431 Pin | Mode / Configuration |
| :--- | :--- | :--- | :--- |
| **Keypad** | **Row 1 (R1)** | **PB0** | GPIO Output Push-Pull |
| **Keypad** | **Row 2 (R2)** | **PB1** | GPIO Output Push-Pull |
| **Keypad** | **Row 3 (R3)** | **PB2** | GPIO Output Push-Pull |
| **Keypad** | **Row 4 (R4)** | **PB3** | GPIO Output Push-Pull |
| **Keypad** | **Col 1 (C1)** | **PB4** | GPIO Input with Internal `PULLUP` |
| **Keypad** | **Col 2 (C2)** | **PB5** | GPIO Input with Internal `PULLUP` |
| **Keypad** | **Col 3 (C3)** | **PB6** | GPIO Input with Internal `PULLUP` |
| **Keypad** | **Col 4 (C4)** | **PB7** | GPIO Input with Internal `PULLUP` |
| **I2C LCD**| **SCL** | **PB8** | `I2C1_SCL` (AF4) |
| **I2C LCD**| **SDA** | **PB9** | `I2C1_SDA` (AF4) |
| **I2C LCD**| **VCC / GND** | **5V / GND** | Power |

---

## 🎮 Interactive Demo Features
* **Line 1:** Shows the currently pressed key (e.g. `Key: 'B'`).
* **Line 2:** Displays typed text (e.g. `Input: 1984`).
* **Pressing `*`:** Clears the input buffer!

---

## 🛠️ STM32CubeIDE Setup Steps
1. In `.ioc`:
   - Set **PB0, PB1, PB2, PB3** as `GPIO_Output`.
   - Set **PB4, PB5, PB6, PB7** as `GPIO_Input` (In Configuration $\rightarrow$ GPIO $\rightarrow$ Set **Pull-up**).
   - In Connectivity $\rightarrow$ **I2C1**: Set Mode to `I2C` (Pins **PB8, PB9**).
2. Copy `main.c` to `Core/Src/main.c`.
3. Build, flash, and type on the keypad!
