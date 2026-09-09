🎓 STM32G431 Basics Demo Suite

Hands-On STM32 Embedded Systems Demonstrations

A collection of six standalone STM32G431 embedded-system demonstrations designed for learning, classroom teaching, laboratory sessions, and hands-on experimentation.

The projects progress from basic GPIO programming to relay control, digital light sensing, I²C temperature monitoring, matrix keypad scanning, LCD interfacing, and 7-segment display control.

All demonstrations are built around the STM32G431 and are intended to be simple enough for beginners while still demonstrating practical embedded-system concepts.

---

🚀 Repository Overview

STM-32-BASICS-DEMO/
│
├── 01_LED_Blink/
├── 02_Relay_Module/
├── 03_LDR_Automatic_Light/
├── 04_LM35_Temperature_with_LCD/
├── 05_Matrix_Keypad_with_LCD/
├── 06_Seven_Segment_4Digit/
│
├── DRIVER/
│
└── README.md

The repository currently contains 6 practical STM32 demonstrations and an "DRIVER" directory containing ST-LINK driver resources.

---

📚 Demo Index

#| Demo| Main Concept| Peripherals
01| 💡 LED Blink| GPIO Output| GPIO
02| 🔌 Relay Module| Actuator Control| GPIO
03| 🌙 LDR Automatic Light| Light Detection & Automation| GPIO
04| 🌡️ LM75 Temperature + LCD| Digital Sensor & Display| I²C1 + I²C2
05| ⌨️ 4×4 Matrix Keypad + LCD| Matrix Scanning & User Input| GPIO + I²C
06| 🔢 7-Segment Counter| Display Control| GPIO

---

01 — 💡 LED Blink

"01_LED_Blink"

The first and simplest demonstration in the collection.

It introduces the fundamental concepts required to control a GPIO pin on the STM32G431:

- GPIO peripheral clock enable
- GPIO output configuration
- Push-pull output
- Digital HIGH / LOW states
- GPIO toggling
- Software delays

The default demonstration toggles the LED approximately every 500 ms. The project documentation supports both the onboard LED configuration and an optional external LED through a resistor.

Hardware

Component| Connection
Onboard LED| "PA5"
External LED| "PA5 → 220Ω → LED → GND"

Basic concept

STM32G431
    │
    └── GPIO PA5
           │
           ▼
          LED
           │
          GND

What you learn

Clock Enable
     ↓
GPIO Configuration
     ↓
Digital Output
     ↓
Toggle
     ↓
Delay

---

02 — 🔌 Relay Module

"02_Relay_Module"

This demonstration introduces actuator control using a 1-channel relay module.

A microcontroller GPIO cannot directly drive high-power electrical loads. A relay provides electrical switching through an isolated mechanical contact system.

The demo uses "PA0" as the relay control signal. On the documented SieCore 32 G431 setup, the relay input is connected to "A0 / PA0".

Hardware Connections

Relay| STM32G431 Board
IN| "PA0 / A0"
VCC| "+5V" or supported supply
GND| "GND"

Operation

The firmware continuously alternates the relay state:

Relay ON
   ↓
3 seconds
   ↓
Relay OFF
   ↓
3 seconds
   ↓
Repeat

The current project uses an active-low relay behavior:

PA0 = LOW   → Relay ON
PA0 = HIGH  → Relay OFF

as documented in the project README.

⚠️ Safety

Do not experiment with mains voltage unless you have appropriate electrical isolation, protection, equipment, and supervision.

For classroom experiments, use low-voltage loads.

---

03 — 🌙 LDR Automatic Light

"03_LDR_Automatic_Light"

This project demonstrates a simple automatic lighting system using an HW-072 LDR/photoresistor module and a relay.

The HW-072 module includes an LDR, LM393 comparator, and adjustable sensitivity potentiometer. Its digital output changes according to the detected light level.

Hardware Connections

Device| Signal| STM32 Pin
HW-072| DO| "PA1 / A1"
HW-072| VCC| "3.3V" / supported supply
HW-072| GND| "GND"
Relay| IN| "PA0 / A0"
Relay| VCC| "+5V"
Relay| GND| "GND"

