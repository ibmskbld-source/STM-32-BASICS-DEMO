/**
 ******************************************************************************
 * @file           : 08_Seven_Segment_4Digit/main.c
 * @brief          : Single Digit 7-Segment (D5611A/B / KNOT542R) Counter Demo
 *                   on SieCore 32 G431 (STM32G431CBT6).
 * @author         : Embedded Systems Classroom Demo
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
 *   Common Mode Selector:
 *     - Default Mode: Common Cathode (COM to GND).
 *     - If Common Anode: Connect COM to +3.3V, and connect PB0 to GND.
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdbool.h>

/* Segment Pins on GPIOA: PA0..PA7 */
#define SEG_PORT    GPIOA

/* Segment Lookup Table for Digits 0 to 9 (Bit order: DP g f e d c b a) */
const uint8_t DIGIT_LUT[10] = {
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

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void Display_Digit(uint8_t digit, bool show_dp, bool is_common_anode);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    bool is_common_anode = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET);

    /* Startup Test: Light all segments (8.) for 1 second */
    Display_Digit(8, true, is_common_anode);
    HAL_Delay(1000);

    uint8_t count = 0;
    bool dp_state = false;

    /* Infinite loop: Count 0 to 9 every 1 second */
    while (1)
    {
        is_common_anode = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET);

        Display_Digit(count, dp_state, is_common_anode);
        dp_state = !dp_state;

        HAL_Delay(1000);

        count++;
        if (count > 9)
        {
            count = 0;
        }
    }
}

void Display_Digit(uint8_t digit, bool show_dp, bool is_common_anode)
{
    if (digit > 9) digit = 0;

    uint8_t pattern = DIGIT_LUT[digit];
    if (show_dp)
    {
        pattern |= (1 << 7);
    }

    if (is_common_anode)
    {
        pattern = ~pattern;
    }

    for (int i = 0; i < 8; i++)
    {
        GPIO_PinState pin_state = (pattern & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(GPIOA, (1 << i), pin_state);
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure PA0..PA7 as Output Push-Pull */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure PB0 as Input with Pull-Up (Mode Selector: CC default, CA if grounded) */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}
