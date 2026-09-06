/**
 ******************************************************************************
 * @file           : 01_LED_Blink/main.c
 * @brief          : Relay + LED 5s ON / 5s OFF for SieCore 32 G431 (STM32G431CBT6)
 ******************************************************************************
 * Hardware Connections:
 *   - RELAY_PIN : PA0 (Header P1, pin 'A0')
 *   - LED_PIN   : PA1 (Header P1, pin 'A1')
 *   - Onboard/Test Pins: PA5, PB7, PC13
 ******************************************************************************
 */

#if __has_include("main.h")
#include "main.h"
#else
#include "stm32g4xx_hal.h"
#endif

#define RELAY_PORT  GPIOA
#define RELAY_PIN   GPIO_PIN_0

#define LED_PORT    GPIOA
#define LED_PIN     GPIO_PIN_1

/* Additional Onboard / Multi-pin definitions */
#define LED_PIN_PA5     GPIO_PIN_5
#define LED_PIN_PB7     GPIO_PIN_7
#define LED_PIN_PC13    GPIO_PIN_13
#define LED_PORT_PB     GPIOB
#define LED_PORT_PC     GPIOC

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    // Start: Relay ON + LED ON
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, LED_PIN_PA5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_PORT_PC, LED_PIN_PC13, GPIO_PIN_SET);

    while (1)
    {
        // ON for 5 seconds
        HAL_Delay(5000);

        // Relay OFF + LED OFF
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, LED_PIN_PA5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB7, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT_PC, LED_PIN_PC13, GPIO_PIN_RESET);

        // OFF for 5 seconds
        HAL_Delay(5000);

        // Relay ON + LED ON
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, LED_PIN_PA5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB7, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT_PC, LED_PIN_PC13, GPIO_PIN_SET);
    }
}

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO Clocks (Port A, Port B, Port C) */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* --- Configure Port A: PA0 (Relay), PA1 (LED), PA5 (LED) --- */
    HAL_GPIO_WritePin(GPIOA, RELAY_PIN | LED_PIN | LED_PIN_PA5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = RELAY_PIN | LED_PIN | LED_PIN_PA5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* --- Configure Port B: PB7 --- */
    HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB7, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PB7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT_PB, &GPIO_InitStruct);

    /* --- Configure Port C: PC13 --- */
    HAL_GPIO_WritePin(LED_PORT_PC, LED_PIN_PC13, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PC13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT_PC, &GPIO_InitStruct);
}

/**
 * @brief System Clock Configuration (Internal HSI 16MHz)
 */
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
