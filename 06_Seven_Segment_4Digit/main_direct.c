/**
 ******************************************************************************
 * @file           : 08_Seven_Segment_4Digit/main_direct.c
 * @brief          : Single Digit 7-Segment (D5611A/B / KNOT542R) Counter Demo
 *                   on SieCore 32 G431 (STM32G431CBT6).
 ******************************************************************************
 * Hardware Pin Connections (10-Pin Display Module):
 *
 *   Display Pinout (Facing Front, DP at bottom right):
 *     Top Pins (Left to Right):
 *       Pin 10 -> Segment g   -> PA6 (Header P1, pin 'A6')
 *       Pin 9  -> Segment f   -> PA5 (Header P1, pin 'A5')
 *       Pin 8  -> Common COM  -> GND (Header P1) [or +3.3V if Common Anode]
 *       Pin 7  -> Segment a   -> PA0 (Header P1, pin 'A0')
 *       Pin 6  -> Segment b   -> PA1 (Header P1, pin 'A1')
 *
 *     Bottom Pins (Left to Right):
 *       Pin 1  -> Segment e   -> PA4 (Header P1, pin 'A4')
 *       Pin 2  -> Segment d   -> PA3 (Header P1, pin 'A3')
 *       Pin 3  -> Common COM  -> GND (Header P1) [internally tied to Pin 8]
 *       Pin 4  -> Segment c   -> PA2 (Header P1, pin 'A2')
 *       Pin 5  -> Decimal DP  -> PA7 (Header P1, pin 'A7')
 *
 *   Common Cathode vs Common Anode:
 *     - Default Mode: Common Cathode (5611A / KNOT542R) -> COM to GND.
 *     - If your display is Common Anode (5611B): Connect COM to +3.3V,
 *       and connect PB0 to GND (Header P1 'B0' to 'GND').
 ******************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>

/* Hardware Register Definitions */
#define RCC_BASE        0x40021000UL
#define GPIOA_BASE      0x48000000UL
#define GPIOB_BASE      0x48000400UL

#define RCC_AHB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x4CUL))

#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER    (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_OSPEEDR   (*(volatile uint32_t *)(GPIOA_BASE + 0x08UL))
#define GPIOA_PUPDR     (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x14UL))
#define GPIOA_BSRR      (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

#define GPIOB_MODER     (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_PUPDR     (*(volatile uint32_t *)(GPIOB_BASE + 0x0CUL))
#define GPIOB_IDR       (*(volatile uint32_t *)(GPIOB_BASE + 0x10UL))

extern void delay_ms(uint32_t ms);

/* 7-Segment Patterns (Bit order: DP g f e d c b a) */
/*
 * Bit 0: a (PA0)
 * Bit 1: b (PA1)
 * Bit 2: c (PA2)
 * Bit 3: d (PA3)
 * Bit 4: e (PA4)
 * Bit 5: f (PA5)
 * Bit 6: g (PA6)
 * Bit 7: dp (PA7)
 */
static const uint8_t DIGIT_LUT[10] = {
    0x3F, // 0: a, b, c, d, e, f
    0x06, // 1: b, c
    0x5B, // 2: a, b, d, e, g
    0x4F, // 3: a, b, c, d, g
    0x66, // 4: b, c, f, g
    0x6D, // 5: a, c, d, f, g
    0x7D, // 6: a, c, d, e, f, g
    0x07, // 7: a, b, c
    0x7F, // 8: a, b, c, d, e, f, g
    0x6F  // 9: a, b, c, d, f, g
};

static void display_digit(uint8_t digit, bool show_dp, bool is_common_anode)
{
    if (digit > 9) digit = 0;

    uint8_t pattern = DIGIT_LUT[digit];
    if (show_dp)
    {
        pattern |= (1 << 7); // Turn on DP (PA7)
    }

    if (is_common_anode)
    {
        pattern = ~pattern; // Invert logic for Common Anode
    }

    // Set PA0..PA7 outputs in a single atomic cycle
    uint32_t current = GPIOA_ODR & ~0xFFUL;
    GPIOA_ODR = current | (uint32_t)pattern;
}

int main(void)
{
    /* 1. Enable GPIOA and GPIOB clocks */
    RCC_AHB2ENR |= (1UL << 0) | (1UL << 1);

    /* 2. Configure PA0..PA7 as General Purpose Output Push-Pull (01b) */
    // Clear MODER for pins 0..7 (bits 15:0)
    GPIOA_MODER &= ~0x0000FFFFUL;
    // Set MODER to 01b for pins 0..7
    GPIOA_MODER |=  0x00005555UL;

    // Push-pull for pins 0..7
    GPIOA_OTYPER &= ~0x00FFUL;

    // High speed for pins 0..7
    GPIOA_OSPEEDR |= 0x0000FFFFUL;

    // No pull-up/pull-down
    GPIOA_PUPDR &= ~0x0000FFFFUL;

    /* 3. Configure PB0 as Input with internal Pull-up (Mode Selector)
     *    PB0 = HIGH (Open)  -> Common Cathode mode (default)
     *    PB0 = LOW  (to GND)-> Common Anode mode
     */
    GPIOB_MODER &= ~(3UL << 0);
    GPIOB_PUPDR &= ~(3UL << 0);
    GPIOB_PUPDR |=  (1UL << 0); // Pull-up

    /* Check Mode */
    bool is_common_anode = ((GPIOB_IDR & (1UL << 0)) == 0);

    /* 4. Startup Self-Test: Light all segments (8.) for 1 second */
    display_digit(8, true, is_common_anode);
    delay_ms(1000);

    uint8_t count = 0;
    bool dp_state = false;

    /* 5. Main Single Digit Counter Loop (0 to 9) */
    while (1)
    {
        // Re-check mode pin in case user swapped jumper
        is_common_anode = ((GPIOB_IDR & (1UL << 0)) == 0);

        // Display current digit with blinking DP
        display_digit(count, dp_state, is_common_anode);

        // Toggle decimal point
        dp_state = !dp_state;

        // Wait 1 second (1000 ms)
        delay_ms(1000);

        // Increment counter (0 -> 9 -> 0)
        count++;
        if (count > 9)
        {
            count = 0;
        }
    }

    return 0;
}
