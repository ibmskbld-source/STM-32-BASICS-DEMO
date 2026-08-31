/**
 ******************************************************************************
 * @file           : 03_LDR_Automatic_Light/main.c
 * @brief          : Automatic Light Control using LDR, Relay & LED on STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Behavior (Automatic Streetlight / Night-Light System):
 *   - Dark Environment   (Low Light)  -> Turns ON Relay (for AC Lamp) & LED Indicator
 *   - Bright Environment (High Light) -> Turns OFF Relay & LED Indicator
 *   - Features 8-sample ADC averaging for noise filtering
 *   - Features Hysteresis to prevent rapid relay clicking around threshold
 *
 * Hardware Connections:
 *   LDR Module VCC -> 3.3V
 *   LDR Module GND -> GND
 *   LDR Analog Out -> PA1 (ADC1_IN2)
 *   Relay IN       -> PA0 (GPIO Output, Active-Low)
 *   Indicator LED  -> PA5 (Nucleo-64 User LED) & PB8 (Nucleo-32 User LED)
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"

/* Pin Definitions */
#define RELAY_PIN           GPIO_PIN_0
#define RELAY_PORT          GPIOA

#define LED_PIN_PA5         GPIO_PIN_5
#define LED_PORT_PA         GPIOA

#define LED_PIN_PB8         GPIO_PIN_8
#define LED_PORT_PB         GPIOB

/* ADC Hysteresis Thresholds (12-bit range: 0 to 4095) */
/* In standard LDR divider: Dark = High ADC Value (>2200), Bright = Low ADC Value (<1600) */
#define THRESHOLD_DARK_ON   2200  // When ADC exceeds 2200 (Dark)   -> Turn Relay & LED ON
#define THRESHOLD_BRIGHT_OFF 1600 // When ADC drops below 1600 (Bright) -> Turn Relay & LED OFF

/* Peripheral Handles */
ADC_HandleTypeDef hadc1;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
uint16_t LDR_Read_Filtered_ADC(void);
void Streetlight_Set_State(uint8_t state);

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
    MX_ADC1_Init();

    /* Run ADC Calibration for high conversion accuracy */
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

    uint8_t light_is_on = 0; // Current state: 0 = OFF, 1 = ON

    /* Infinite loop */
    while (1)
    {
        /* 1. Read Filtered Analog Value from LDR (0 to 4095) */
        uint16_t adc_val = LDR_Read_Filtered_ADC();

        /* 2. Automatic Night-Light Logic with Hysteresis */
        if (adc_val >= THRESHOLD_DARK_ON && light_is_on == 0)
        {
            /* Dark environment detected -> Turn Relay & LED ON */
            Streetlight_Set_State(1);
            light_is_on = 1;
        }
        else if (adc_val <= THRESHOLD_BRIGHT_OFF && light_is_on == 1)
        {
            /* Bright environment detected -> Turn Relay & LED OFF */
            Streetlight_Set_State(0);
            light_is_on = 0;
        }

        /* 3. Small sampling delay */
        HAL_Delay(100);
    }
}

/**
 * @brief  Reads ADC1 Channel 2 (PA1) with 8-sample noise averaging.
 * @return Filtered ADC Value (0 to 4095)
 */
uint16_t LDR_Read_Filtered_ADC(void)
{
    uint32_t sum = 0;
    const uint8_t SAMPLES = 8;

    for (int i = 0; i < SAMPLES; i++)
    {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        sum += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(2);
    }

    return (uint16_t)(sum / SAMPLES);
}

/**
 * @brief Controls both Relay (PA0) and Indicator LEDs (PA5 & PB8).
 * @note  Relay is Active-Low: 0V = ON, 3.3V = OFF
 */
void Streetlight_Set_State(uint8_t state)
{
    if (state)
    {
        /* Turn ON Relay (Active-Low) and LEDs */
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT_PA, LED_PIN_PA5, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB8, GPIO_PIN_SET);
    }
    else
    {
        /* Turn OFF Relay and LEDs */
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_PORT_PA, LED_PIN_PA5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB8, GPIO_PIN_RESET);
    }
}

/**
 * @brief ADC1 Initialization Function
 */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.OversamplingMode = DISABLE;
    HAL_ADC_Init(&hadc1);

    /** Configure Regular Channel: PA1 -> ADC1_IN2 */
    sConfig.Channel = ADC_CHANNEL_2;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_ADC12_CLK_ENABLE();

    /* Configure PA1 as Analog Input (LDR) */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure PA0 as Output Push-Pull (Relay, Default OFF / HIGH) */
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = RELAY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(RELAY_PORT, &GPIO_InitStruct);

    /* Configure PA5 as Output Push-Pull (User LED) */
    HAL_GPIO_WritePin(LED_PORT_PA, LED_PIN_PA5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PA5;
    HAL_GPIO_Init(LED_PORT_PA, &GPIO_InitStruct);

    /* Configure PB8 as Output Push-Pull (Alternative User LED) */
    HAL_GPIO_WritePin(LED_PORT_PB, LED_PIN_PB8, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = LED_PIN_PB8;
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
