# 05 - LM75 Digital Temperature Sensor (I2C2) with RG1602A-I2C(P) LCD Display (I2C1)

## 📌 Concept for Students
This project demonstrates interfacing two independent I2C peripherals using **two separate hardware I2C controllers** (**I2C1** and **I2C2**) on the **STM32G431**. 

By using separate I2C peripherals:
* **No breadboard is needed!** Each device connects directly to its own dedicated header pins on the STM32 board.
* **RG1602A-I2C(P) LCD** is controlled via **I2C1** on pins `PB6` (SCL) and `PB7` (SDA).
* **LM75 Digital Sensor** is sampled via **I2C2** on pins `PB10` (SCL) and `PB11` (SDA).

---

## 🔌 Hardware Wiring Guide (Direct Pin-to-Pin, No Breadboard)

### 1. RG1602A-I2C(P) LCD Module $\rightarrow$ STM32 (Uses I2C1)
| LCD Pin | STM32 Board Pin | Function |
| :--- | :--- | :--- |
| **VCC** | **5V** (or 3.3V) | Power |
| **GND** | **GND** | Ground |
| **SCL** | **PB6** | `I2C1_SCL` |
| **SDA** | **PB7** | `I2C1_SDA` |

### 2. LM75 Digital Temperature Sensor $\rightarrow$ STM32 (Uses I2C2)
| LM75 Pin | STM32 Board Pin | Function |
| :--- | :--- | :--- |
| **VCC** | **3.3V** (or 5V) | Power |
| **GND** | **GND** (2nd GND pin) | Ground |
| **SCL** | **PB10** | `I2C2_SCL` |
| **SDA** | **PB11** | `I2C2_SDA` |
| **OS, A0..A2** | *Leave Open / Floating* | Not connected |

---

## 🧮 LM75 Data Decoding Formula
Reading from register `0x00` returns 2 consecutive bytes in 2's complement format:
$$\text{raw\_temp} = (\text{Byte}_0 \ll 8) \mid \text{Byte}_1$$
$$\text{Temperature } (^\circ\text{C}) = \frac{\text{raw\_temp (signed 16-bit)}}{256.0}$$

---

## 🖥️ Display Output Example

```
+----------------+
|Temp: 25.50°C   |
|Status: NORMAL  |
+----------------+
```

* **Line 1:** Real-time temperature measurement with high resolution ($0.125^\circ\text{C}$) and degree symbol (`\xDF`).
* **Line 2:** Live status descriptor:
  * `Status: HOT!` ($\ge 35.0^\circ\text{C}$)
  * `Status: WARM` ($\ge 28.0^\circ\text{C}$)
  * `Status: NORMAL` ($18.0^\circ\text{C} \text{ to } 28.0^\circ\text{C}$)
  * `Status: COLD` ($\le 18.0^\circ\text{C}$)

---

## 🛠️ STM32CubeIDE Setup Steps

1. Create a standard STM32 project targeting the **STM32G431KB**.
2. In the `.ioc` Device Configuration Tool:
   - **Connectivity $\rightarrow$ I2C1**: Set Mode to `I2C` (Pins **PB6 = SCL**, **PB7 = SDA**).
   - **Connectivity $\rightarrow$ I2C2**: Set Mode to `I2C` (Pins **PB10 = SCL**, **PB11 = SDA**).
3. Copy `main.c` into `Core/Src/main.c`.
4. Build and flash the microcontroller (`Ctrl + F11`).
5. Touch the LM75 IC with your warm finger to observe live temperature updates!
