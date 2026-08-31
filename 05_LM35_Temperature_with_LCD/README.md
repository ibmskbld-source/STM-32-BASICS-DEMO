# 05 - LM35 Temperature Sensor with I2C LCD Display (STM32G431)

## 📌 Concept for Students
The **LM35** is a precision analog integrated circuit temperature sensor whose output voltage is linearly proportional to the Celsius temperature.
* Scale factor: **$10\text{ mV} / ^\circ\text{C}$**
* $0^\circ\text{C} \rightarrow 0\text{ mV}$
* $25^\circ\text{C} \rightarrow 250\text{ mV}$ ($0.25\text{V}$)
* $100^\circ\text{C} \rightarrow 1000\text{ mV}$ ($1.0\text{V}$)

---

## 🧮 Conversion Mathematics
Using the STM32G431 12-bit ADC ($0\text{ to }4095$ range) with a $3.3\text{V}$ ($3300\text{ mV}$) reference:
$$\text{Voltage (mV)} = \frac{\text{ADC\_Average} \times 3300\text{ mV}}{4095}$$
$$\text{Temperature } (^\circ\text{C}) = \frac{\text{Voltage (mV)}}{10\text{ mV}/^\circ\text{C}}$$

---

## 🔌 Hardware Connections

| Component | Component Pin | STM32G431 Pin | Function |
| :--- | :--- | :--- | :--- |
| **LM35** | **Pin 1 (Vs)** | **5V** or **3.3V** | Power Supply |
| **LM35** | **Pin 2 (Vout)**| **PA2** | ADC1 Channel 3 (`ADC1_IN3`) |
| **LM35** | **Pin 3 (GND)** | **GND** | Ground |
| **I2C LCD** | **VCC** | **5V** | Display Power |
| **I2C LCD** | **GND** | **GND** | Ground |
| **I2C LCD** | **SCL** | **PB8** | `I2C1_SCL` (AF4) |
| **I2C LCD** | **SDA** | **PB9** | `I2C1_SDA` (AF4) |

---

## 🖥️ Display Output Example
```
+----------------+
|Temp: 26.8°C    |
|Status: NORMAL  |
+----------------+
```

---

## 🛠️ STM32CubeIDE Setup Steps
1. Create an STM32G431 project.
2. In `.ioc`:
   - Set **PA2** as `ADC1_IN3` (Single-ended, 12-bit).
   - In Connectivity $\rightarrow$ **I2C1**: Set Mode to `I2C` (Pins **PB8 = SCL**, **PB9 = SDA**).
3. Copy `main.c` to `Core/Src/main.c`.
4. Build and flash. Touch the LM35 with your warm finger to watch the temperature rise!
