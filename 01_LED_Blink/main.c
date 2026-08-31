/**
 ******************************************************************************
 * @file           : 01_LED_Blink/main.c
 * @brief          : "Hello World" LED Blink Demo for STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Connections:
 *   - On-board User LED LD2: PA5 (Nucleo-64 / Standard) & PB8 (Nucleo-32)
 *   - Or connect an external LED with a 220 ohm resistor from PA5 to GND
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"

/* Pin Definitions */
#define LED_PIN_PA5     GPIO_PIN_5
#define LED_PORT_PA     GPIOA

#define LED_PIN_PB8     GPIO_PIN_8
#define LED_PORT_PB     GPIOB

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize GPIO peripherals */
    MX_GPIO_Init();

    /* Infinite loop */
    while (1)
    {
        /* Toggle LED state on both PA5 and PB8 */
        HAL_GPIO_TogglePin(LED_PORT_PA, LED_PIN_PA5);
        HAL_GPIO_TogglePin(LED_PORT_PB, LED_PIN_PB8);

        /* Delay for 500 milliseconds (Blink rate = 1 Hz) */
        HAL_Delay(500);
    }
}

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure LED Pin PA5 (Standard Nucleo-64 User LED) */
    HAL_GPIO_WritePin(LED_PORT_PA, LED_PIN_PA5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PA5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT_PA, &GPIO_InitStruct);

    /* Configure LED Pin PB8 (Nucleo-32 / Alternative User LED) */
    HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB8, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PB8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT_PB, &GPIO_InitStruct);
}

/**
 * @brief System Clock Configuration (HSI 16MHz)
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
