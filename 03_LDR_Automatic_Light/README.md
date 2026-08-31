# 03 - LDR Automatic Light & Relay Control (STM32G431)

## 📌 Concept for Students
An **LDR (Light Dependent Resistor / Photoresistor)** changes its electrical resistance based on incoming ambient light:
* **Bright Light** $\rightarrow$ Low Resistance ($\approx 1\text{k}\Omega\text{ to }5\text{k}\Omega$)
* **Darkness** $\rightarrow$ High Resistance ($\approx 100\text{k}\Omega\text{ to }1\text{M}\Omega$)

In a standard sensor module (or a voltage divider with a $10\text{k}\Omega$ resistor), this resistance variation creates a variable analog voltage between **0V** and **3.3V**. The STM32's **12-bit Analog-to-Digital Converter (ADC1)** converts this analog voltage into a digital integer ranging from `0` to `4095`.

---

## 🌓 Automatic Streetlight & Relay Automation Logic
* **Dark Detected (`ADC > 2200`)**:
  - The STM32 turns **ON the Relay on `PA0`** (to energize high-voltage/AC streetlamps).
  - Turns **ON the Visual Indicator LED (`PA5` / `PB8`)**.
* **Bright Light Detected (`ADC < 1600`)**:
  - The STM32 turns **OFF the Relay on `PA0`**.
  - Turns **OFF the Indicator LED**.
* **Hysteresis Band (`1600 - 2200`)**: Prevents rapid clicking of the mechanical relay when ambient light fluctuates near twilight/threshold.
* **8-Sample Software Averaging**: Filters out noise from fluorescent lights and electrical line ripple.

---

## 🔌 Hardware Connections

| Component | Pin Label | STM32G431 Pin | Function |
| :--- | :--- | :--- | :--- |
| **LDR Module** | **VCC** | **3.3V** | Sensor Power |
| **LDR Module** | **GND** | **GND** | Common Ground |
| **LDR Module** | **AO (Analog Output)** | **PA1** | ADC1 Channel 2 (`ADC1_IN2`) |
| **Relay Module**| **IN** | **PA0** | GPIO Output (Active-Low Control) |
| **Relay Module**| **VCC / GND** | **5V / GND** | Relay Coil Power |
| **Indicator LED**| **Anode (+)** | **PA5** / **PB8** | GPIO Output (Onboard LD2) |

---

## 🛠️ STM32CubeIDE Setup Steps
1. Create a new STM32 project for **STM32G431**.
2. In the `.ioc` Pinout & Configuration view:
   - Click **PA1** $\rightarrow$ Set to `ADC1_IN2`.
   - Click **PA0** $\rightarrow$ Set to `GPIO_Output` (Relay).
   - Click **PA5** $\rightarrow$ Set to `GPIO_Output` (LED).
   - In Analog $\rightarrow$ **ADC1**: Enable **IN2 (Single-ended)**, leave resolution at **12-bit**.
3. Copy `main.c` into `Core/Src/main.c`.
4. Build and flash. Cover the LDR with your palm: you will hear the relay click ON and see the LED illuminate!
