/**
 ******************************************************************************
 * @file           : 03_LDR_Automatic_Light/main_direct.c
 * @brief          : Standalone HW-072 (DO Pin) Light Sensor with Relay & LEDs
 *                   on SieCore 32 G431 (STM32G431CBT6).
 ******************************************************************************
 * Hardware Pin Connections:
 *
 *   1. HW-072 LDR Sensor Module (3-Pin: VCC, GND, DO):
 *      - VCC -> +3.3V (Header P5 on right edge) or +5V (Header P3)
 *      - GND -> GND   (Header P1, bottom pin)
 *      - DO  -> PA1   (Header P1, pin 'A1', left column pin 7)
 *
 *   2. Relay Module:
 *      - IN  -> PA0   (Header P1, pin 'A0', right column pin 6)
 *      - VCC -> +5V   (Header P3 top header labeled +5V)
 *      - GND -> GND   (Header P1 or P2 bottom pin)
 *
 *   3. Indicator LEDs (Synchronized with Relay):
 *      - PA5  (Header P1, pin 'A5')
 *      - PB7  (Header P2, pin 'B7')
 *      - PC13 (Header P1, pin 'C13')
 ******************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 * Hardware Register Definitions (STM32G431)
 * ========================================================================== */
#define RCC_BASE            0x40021000UL
#define GPIOA_BASE          0x48000000UL
#define GPIOB_BASE          0x48000400UL
#define GPIOC_BASE          0x48000800UL

#define RCC_AHB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0x4CUL))

#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER        (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_OSPEEDR       (*(volatile uint32_t *)(GPIOA_BASE + 0x08UL))
#define GPIOA_PUPDR         (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_IDR           (*(volatile uint32_t *)(GPIOA_BASE + 0x10UL))
#define GPIOA_BSRR          (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

#define GPIOB_MODER         (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_BSRR          (*(volatile uint32_t *)(GPIOB_BASE + 0x18UL))

#define GPIOC_MODER         (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_BSRR          (*(volatile uint32_t *)(GPIOC_BASE + 0x18UL))

extern void delay_ms(uint32_t ms);

/* ==========================================================================
 * Actuator Control Function
 * ========================================================================== */

static void streetlight_set_state(bool is_dark)
{
    if (is_dark)
    {
        /* DARK ENVIRONMENT:
         * - Relay ON (PA0 = LOW for Active-Low Relay modules)
         * - Indicator LEDs ON (PA5 = HIGH, PB7 = HIGH, PC13 = HIGH)
         */
        GPIOA_BSRR = (1UL << (0 + 16)); // PA0 = LOW (Relay ON)
        GPIOA_BSRR = (1UL << 5);        // PA5 = HIGH (LED ON)
        GPIOB_BSRR = (1UL << 7);        // PB7 = HIGH (LED ON)
        GPIOC_BSRR = (1UL << 13);       // PC13 = HIGH (LED ON)
    }
    else
    {
        /* BRIGHT ENVIRONMENT:
         * - Relay OFF (PA0 = HIGH for Active-Low Relay modules)
         * - Indicator LEDs OFF (PA5 = LOW, PB7 = LOW, PC13 = LOW)
         */
        GPIOA_BSRR = (1UL << 0);        // PA0 = HIGH (Relay OFF)
        GPIOA_BSRR = (1UL << (5 + 16)); // PA5 = LOW (LED OFF)
        GPIOB_BSRR = (1UL << (7 + 16)); // PB7 = LOW (LED OFF)
        GPIOC_BSRR = (1UL << (13 + 16));// PC13 = LOW (LED OFF)
    }
}

/* ==========================================================================
 * Application Entry Point
 * ========================================================================== */

int main(void)
{
    /* 1. Enable GPIO Clocks: GPIOA, GPIOB, GPIOC */
    RCC_AHB2ENR |= (1UL << 0) | (1UL << 1) | (1UL << 2);

    /* 2. Configure PA1 (HW-072 DO Pin) as Digital Input with Pull-up */
    GPIOA_MODER &= ~(3UL << 2); // Input mode (00b)
    GPIOA_PUPDR &= ~(3UL << 2);
    GPIOA_PUPDR |=  (1UL << 2); // Pull-up enabled (01b)

    /* 3. Configure PA0 (Relay) and PA5 (LED) as Output Push-Pull */
    GPIOA_MODER &= ~((3UL << 0) | (3UL << 10));
    GPIOA_MODER |=  ((1UL << 0) | (1UL << 10)); // Output mode (01b)
    GPIOA_OTYPER &= ~((1UL << 0) | (1UL << 5));  // Push-Pull
    GPIOA_OSPEEDR |= ((3UL << 0) | (3UL << 10)); // High speed

    /* 4. Configure PB7 (LED) and PC13 (LED) as Output Push-Pull */
    GPIOB_MODER &= ~(3UL << 14);
    GPIOB_MODER |=  (1UL << 14); // Output mode (01b)

    GPIOC_MODER &= ~(3UL << 26);
    GPIOC_MODER |=  (1UL << 26); // Output mode (01b)

    /* Start with Relay and LEDs in OFF state */
    streetlight_set_state(false);

    /* Debounce / Filter Counters */
    uint32_t high_count = 0;
    uint32_t low_count = 0;
    bool is_dark_active = false;

    /* 5. Main Control Loop */
    while (1)
    {
        /* Read PA1 (HW-072 DO Pin)
         * Standard HW-072 behavior:
         *   - Dark   -> DO = HIGH (1)
         *   - Bright -> DO = LOW  (0)
         */
        bool do_pin = (GPIOA_IDR & (1UL << 1)) != 0;

        if (do_pin)
        {
            high_count++;
            low_count = 0;
        }
        else
        {
            low_count++;
            high_count = 0;
        }

        /* 5 consecutive stable samples (~50ms debounce) */
        if (high_count >= 5 && !is_dark_active)
        {
            is_dark_active = true;
            streetlight_set_state(true);
        }
        else if (low_count >= 5 && is_dark_active)
        {
            is_dark_active = false;
            streetlight_set_state(false);
        }

        delay_ms(10);
    }

    return 0;
}
