# 02 - 1-Channel Relay Module Demo (STM32G431)

## 📌 Concept for Students
Microcontrollers operate at low voltage (**3.3V**) and can only source or sink tiny currents (~20mA). High-voltage or high-current appliances (light bulbs, AC fans, 12V motors) cannot be connected directly to the STM32. 

A **Relay** acts as an electrically operated mechanical switch. It uses a small electromagnet powered by a switching transistor (driven by the STM32 pin) to physically open or close higher-power electrical contacts (**NO = Normally Open**, **NC = Normally Closed**, **COM = Common**).

---

## 🔌 Hardware Connections (SieCore 32 G431 Board)

| Relay Module Pin | SieCore 32 Board Pin | Physical Location on Board |
| :--- | :--- | :--- |
| **IN** (Signal) | **A0** (`PA0`) | Header **P1**, right column, pin 6 (labeled `A0`) |
| **VCC** (Coil Power)| **+5V** (or +3.3V) | Header **P3** (top STLINK header, pin labeled `+5V`) |
| **GND** | **GND** | Header **P1** or **P2**, bottom pin (labeled `GND`) |

*(No onboard LEDs are used in this demo — only the relay is toggled).*

---

## ⚡ Relay Timing Behavior
The firmware continuously cycles the relay on pin `PA0`:
* **State 1 (3 Seconds):** Relay turns **ON** (`PA0 = 0V / LOW`, audible click heard, NO contact closes).
* **State 2 (3 Seconds):** Relay turns **OFF** (`PA0 = 3.3V / HIGH`, audible click heard, NO contact opens).
* Repeated continuously in an infinite loop every 3 seconds.
