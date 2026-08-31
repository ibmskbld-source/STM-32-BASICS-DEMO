# 07 - 16x4 Character LCD (HD44780 4-Bit Parallel) Demo

## 📌 Concept for Students
The **Hitachi HD44780** controller is the industry-standard chip found inside almost all alphanumeric character LCDs.
* In **8-bit mode**, data is transferred on 8 parallel wires (`D0..D7`).
* In **4-bit mode**, only 4 data lines (`D4..D7`) are used. Each 8-bit command or character byte is split into **two 4-bit nibbles** (High Nibble first, then Low Nibble), saving 4 GPIO pins on the STM32!

---

## 🔌 Hardware Connections

| LCD Pin # | Label | STM32G431 Pin / Connection | Description |
| :--- | :--- | :--- | :--- |
| **Pin 1** | **VSS** | **GND** | Ground |
| **Pin 2** | **VDD** | **5V** | Power Supply |
| **Pin 3** | **V0** | **Potentiometer Wiper** | 10k Pot between 5V & GND for Contrast |
| **Pin 4** | **RS** | **PC0** | Register Select (`0 = Command`, `1 = Data`) |
| **Pin 5** | **RW** | **GND** | Read/Write (`GND = Always Write Mode`) |
| **Pin 6** | **E** | **PC1** | Enable Strobe (Latches data on falling edge) |
| **Pins 7..10**| **D0..D3** | *(Not Connected)* | Left floating in 4-bit mode |
| **Pin 11** | **D4** | **PC2** | Data Line 4 |
| **Pin 12** | **D5** | **PC3** | Data Line 5 |
| **Pin 13** | **D6** | **PC4** | Data Line 6 |
| **Pin 14** | **D7** | **PC5** | Data Line 7 |
| **Pin 15** | **A (LED+)** | **5V (via 220Ω resistor)**| Backlight Anode |
| **Pin 16** | **K (LED-)** | **GND** | Backlight Cathode |

---

## 📺 Demonstration Output (All 4 Lines)
```
+----------------+
|* STM32G431 DEV*|  <- Row 0
|CLASS DEMO 2026 |  <- Row 1
|HD44780 4-BIT OK|  <- Row 2
|READY FOR LAB!  |  <- Row 3
+----------------+
```

---

## 🛠️ STM32CubeIDE Setup Steps
1. In `.ioc`:
   - Set **PC0, PC1, PC2, PC3, PC4, PC5** as `GPIO_Output` (Push-Pull, Low speed, No pull-up/pull-down).
2. Copy `main.c` to `Core/Src/main.c`.
3. Build and flash. (If screen is blank, rotate the 10k contrast potentiometer until clear letters appear).
