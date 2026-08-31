/**
 ******************************************************************************
 * @file           : 02_Relay_Module/main.c
 * @brief          : 1-Channel Relay Module Control Demo for STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Connections:
 *   Relay VCC  -> 5V (or 3.3V depending on module)
 *   Relay GND  -> GND
 *   Relay IN   -> PA0 (GPIO Output)
 *   Onboard LED-> PA5 (Visual Indicator)
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"

/* Pin Definitions */
#define RELAY_PIN       GPIO_PIN_0
#define RELAY_PORT      GPIOA

#define LED_PIN         GPIO_PIN_5
#define LED_PORT        GPIOA

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void Relay_Set(uint8_t state);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    /* Infinite loop */
    while (1)
    {
        /* 1. Turn Relay ON (LED Indicator ON) */
        Relay_Set(1);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(2000); // Keep ON for 2 seconds

        /* 2. Turn Relay OFF (LED Indicator OFF) */
        Relay_Set(0);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(2000); // Keep OFF for 2 seconds
    }
}

/**
 * @brief  Controls Relay state.
 * @note   Most hobby relay modules are ACTIVE-LOW:
 *         - LOW  (0V) -> Relay Coil Energized (ON)
 *         - HIGH (3.3V) -> Relay Coil De-energized (OFF)
 * @param  state: 1 to Turn ON, 0 to Turn OFF
 */
void Relay_Set(uint8_t state)
{
    if (state)
    {
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET); // Active-Low ON
    }
    else
    {
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);   // OFF
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

    /* Configure Relay Pin (PA0) as Output Push-Pull */
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET); // Default OFF (Active-Low)
    GPIO_InitStruct.Pin = RELAY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RELAY_PORT, &GPIO_InitStruct);

    /* Configure Onboard Indicator LED (PA5) as Output Push-Pull */
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

/**
 * @brief System Clock Configuration (HSI 16MHz default)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /** Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}
