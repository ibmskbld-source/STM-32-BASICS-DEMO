# 04 - I2C Character LCD Display (16x2 / 20x4) Demo

## 📌 Concept for Students
A standard alphanumeric LCD requires **6 to 8 GPIO lines** to operate in parallel mode. 

By attaching an **I²C Backpack Module (PCF8574 I/O expander)** to the back of the LCD, the entire display is controlled using only **2 wires (SCL & SDA)** over the **I²C bus**.
* **SCL (Serial Clock):** Synchronizes data transmission.
* **SDA (Serial Data):** Sends address and data bits.
* **Default I2C Address:** Usually `0x27` (or `0x3F` depending on the backpack chip).

---

## 🔌 Hardware Connections

| I2C LCD Backpack Pin | STM32G431 Pin | Mode / Configuration |
| :--- | :--- | :--- |
| **VCC** | **5V** | Display & Backlight Power |
| **GND** | **GND** | Ground |
| **SCL** | **PB8** | `I2C1_SCL` (Alternate Function 4) |
| **SDA** | **PB9** | `I2C1_SDA` (Alternate Function 4) |

> 💡 **Tip:** If the screen turns on but shows no text, adjust the small blue potentiometer (contrast trimmer) on the back of the I2C backpack with a screwdriver.

---

## 📺 Demonstration Sequence in Code
1. **Screen 1 (3 seconds):**
   ```
   +----------------+
   |STM32G431 DEMO  |
   |WELCOME STUDENTS|
   +----------------+
   ```
2. **Screen 2 (3 seconds):**
   ```
   +----------------+
   |EMBEDDED SYSTEMS|
   |I2C LCD RUNNING!|
   +----------------+
   ```
3. **Screen 3 (Continuous):**
   ```
   +----------------+
   |SYSTEM STATUS:OK|
   |Uptime: 00:15 s |
   +----------------+
   ```

---

## 🛠️ STM32CubeIDE Setup Steps
1. In `.ioc`:
   - Under Connectivity $\rightarrow$ **I2C1**: Select **I2C**.
   - Ensure pins are mapped to **PB8 (SCL)** and **PB9 (SDA)** with **I2C Speed Mode = Standard Mode (100kHz)**.
2. Copy `main.c` to `Core/Src/main.c`.
3. Build and flash!