The project also uses indicator LEDs connected to "PA5", "PB7", and "PC13".

Working Principle

             LIGHT
               │
               ▼
          ┌─────────┐
          │ HW-072  │
          │   LDR   │
          └────┬────┘
               │ DO
               ▼
          STM32G431
               │
       ┌───────┴───────┐
       ▼               ▼
    Indicator         Relay
       │               │
       ▼               ▼
     Status           Light

Demonstration

When the LDR is covered:

Dark
 ↓
HW-072 detects darkness
 ↓
Relay ON
 ↓
Indicator LEDs ON

When the LDR receives sufficient light:

Light
 ↓
HW-072 detects light
 ↓
Relay OFF
 ↓
Indicator LEDs OFF

The sensitivity can be adjusted using the module's onboard potentiometer.

---

04 — 🌡️ LM75 Digital Temperature + I²C LCD

"04_LM35_Temperature_with_LCD"

«Note: The folder is named "LM35_Temperature_with_LCD", but the actual project documentation and implementation describe an LM75 digital temperature sensor, not an analog LM35.»

This is one of the more advanced demonstrations because it uses two independent hardware I²C peripherals on the STM32G431:

- "I2C1" → LCD
- "I2C2" → LM75 temperature sensor

I²C Architecture

                 STM32G431
              ┌──────────────┐
              │              │
        I2C1  │              │  I2C2
              │              │
       PB6 ───┤ SCL          │─── PB10
       PB7 ───┤ SDA          │─── PB11
              │              │
              └──────┬───────┘
                     │
             ┌───────┴────────┐
             ▼                ▼
        I²C LCD             LM75
        Display          Temperature

The repository specifies "PB6/PB7" for the LCD and "PB10/PB11" for the LM75.

LCD Connections

LCD| STM32
SCL| "PB6"
SDA| "PB7"
VCC| "5V" / "3.3V"
GND| "GND"

LM75 Connections

LM75| STM32
SCL| "PB10"
SDA| "PB11"
VCC| "3.3V" / supported supply
GND| "GND"
OS| Leave open
A0–A2| Leave open

Temperature Processing

The LM75 temperature register provides two bytes of data. The project decodes the signed temperature value and converts it to degrees Celsius.

Example display:

+----------------+
|Temp: 25.50°C   |
|Status: NORMAL  |
+----------------+

The status changes according to the measured temperature:

≥ 35°C       → HOT!
≥ 28°C       → WARM
18–28°C      → NORMAL
≤ 18°C       → COLD

---

05 — ⌨️ 4×4 Matrix Keypad + I²C LCD

"05_Matrix_Keypad_with_LCD"

This demonstration introduces matrix keypad scanning and LCD-based user interaction.

A 4×4 keypad provides 16 keys using only 8 GPIO pins:

4 Rows + 4 Columns = 8 GPIOs

instead of requiring 16 independent GPIO inputs.

---

🔍 Matrix Scanning

The four rows are configured as outputs while the four columns use inputs with internal pull-ups.

The STM32 scans each row sequentially:

All Rows HIGH
      ↓
Drive Row 1 LOW
      ↓
Read Columns
      ↓
Detect Key
      ↓
Restore Row 1
      ↓
Drive Row 2 LOW
      ↓
Read Columns
      ↓
...

Keypad Pinout

Keypad| STM32
Row 1| "PC13"
Row 2| "PC15"
Row 3| "PF1"
Row 4| "PA1"
Column 1| "PA3"
Column 2| "PA5"
Column 3| "PA7"
Column 4| "PB1"

LCD

LCD| STM32
SCL| "PB2"
SDA| "PB10"
VCC| "+5V"
GND| "GND"

Interactive Features

The LCD provides:

SIECORE32 G431
4x4 KEYPAD DEMO

and displays the currently pressed key and typed text.

Special keys:

Key| Function
"*"| Clear typed text
"#"| Insert a space

---

