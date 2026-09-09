# STM-32-BASICS-DEMO

[![Platform](https://img.shields.io/badge/Platform-STM32%20%7C%20ARM%20Cortex--M-032347?logo=stmicroelectronics)](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html)
[![Language](https://img.shields.io/badge/Language-C%20%2F%20Embedded%20C-00599C?logo=c)](https://en.cppreference.com/w/c)
[![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-002060)](https://www.st.com/en/development-tools/stm32cubeide.html)
[![HAL](https://img.shields.io/badge/Driver-STM32Cube%20HAL%20%26%20CMSIS-blue)](https://www.st.com/en/embedded-software/stm32cube-mcu-packages.html)

A modular, hardware-tested embedded firmware demonstration suite for STMicroelectronics STM32 microcontrollers (ARM Cortex-M family). This repository provides foundational peripheral drivers, bare-metal integration examples, and standard STM32Cube Hardware Abstraction Layer (HAL) implementations.

---

## 📋 Table of Contents

1. [Project Overview](#-project-overview)
2. [Target Hardware Specifications](#-target-hardware-specifications)
3. [Software & Toolchain Requirements](#-software--toolchain-requirements)
4. [Repository Architecture](#-repository-architecture)
5. [Peripheral Demonstration Modules](#-peripheral-demonstration-modules)
   - [1. GPIO Digital Output (Blink / Status Indicator)](#1-gpio-digital-output-blink--status-indicator)
   - [2. External Interrupts (EXTI & NVIC)](#2-external-interrupts-exti--nvic)
   - [3. General-Purpose Timers & PWM Generation](#3-general-purpose-timers--pwm-generation)
   - [4. USART/UART Telemetry & Retargeted printf](#4-usartuart-telemetry--retargeted-printf)
   - [5. Analog-to-Digital Conversion (ADC Single & Continuous)](#5-analog-to-digital-conversion-adc-single--continuous)
   - [6. Direct Memory Access (DMA) Integration](#6-direct-memory-access-dma-integration)
6. [Getting Started & Build Guide](#-getting-started--build-guide)
   - [Clone the Repository](#1-clone-the-repository)
   - [Import into STM32CubeIDE](#2-import-into-stm32cubeide)
   - [Clock Configuration Tree](#3-clock-configuration-tree)
   - [Compile & Flash via ST-LINK](#4-compile--flash-via-st-link)
7. [Serial Communication & Debug Console](#-serial-communication--debug-console)
8. [Hardware Wiring & Pinout Mapping](#-hardware-wiring--pinout-mapping)
9. [Troubleshooting & Debugging Guide](#-troubleshooting--debugging-guide)

---

## 🔬 Project Overview

Developing firmware for modern ARM Cortex-M microcontrollers requires a solid grasp of clock tree propagation, bus architectures (AHB/APB), memory mapping, and interrupt routing. 

This repository serves as a self-contained learning and reference framework covering:
- **Bare-Metal & HAL Parity:** Understanding how low-level register manipulations compare with HAL library abstractions.
- **Deterministic Timing:** Eliminating blocking software delay loops (`HAL_Delay`) in favor of hardware timer interrupts and SysTick.
- **Asynchronous Event Handling:** Transitioning from CPU-intensive polling to interrupt-driven and DMA-driven architectures.
- **Telemetry & Observability:** Real-time debug logging over asynchronous serial interfaces.

---

## 🔌 Target Hardware Specifications

While written with standard STM32Cube HAL drivers portable across the entire STM32 lineup, the examples are pre-configured for standard Nucleo and discovery prototyping kits:

| Parameter | Primary Reference Board | Alternate Supported Target |
| :--- | :--- | :--- |
| **Development Board** | NUCLEO-F401RE / NUCLEO-F446RE | STM32F103C8T6 ("Blue Pill") |
| **Microcontroller** | STM32F401RET6 (Cortex-M4F) | STM32F103C8T6 (Cortex-M3) |
| **Max System Clock** | 84 MHz (F401) / 180 MHz (F446) | 72 MHz |
| **Flash Memory** | 512 KB | 64 KB / 128 KB |
| **SRAM** | 96 KB | 20 KB |
| **FPU** | Single-precision hardware FPU | None |
| **On-board Debugger** | Integrated ST-LINK/V2-1 (SWD) | External ST-LINK V2 Dongle |
| **User Interface** | Green LED (PA5), User Button B1 (PC13) | On-board LED (PC13) |

---

## 💻 Software & Toolchain Requirements

- **Integrated Development Environment:** [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) (v1.14.0 or later)
- **Compiler:** GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`)
- **Firmware Package:** STM32CubeF4 Firmware Package (`v1.27.0`+) / STM32CubeF1 (`v1.8.5`+)
- **Programmer / Flasher Utilities:**
  - `ST-LINK Utility` or `STM32CubeProgrammer`
  - OpenOCD (optional for command-line workflows)
- **Host Serial Console:** PuTTY, Tera Term, minicom, or VS Code Serial Monitor

---

## 📁 Repository Architecture

```text
STM-32-BASICS-DEMO/
├── Core/
│   ├── Inc/
│   │   ├── main.h                 # Peripheral handles, global pin aliases, macros
│   │   ├── stm32f4xx_hal_conf.h   # HAL module enablement flags and oscillator frequencies
│   │   └── stm32f4xx_it.h         # Interrupt handler prototypes
│   ├── Src/
│   │   ├── main.c                 # Application entry point, peripherals init & main loop
│   │   ├── stm32f4xx_hal_msp.c    # MCU Support Package (clocks, pin muxing, NVIC setup)
│   │   ├── stm32f4xx_it.c         # ISR vectors (SysTick, EXTI, TIM, USART)
│   │   └── system_stm32f4xx.c     # CMSIS clock generation and vector table offset setup
│   └── Startup/
│       └── startup_stm32f401retx.s# Reset handler, stack/heap reservation, vector table
├── Drivers/
│   ├── CMSIS/                     # ARM Cortex-M architecture definitions & core headers
│   └── STM32F4xx_HAL_Driver/      # ST Hardware Abstraction Layer API source & headers
├── .cproject                      # Eclipse CDT project configuration
├── .project                       # Eclipse project metadata
├── STM32F401RETX_FLASH.ld         # GNU Linker script (flash/RAM region partitioning)
└── README.md                      # Comprehensive project documentation
```

---

## 🧩 Peripheral Demonstration Modules

### 1. GPIO Digital Output (Blink / Status Indicator)
Configures a general-purpose output pin in Push-Pull mode with internal pull-up/pull-down disabled.
- **Port/Pin:** `GPIOA`, `GPIO_PIN_5` (On-board LD2)
- **Mode:** Output Push-Pull, Low Speed
- **Behavior:** Toggles state every 500 ms.
```c
/* Basic Polling Toggle in main() while loop */
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
HAL_Delay(500);
```

### 2. External Interrupts (EXTI & NVIC)
Demonstrates asynchronous edge detection without CPU polling. Configures `PC13` (User Button) as an EXTI falling-edge interrupt source.
- **Port/Pin:** `GPIOC`, `GPIO_PIN_13`
- **Interrupt Line:** EXTI Line 13 (mapped to `EXTI15_10_IRQn`)
- **Implementation Callback:**
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        // Toggle indicator state on button press
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}
```

### 3. General-Purpose Timers & PWM Generation
Generates a hardware Pulse Width Modulation (PWM) signal on a timer output channel to create an LED "breathing" intensity fade without software cycle waste.
- **Timer:** `TIM2`, Channel 1 (`TIM_CHANNEL_1`)
- **Clock Configuration:** Prescaler calculated for 1 MHz counter clock; Auto-reload register (`ARR`) set to 1000 (1 kHz PWM carrier frequency).
- **Duty Cycle Adjustment:**
```c
/* Update duty cycle in hardware CCR register */
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty_cycle);
```

### 4. USART/UART Telemetry & Retargeted printf
Sets up an asynchronous serial channel for diagnostic output and telemetry streaming.
- **Instance:** `USART2` (Connected to ST-LINK Virtual COM Port on Nucleo boards)
- **Baud Rate:** `115200 bps`, 8 data bits, no parity, 1 stop bit
- **Retargeting C Standard I/O:**
```c
#include <stdio.h>

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

// In application loop:
printf("[SYS_STATUS] Uptime: %lu ms | Core Temp: %.2f C\r\n", HAL_GetTick(), temp_c);
```

### 5. Analog-to-Digital Conversion (ADC Single & Continuous)
Converts continuous analog signals into 12-bit digital values ($0 - 4095$ range across $0 - 3.3	ext{V}$).
- **Instance:** `ADC1`, Channel 0 (`PA0`)
- **Resolution:** 12 bits, Right aligned data
- **Sampling:**
```c
HAL_ADC_Start(&hadc1);
if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
{
    uint32_t raw_adc = HAL_ADC_GetValue(&hadc1);
    float voltage = ((float)raw_adc * 3.3f) / 4095.0f;
    printf("ADC Raw: %lu | Measured: %.2f V\r\n", raw_adc, voltage);
}
HAL_ADC_Stop(&hadc1);
```

### 6. Direct Memory Access (DMA) Integration
Transfers data buffers from peripheral memory directly to SRAM without CPU intervention, freeing processor cycles for calculations.
- **Use Case:** High-throughput circular UART reception or continuous multi-channel ADC scanning.

---

## 🚀 Getting Started & Build Guide

### 1. Clone the Repository
```bash
git clone https://github.com/ibmskbld-source/STM-32-BASICS-DEMO.git
cd STM-32-BASICS-DEMO
```

### 2. Import into STM32CubeIDE
1. Open **STM32CubeIDE**.
2. Select your default workspace directory.
3. Click `File` ➔ `Import...` ➔ `General` ➔ `Existing Projects into Workspace`.
4. Browse to the directory where you cloned `STM-32-BASICS-DEMO`.
5. Ensure the project is checked and click **Finish**.

### 3. Clock Configuration Tree
For high-frequency stability, the system clock is configured using the on-board High-Speed External (HSE) clock source bypassed from the ST-LINK MCO (8 MHz):
- **Source:** HSE / HSI -> PLL Multiplication
- **System Clock (SYSCLK):** 84 MHz
- **AHB Bus (HCLK):** 84 MHz
- **APB1 Peripheral Bus (PCLK1):** 42 MHz
- **APB2 Peripheral Bus (PCLK2):** 84 MHz

### 4. Compile & Flash via ST-LINK
1. **Build Project:** Press `Ctrl + B` (or click the hammer icon 🔨 in the top toolbar). Verify that the console reports zero errors and zero warnings.
2. **Connect Hardware:** Connect the Nucleo development board via USB.
3. **Run / Flash:** Click the green **Run** icon (`Ctrl + F11`) or select **Debug** (`F11`) to initiate an interactive GDB debugging session.

---

## 🖥 Serial Communication & Debug Console

To read the serial console output:
1. Connect the board via USB (the ST-LINK Virtual COM Port is recognized as a USB Serial Device).
2. Identify the assigned port:
   - **Windows:** Device Manager ➔ Ports (COM & LPT) ➔ `STMicroelectronics STLink Virtual COM Port (COMx)`
   - **Linux:** `dmesg | grep tty` ➔ `/dev/ttyACM0`
   - **macOS:** `ls /dev/tty.usbmodem*`
3. Launch your serial client with the following parameters:
   - **Baud Rate:** `115200`
   - **Data Bits:** `8`
   - **Stop Bits:** `1`
   - **Parity:** `None`
   - **Flow Control:** `None`
   - **Line Endings:** `CR + LF` (`\r\n`)

---

## 📌 Hardware Wiring & Pinout Mapping

| MCU Pin | Peripheral Function | Board Connection | Purpose / Target |
| :--- | :--- | :--- | :--- |
| **PA5** | `GPIO_Output` / `TIM2_CH1` | Arduino D13 (Green LED LD2) | Digital Status Toggle / PWM Fade |
| **PC13** | `GPIO_EXTI13` | Blue Push Button B1 | User Interrupt Trigger |
| **PA2** | `USART2_TX` | ST-LINK Virtual COM Port | Telemetry & printf log output |
| **PA3** | `USART2_RX` | ST-LINK Virtual COM Port | Telemetry command input |
| **PA0** | `ADC1_IN0` | Arduino A0 Pin | Analog sensor / potentiometer input |
| **GND** | Ground Reference | Power Header GND | Common circuit ground |
| **3V3** | 3.3V Power Out | Power Header 3V3 | Peripheral supply voltage |

---

## 🔧 Troubleshooting & Debugging Guide

### 1. `ST-LINK: No target connected` or `Target not halted`
- Ensure the USB cable supports data lines and is not a charge-only cable.
- In STM32CubeIDE, open **Debug Configurations** ➔ **Debugger** tab:
  - Check **Software System Reset** or **Connect under reset**.
- If the chip is locked in deep sleep or low-power state, hold down the black hardware **RESET** button, click **Debug / Run**, and release the button when the debugger establishes SWD communication.

### 2. Garbled or Gibberish Characters on Serial Terminal
- Baud rate mismatch: Ensure host terminal is configured to `115200`.
- System clock divergence: Ensure `HSE_VALUE` in `stm32f4xx_hal_conf.h` matches your hardware crystal frequency (typically `8000000` for 8 MHz).

### 3. Program Freezes in `Error_Handler()` or `HardFault_Handler()`
- An uninitialized peripheral pointer was accessed, or a clock gate was not enabled prior to accessing peripheral registers.
- Always check that `__HAL_RCC_<PERIPH>_CLK_ENABLE()` is called before configuring the peripheral registers.

---

## 🤝 Contributing

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/new-peripheral-demo`).
3. Commit your changes (`git commit -m "Add SPI loopback demonstration"`).
4. Push to the branch (`git push origin feature/new-peripheral-demo`).
5. Open a Pull Request.
