/**
 ******************************************************************************
 * @file           : 02_Relay_Module/main_direct.c
 * @brief          : Standalone Direct-Register Relay ON/OFF Demo (Every 3 Seconds)
 *                   Relay Pin: PA0 ONLY (No LEDs used).
 *                   SieCore 32 G431 (STM32G431CBT6).
 ******************************************************************************
 * Hardware Connection:
 *   - Relay IN  -> PA0 (Header P1, right column pin 6 labeled 'A0')
 *   - Relay VCC -> +5V (Top Header P3 labeled '+5V') or +3.3V (Header P5)
 *   - Relay GND -> GND (Header P1 or P2, bottom pin labeled 'GND')
 ******************************************************************************
 */

#include <stdint.h>

#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL

#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4CUL))

#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER    (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_OSPEEDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x08UL))
#define GPIOA_PUPDR     (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_BSRR      (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

extern void delay_ms(uint32_t ms);

int main(void)
{
    /* 1. Enable Clock for GPIOA only */
    RCC_AHB2ENR |= (1UL << 0);

    /* 2. Configure PA0 (Relay IN) as Output Push-Pull (01b) */
    GPIOA_MODER &= ~(3UL << 0);
    GPIOA_MODER |=  (1UL << 0);   // General Purpose Output (01b)

    GPIOA_OTYPER &= ~(1UL << 0);  // Push-Pull (0)
    GPIOA_OSPEEDR |= (3UL << 0);  // High Speed (11b)
    GPIOA_PUPDR  &= ~(3UL << 0);  // No pull (00b)

    /* 3. Main Loop: Toggle PA0 every 3 seconds */
    while (1)
    {
        /* --- STATE 1: Relay ON (3 Seconds) ---
         * Drives PA0 LOW (Active-Low relays turn ON)
         * If using an Active-High relay, this is one state and the next is opposite.
         */
        GPIOA_BSRR = (1UL << (0 + 16)); // PA0 = 0V (LOW)
        delay_ms(3000);

        /* --- STATE 2: Relay OFF (3 Seconds) ---
         * Drives PA0 HIGH (3.3V)
         */
        GPIOA_BSRR = (1UL << 0);        // PA0 = 3.3V (HIGH)
        delay_ms(3000);
    }

    return 0;
}
