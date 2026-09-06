/**
 ******************************************************************************
 * @file           : 03_LDR_Automatic_Light/main.c
 * @brief          : Automatic Light Control using HW-072 (DO Pin), Relay & LED
 *                   on SieCore 32 G431 (STM32G431CBT6).
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Pin Connections:
 *   1. HW-072 LDR Sensor (3-Pin: VCC, GND, DO):
 *      - VCC -> +3.3V (Header P5) or +5V (Header P3)
 *      - GND -> GND   (Header P1 / P2)
 *      - DO  -> PA1   (Header P1, pin 'A1', left column pin 7)
 *
 *   2. Relay Module:
 *      - IN  -> PA0   (Header P1, pin 'A0', right column pin 6)
 *      - VCC -> +5V   (Header P3)
 *      - GND -> GND   (Header P1 / P2)
 *
 *   3. Indicator LEDs:
 *      - PA5  (Header P1, pin 'A5')
 *      - PB7  (Header P2, pin 'B7')
 *      - PC13 (Header P1, pin 'C13')
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdbool.h>

#define RELAY_PIN           GPIO_PIN_0
#define RELAY_PORT          GPIOA

#define SENSOR_DO_PIN       GPIO_PIN_1
#define SENSOR_DO_PORT      GPIOA

#define LED_PIN_PA5         GPIO_PIN_5
#define LED_PORT_PA         GPIOA

#define LED_PIN_PB7         GPIO_PIN_7
#define LED_PORT_PB         GPIOB

#define LED_PIN_PC13        GPIO_PIN_13
#define LED_PORT_PC         GPIOC

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void Streetlight_Set_State(bool is_dark);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    bool light_is_on = false;
    Streetlight_Set_State(light_is_on);

    uint32_t high_count = 0;
    uint32_t low_count = 0;

    while (1)
    {
        /* Read HW-072 DO Pin (PA1):
         * Dark  -> DO = HIGH (1)
         * Light -> DO = LOW  (0)
         */
        GPIO_PinState do_state = HAL_GPIO_ReadPin(SENSOR_DO_PORT, SENSOR_DO_PIN);

        if (do_state == GPIO_PIN_SET)
        {
            high_count++;
            low_count = 0;
        }
        else
        {
            low_count++;
            high_count = 0;
        }

        /* 5 consecutive stable readings (~50ms debounce) */
        if (high_count >= 5 && !light_is_on)
        {
            light_is_on = true;
            Streetlight_Set_State(true);
        }
        else if (low_count >= 5 && light_is_on)
        {
            light_is_on = false;
            Streetlight_Set_State(false);
        }

        HAL_Delay(10);
    }
}

void Streetlight_Set_State(bool is_dark)
{
    if (is_dark)
    {
        /* Dark: Relay ON (Active-Low) + LEDs ON */
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT_PA, LED_PIN_PA5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB7, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT_PC, LED_PIN_PC13, GPIO_PIN_SET);
    }
    else
    {
        /* Bright: Relay OFF + LEDs OFF */
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT_PA, LED_PIN_PA5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB7, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT_PC, LED_PIN_PC13, GPIO_PIN_RESET);
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* PA1: HW-072 DO Input with Pull-up */
    GPIO_InitStruct.Pin = SENSOR_DO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(SENSOR_DO_PORT, &GPIO_InitStruct);

    /* PA0 (Relay) and PA5 (LED) Output Push-Pull */
    HAL_GPIO_WritePin(GPIOA, RELAY_PIN, GPIO_PIN_SET); // Default OFF
    HAL_GPIO_WritePin(GPIOA, LED_PIN_PA5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = RELAY_PIN | LED_PIN_PA5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB7 (LED) Output Push-Pull */
    HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB7, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PB7;
    HAL_GPIO_Init(LED_PORT_PB, &GPIO_InitStruct);

    /* PC13 (LED) Output Push-Pull */
    HAL_GPIO_WritePin(LED_PORT_PC, LED_PIN_PC13, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PC13;
    HAL_GPIO_Init(LED_PORT_PC, &GPIO_InitStruct);
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