06 — 🔢 7-Segment Display Counter

"06_Seven_Segment_4Digit"

«Important: Although the folder is named "06_Seven_Segment_4Digit", the current project README describes this implementation as a single-digit 7-segment display counter. It should therefore not be described as a four-digit multiplexed display unless the hardware/code is changed.»

The demonstration uses a standard 10-pin single-digit 7-segment display with:

- "a"
- "b"
- "c"
- "d"
- "e"
- "f"
- "g"
- "dp"

Segment Connections

Segment| STM32
"a"| "PA0"
"b"| "PA1"
"c"| "PA2"
"d"| "PA3"
"e"| "PA4"
"f"| "PA5"
"g"| "PA6"
"dp"| "PA7"

For the documented common-cathode configuration, the common pin is connected to GND.

Counter Behavior

On startup:

8.

is displayed briefly to verify that all segments are working.

The counter then cycles:

0 → 1 → 2 → 3 → 4
  → 5 → 6 → 7 → 8 → 9
  → 0 → ...

The decimal point also provides a blinking heartbeat indication.

---

🧰 Hardware

The demonstrations are designed around the STM32G431, particularly the SieCore 32 G431 board referenced by the individual project documentation.

Recommended Hardware

- STM32G431 development board
- ST-LINK programmer/debugger
- USB cable
- HW-072 LDR module
- 1-channel relay module
- LM75 temperature sensor
- I²C character LCD
- 4×4 matrix keypad
- Single-digit 7-segment display
- Jumper wires
- Breadboard where required
- Appropriate resistors for LEDs/displays

---

💻 Software

Recommended development environment:

- STM32CubeIDE
- STM32G431 device support
- ST-LINK tools/drivers
- STM32 HAL / CMSIS where applicable

The repository also includes prebuilt firmware artifacts in several project folders, including ".bin" and ".elf" files.

---

🔧 ST-LINK Driver

The repository includes a "DRIVER" directory containing ST-LINK driver resources, including:

DRIVER/
├── TXT.txt
└── stsw-link009.zip

If your STM32 board is not detected correctly by the programming/debugging software, install the appropriate ST-LINK driver before attempting to flash the firmware.

---

🚀 Getting Started

1. Clone the Repository

git clone https://github.com/ibmskbld-source/STM-32-BASICS-DEMO.git

cd STM-32-BASICS-DEMO

---

2. Install the Required Driver

If ST-LINK is not already installed:

DRIVER/
└── stsw-link009.zip

Install the appropriate ST-LINK driver for your system.

---

3. Choose a Demonstration

Start with:

01_LED_Blink

and progress through the remaining projects.

---

4. Open STM32CubeIDE

Create/open an STM32G431 project corresponding to the demonstration.

Configure the required GPIO/I²C peripherals according to that project's README.

---

5. Build

Build the project in STM32CubeIDE.

Project
   ↓
Build
   ↓
Compile
   ↓
Generate firmware

---

6. Connect ST-LINK

Connect the STM32G431 development board to the computer through the ST-LINK interface.

Verify that the target is detected.

---

7. Flash

Use STM32CubeIDE's Run/Debug functionality or the included firmware artifacts where appropriate.

The individual projects also include "upload.ps1" scripts in several demos for firmware upload workflows.

---

🧠 Learning Path

The six experiments can be followed as a progressive embedded-systems course:

                 STM32G431
                     │
                     ▼
              ┌─────────────┐
              │ 01 GPIO     │
              │ LED Blink   │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │ 02 Actuator │
              │ Relay       │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │ 03 Sensor   │
              │ LDR + Relay │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │ 04 I²C      │
              │ LM75 + LCD  │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │ 05 Input    │
              │ Keypad+LCD  │
              └──────┬──────┘
                     │
                     ▼
              ┌─────────────┐
              │ 06 Display    │
              │ 7-Segment     │
              └─────────────┘

---

📖 What These Demos Teach

