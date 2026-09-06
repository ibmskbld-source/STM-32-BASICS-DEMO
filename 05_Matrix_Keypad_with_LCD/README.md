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

## 🔌 Hardware Pinout (SieCore 32 G431 Board)

### 1. Keypad 8-Pin Ribbon Connector (Plugs into Header P1 Left Column, Pins 1 to 8):
| Ribbon Pin | Function | Board Pin | Physical Location on Header P1 |
| :--- | :--- | :--- | :--- |
| **Pin 1 (Left)** | **Row 1** | **C13** (`PC13`) | Header P1, Left column, Pin 1 |
| **Pin 2** | **Row 2** | **C15** (`PC15`) | Header P1, Left column, Pin 2 |
| **Pin 3** | **Row 3** | **F1**  (`PF1`)  | Header P1, Left column, Pin 3 |
| **Pin 4** | **Row 4** | **A1**  (`PA1`)  | Header P1, Left column, Pin 4 |
| **Pin 5** | **Col 1** | **A3**  (`PA3`)  | Header P1, Left column, Pin 5 |
| **Pin 6** | **Col 2** | **A5**  (`PA5`)  | Header P1, Left column, Pin 6 |
| **Pin 7** | **Col 3** | **A7**  (`PA7`)  | Header P1, Left column, Pin 7 |
| **Pin 8 (Right)**| **Col 4** | **B1**  (`PB1`)  | Header P1, Left column, Pin 8 |

### 2. RG1602A-I2C(P) Character LCD Display:
| LCD Pin | Board Pin | Physical Location on Board |
| :--- | :--- | :--- |
| **SCL** | **B2** (`PB2`) | Header **P1**, Right column, Pin 16 (labeled `B2`) |
| **SDA** | **B10** (`PB10`)| Header **P1**, Left column, Pin 17 (labeled `B10`) |
| **VCC** | **+5V** | Header **P3** (top STLINK header labeled `+5V`) |
| **GND** | **GND** | Header **P1** or **P2**, bottom pin (labeled `GND`) |

---

## 🎮 Interactive Features
* **Boot Screen:** Shows `SIECORE32 G431` and `4x4 KEYPAD DEMO`.
* **Line 1:** Shows the currently pressed key (e.g. `Key: ['7']`).
* **Line 2:** Displays typed text (e.g. `Typed: 1234`).
* **Key `*`:** Clears the typed text buffer.
* **Key `#`:** Inserts a space.
