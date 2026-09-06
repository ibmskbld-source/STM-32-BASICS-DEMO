/**
 ******************************************************************************
 * @file           : 01_LED_Blink/main_direct.c
 * @brief          : Standalone Relay + LED (5s ON / 5s OFF) for SieCore 32 G431
 ******************************************************************************
 */
#include <stdint.h>

/* Peripheral Base Addresses for STM32G431 */
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL
#define GPIOB_BASE      0x48000400UL
#define GPIOC_BASE      0x48000800UL

/* RCC Register */
#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4CUL))

/* GPIO Registers */
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_BSRR      (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

#define GPIOB_MODER     (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_BSRR      (*(volatile uint32_t *)(GPIOB_BASE + 0x18UL))

#define GPIOC_MODER     (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_BSRR      (*(volatile uint32_t *)(GPIOC_BASE + 0x18UL))

/* External delay function from startup.c */
extern void delay_ms(uint32_t ms);

int main(void)
{
    /* 1. Enable Clocks for GPIOA, GPIOB, GPIOC */
    RCC_AHB2ENR |= (1UL << 0) | (1UL << 1) | (1UL << 2);

    /* 2. Configure Pin Modes as Output (01b) */
    /* PA0 (Relay) -> Bits [1:0] = 01 */
    GPIOA_MODER &= ~(3UL << 0);
    GPIOA_MODER |=  (1UL << 0);

    /* PA1 (LED) -> Bits [3:2] = 01 */
    GPIOA_MODER &= ~(3UL << 2);
    GPIOA_MODER |=  (1UL << 2);

    /* PA5 (Test LED) -> Bits [11:10] = 01 */
    GPIOA_MODER &= ~(3UL << 10);
    GPIOA_MODER |=  (1UL << 10);

    /* PB7 (Test LED) -> Bits [15:14] = 01 */
    GPIOB_MODER &= ~(3UL << 14);
    GPIOB_MODER |=  (1UL << 14);

    /* PC13 (Status LED) -> Bits [27:26] = 01 */
    GPIOC_MODER &= ~(3UL << 26);
    GPIOC_MODER |=  (1UL << 26);

    /* Start: Turn ON Relay + LED immediately (SET = 3.3V) */
    GPIOA_BSRR = (1UL << 0) | (1UL << 1) | (1UL << 5);
    GPIOB_BSRR = (1UL << 7);
    GPIOC_BSRR = (1UL << 13);

    /* 3. 5s ON / 5s OFF Loop */
    while (1)
    {
        /* ON for 5 seconds */
        delay_ms(5000);

        /* Relay OFF + LED OFF (RESET = 0V -> Upper 16 bits of BSRR) */
        GPIOA_BSRR = (1UL << (0 + 16)) | (1UL << (1 + 16)) | (1UL << (5 + 16));
        GPIOB_BSRR = (1UL << (7 + 16));
        GPIOC_BSRR = (1UL << (13 + 16));

        /* OFF for 5 seconds */
        delay_ms(5000);

        /* Relay ON + LED ON (SET = 3.3V) */
        GPIOA_BSRR = (1UL << 0) | (1UL << 1) | (1UL << 5);
        GPIOB_BSRR = (1UL << 7);
        GPIOC_BSRR = (1UL << 13);
    }

    return 0;
}
