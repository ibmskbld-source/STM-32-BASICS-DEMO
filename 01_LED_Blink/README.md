# 01 - "Hello World" LED Blink Demo (STM32G431)

## 📌 Concept for Students
LED Blinking is the foundational introductory experiment in embedded systems. It demonstrates:
1. **Clock Enable:** Powering up the GPIO peripheral clock in the microcontroller (`__HAL_RCC_GPIOA_CLK_ENABLE()`).
2. **Pin Configuration:** Setting the pin mode to **Output Push-Pull** (`GPIO_MODE_OUTPUT_PP`).
3. **State Toggling:** Changing the digital output voltage between **`3.3V` (Logic 1 / HIGH)** and **`0V` (Logic 0 / LOW)** using `HAL_GPIO_TogglePin()`.
4. **Time Delay:** Creating human-visible timing intervals using `HAL_Delay(500)`.

---

## 🔌 Hardware Connections

| Component | Board Pin | Description |
| :--- | :--- | :--- |
| **Onboard User LED (LD2)** | **PA5** (Nucleo-64) or **PB8** (Nucleo-32) | Built right onto the development board (Green LED). |
| *(Optional External LED)* | **PA5** $\rightarrow$ $220\Omega$ resistor $\rightarrow$ LED Anode (+) $\rightarrow$ LED Cathode (-) $\rightarrow$ **GND** | External breadboard LED connection. |

---

## 🛠️ STM32CubeIDE Setup Steps
1. Create a new STM32 project for **STM32G431**.
2. In the `.ioc` Pinout & Configuration view:
   - Click **PA5** $\rightarrow$ Select `GPIO_Output`.
3. In `main.c`, call `HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)` inside the `while(1)` loop with `HAL_Delay(500)`.
4. Build and flash. The green LED will blink ON and OFF every 0.5 seconds!
