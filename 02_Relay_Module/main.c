/**
 ******************************************************************************
 * @file    02_Relay_Module/main.c
 * @brief   Relay ON / OFF Switching Demo (Every 3 Seconds) — SieCore 32 G431
 *          Relay Pin: PA0 ONLY (No LEDs used).
 ******************************************************************************
 * Hardware Connection:
 *   - Relay IN  -> PA0 (Header P1, right column pin 6 labeled 'A0')
 *   - Relay VCC -> +5V (Top Header P3 labeled '+5V') or +3.3V (Header P5)
 *   - Relay GND -> GND (Header P1 or P2 bottom pin labeled 'GND')
 ******************************************************************************
 */

#if __has_include("main.h")
#include "main.h"
#else
#include "stm32g4xx_hal.h"
#endif

/* Pin Definition: Relay on PA0 ONLY */
#define RELAY_PORT   GPIOA
#define RELAY_PIN    GPIO_PIN_0

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    while (1)
    {
        /* Relay ON (3 seconds) */
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET); // 0V (Active-Low ON)
        HAL_Delay(3000);

        /* Relay OFF (3 seconds) */
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);   // 3.3V (Active-Low OFF)
        HAL_Delay(3000);
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef g = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure PA0 ONLY as Push-Pull Output */
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET); // Initial state: OFF
    g.Pin   = RELAY_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RELAY_PORT, &g);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState            = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState        = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&osc);

    clk.ClockType       = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource    = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider   = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider  = RCC_HCLK_DIV1;
    clk.APB2CLKDivider  = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0);
}
