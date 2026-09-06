# 03 - HW-072 LDR Sensor Automatic Light & Relay Control (STM32G431)

## 📌 Concept for Students
The **HW-072** is a 3-pin photoresistor module with an integrated **LM393 comparator** and an onboard blue sensitivity adjustment potentiometer.
* **VCC:** Power (3.3V or 5V)
* **GND:** Ground
* **DO (Digital Output):** 
  * **Dark Environment** $\rightarrow$ `DO` output is **HIGH (3.3V)**.
  * **Bright Light** $\rightarrow$ `DO` output is **LOW (0V)**.

The sensitivity threshold can be fine-tuned by turning the small blue trimpot on the HW-072 module with a screwdriver.

---

## 🔌 Hardware Pinout (SieCore 32 G431 Board)

| Component | Pin Label | SieCore 32 Board Pin | Physical Location on Board |
| :--- | :--- | :--- | :--- |
| **HW-072 Sensor** | **DO** (Digital Out) | **A1** (`PA1`) | Header **P1**, left column, pin 7 |
| **HW-072 Sensor** | **VCC** | **+3.3** | Header **P5** (right edge) or **+5V** (Header P3) |
| **HW-072 Sensor** | **GND** | **GND** | Header **P1**, bottom pins |
| **Relay Module** | **IN (Signal)** | **A0** (`PA0`) | Header **P1**, right column, pin 6 |
| **Relay Module** | **VCC** | **+5V** | Header **P3** (top STLINK header labeled `+5V`) |
| **Relay Module** | **GND** | **GND** | Header **P1** or **P2** bottom pin |
| **Indicator LEDs** | **Anode (+)** | **A5** / **B7** / **C13** | `A5` on P1, `B7` on P2, `C13` on P1 |

---

## 🧪 Testing with HW-072:
1. Connect the **3 pins of HW-072**:
   - `VCC` $\rightarrow$ **+3.3V** (or +5V)
   - `GND` $\rightarrow$ **GND**
   - `DO` $\rightarrow$ **A1**
2. Notice the two LEDs on the HW-072 module:
   - One is the power LED (always ON).
   - The other is the digital switch indicator LED.
3. Turn the blue potentiometer on the HW-072 until the switch LED just turns OFF under ambient room light.
4. **Cover the LDR with your finger (simulate night):**
   - The HW-072 switch LED turns ON.
   - The relay instantly **clicks ON**!
   - The indicator LEDs (`PA5`, `PB7`, `PC13`) **turn ON**!
5. **Remove your finger into light:**
   - The relay **clicks OFF**!
   - The indicator LEDs **turn OFF**!