Concept| Demonstration
GPIO configuration| 01
Digital output| 01, 02
Timing / delays| 01, 02, 06
Actuator control| 02, 03
Relay interfacing| 02, 03
Digital sensors| 03, 04
I²C communication| 04, 05
Multiple I²C peripherals| 04
LCD interfacing| 04, 05
Matrix scanning| 05
GPIO input| 05
7-segment encoding| 06
Embedded user interfaces| 04, 05, 06



⚠️ Important Notes

1. Check the project README

Each demonstration contains its own README with the specific wiring, board pins, operating behavior, and setup instructions.

Always check the individual project README before wiring the hardware.

2. Pin assignments are project-specific

The six projects do not use one common pin configuration.

Do not assume a pin assignment from one demo applies to another.

3. Power requirements

Check the voltage requirements of every external module before connecting it to the STM32.

4. Relay safety

The relay can switch higher-power loads, but the STM32 itself should never be directly connected to mains voltage.

---

🧪 Suggested Classroom Sequence

Lab 1 — GPIO Fundamentals

01_LED_Blink

Learn:

- GPIO clocks
- GPIO output
- Digital logic
- Delays

Lab 2 — Actuator Control

02_Relay_Module

Learn:

- Actuator interfacing
- Active-low control
- Relay switching

Lab 3 — Sensor-Based Automation

03_LDR_Automatic_Light

Learn:

- Digital sensors
- Threshold detection
- Automatic control
- Sensor + actuator integration

Lab 4 — Digital Sensor + Communication

04_LM35_Temperature_with_LCD

Learn:

- I²C
- Digital temperature sensing
- Multiple I²C peripherals
- LCD interfaces

Lab 5 — Human Input

05_Matrix_Keypad_with_LCD

Learn:

- Matrix scanning
- GPIO input
- Pull-ups
- User interfaces
- LCD output

Lab 6 — Digital Display

06_Seven_Segment_4Digit

Learn:

- Segment control
- Display encoding
- Timing
- Counter logic

---

🤖 AI-Assisted Development

These projects can also be used with AI-assisted coding environments such as Antigravity or other coding assistants.

AI can be useful for:

- Explaining STM32 code
- Modifying GPIO behavior
- Adding new features
- Debugging firmware
- Creating additional experiments
- Converting HAL-based examples into lower-level implementations
- Explaining peripheral configuration

However, always verify generated embedded code against:

- STM32G431 datasheet
- STM32G431 reference manual
- Development-board schematic
- Actual peripheral wiring

AI-generated code should be tested on the real hardware before being treated as verified firmware.

---

🎯 Project Philosophy

The purpose of this repository is simple:

«Learn embedded systems by actually connecting hardware and running the code.»

The progression is intentionally practical:

GPIO
 ↓
ACTUATOR
 ↓
SENSOR
 ↓
COMMUNICATION
 ↓
INPUT
 ↓
DISPLAY

Rather than presenting isolated code snippets, each project demonstrates a complete hardware interaction that can be tested on an STM32G431 board.

---

⭐ Recommended Starting Point

If you're completely new to STM32:

Start here
    ↓
01_LED_Blink
    ↓
02_Relay_Module
    ↓
03_LDR_Automatic_Light
    ↓
04_LM75 + LCD
    ↓
05_Matrix_Keypad + LCD
    ↓
06_7-Segment

By the end, you will have worked with GPIO, sensors, actuators, I²C, displays, user input, and basic embedded control logic.

---

📌 Repository

STM-32-BASICS-DEMO

"Open the GitHub Repository" (https://reference-url-citation.invalid/23)

---

👨‍💻 Maintained For

STM32G431 Embedded Systems Learning & Demonstration

Built for:

- 🎓 Students
- 🧑‍🏫 Faculty
- 🔬 Embedded Systems Labs
- 🛠️ Makers
- 💻 Firmware Beginners
- 🤖 AI-assisted embedded development

---

⭐ If You Find It Useful

If this repository helps you learn STM32:

⭐ Star it

🍴 Fork it

🛠️ Build the circuits

📚 Use the demos in your lab

And most importantly:

«Don't just read the code. Flash it, connect the hardware, and see it work.»
