# 02 - 1-Channel Relay Module Demo (STM32G431)

## 📌 Concept for Students
Microcontrollers operate at low voltage (**3.3V**) and can only source or sink tiny currents (~20mA). High-voltage or high-current appliances (light bulbs, AC fans, 12V motors) cannot be connected directly to the STM32. 

A **Relay** acts as an electrically operated mechanical switch. It uses a small electromagnet powered by a switching transistor (driven by the STM32 pin) to physically open or close higher-power electrical contacts (**NO = Normally Open**, **NC = Normally Closed**, **COM = Common**).

---

## 🔌 Hardware Connections

| Relay Module Pin | STM32G431 Pin | Description |
| :--- | :--- | :--- |
| **VCC** | **5V** (or 3.3V) | Module Power Supply |
| **GND** | **GND** | Ground |
| **IN** | **PA0** | Control Signal (GPIO Output) |
| *(On-board LED)* | **PA5** | Visual Indicator (Synchronized with Relay) |

---

## ⚡ Key Details (Active-Low Logic)
Most standard hobby relay boards use **Active-Low** inputs for noise immunity:
* **`PA0 = LOW (0V)`** $\rightarrow$ Optocoupler/Transistor activates $\rightarrow$ Relay turns **ON** (Click sound heard, NO contact closes).
* **`PA0 = HIGH (3.3V)`** $\rightarrow$ Relay turns **OFF** (NO contact opens).

---

## 🛠️ STM32CubeIDE Setup Steps
1. Create a new STM32 project selecting **STM32G431KB** (or your exact G431 variant).
2. In the Pinout view (`.ioc`):
   - Set **PA0** as `GPIO_Output`.
   - Set **PA5** as `GPIO_Output` (for the onboard LED).
3. Generate Code and copy the code from `main.c` into your project's `Core/Src/main.c`.
4. Build and flash the code.
