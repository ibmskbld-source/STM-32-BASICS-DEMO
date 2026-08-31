/**
 ******************************************************************************
 * @file           : 05_LM35_Temperature_with_LCD/main.c
 * @brief          : LM35 Temperature Sensor with I2C LCD Display on STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Connections:
 *   LM35 Pin 1 (Vs)   -> 5V (or 3.3V)
 *   LM35 Pin 2 (Vout) -> PA2 (ADC1_IN3)
 *   LM35 Pin 3 (GND)  -> GND
 *
 *   I2C LCD VCC       -> 5V
 *   I2C LCD GND       -> GND
 *   I2C LCD SCL       -> PB8 (I2C1_SCL)
 *   I2C LCD SDA       -> PB9 (I2C1_SDA)
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdio.h>

/* I2C LCD Definitions */
#define LCD_I2C_ADDR (0x27 << 1) // Default PCF8574 I2C address (0x27 shifted = 0x4E)

/* Peripheral Handles */
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;

/* Private function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);

/* I2C LCD Driver Functions */
void LCD_Send_Cmd(uint8_t cmd);
void LCD_Send_Data(uint8_t data);
void LCD_Init(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(char *str);
void LCD_Clear(void);

/* LM35 Sensor Function */
float LM35_Read_Temperature(void);

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
    MX_I2C1_Init();

    /* Run ADC Calibration */
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

    /* Initialize I2C LCD */
    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("STM32G431 DEMO");
    LCD_SetCursor(1, 0);
    LCD_Print("LM35 THERMOMETER");
    HAL_Delay(2000);
    LCD_Clear();

    char line1_buf[17];
    char line2_buf[17];

    /* Infinite loop */
    while (1)
    {
        /* 1. Read temperature in Celsius */
        float temp_c = LM35_Read_Temperature();

        /* 2. Format Line 1: "Temp: 26.5 \xDF C" (\xDF is degree symbol on HD44780) */
        sprintf(line1_buf, "Temp: %.1f%cC   ", temp_c, 0xDF);
        LCD_SetCursor(0, 0);
        LCD_Print(line1_buf);

        /* 3. Format Line 2: Status descriptor */
        if (temp_c > 35.0f) {
            sprintf(line2_buf, "Status: HOT!   ");
        } else if (temp_c < 20.0f) {
            sprintf(line2_buf, "Status: COLD   ");
        } else {
            sprintf(line2_buf, "Status: NORMAL ");
        }
        LCD_SetCursor(1, 0);
        LCD_Print(line2_buf);

        HAL_Delay(500); // Refresh twice every second
    }
}

/**
 * @brief  Samples LM35 with 16-sample software averaging and converts to °C.
 * @formula Vout (mV) = (ADC_Avg * 3300) / 4095
 *          Temp (°C) = Vout / 10.0 mV
 */
float LM35_Read_Temperature(void)
{
    uint32_t sum = 0;
    const uint8_t SAMPLES = 16;

    for (int i = 0; i < SAMPLES; i++)
    {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        sum += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(5);
    }

    float avg_adc = (float)sum / (float)SAMPLES;
    float voltage_mv = (avg_adc * 3300.0f) / 4095.0f; // 3.3V reference
    float temp_c = voltage_mv / 10.0f;                // 10mV = 1 deg C

    return temp_c;
}

/* ==========================================================================
 * I2C PCF8574 LCD Driver Implementation
 * ========================================================================== */
static void LCD_I2C_Write_Nibbles(uint8_t data, uint8_t rs_bit)
{
    uint8_t high_nibble = data & 0xF0;
    uint8_t low_nibble = (data << 4) & 0xF0;
    uint8_t buf[4];

    // Backlight on = 0x08, EN high = 0x04, RS = rs_bit
    buf[0] = high_nibble | 0x08 | rs_bit | 0x04;
    buf[1] = high_nibble | 0x08 | rs_bit;
    buf[2] = low_nibble  | 0x08 | rs_bit | 0x04;
    buf[3] = low_nibble  | 0x08 | rs_bit;

    HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, buf, 4, 100);
}

void LCD_Send_Cmd(uint8_t cmd)   { LCD_I2C_Write_Nibbles(cmd, 0x00); }
void LCD_Send_Data(uint8_t data) { LCD_I2C_Write_Nibbles(data, 0x01); }

void LCD_Init(void)
{
    HAL_Delay(50);
    LCD_Send_Cmd(0x33);
    LCD_Send_Cmd(0x32);
    LCD_Send_Cmd(0x28); // 4-bit mode, 2 lines, 5x8 font
    LCD_Send_Cmd(0x0C); // Display ON, cursor OFF
    LCD_Send_Cmd(0x06); // Auto-increment cursor
    LCD_Send_Cmd(0x01); // Clear display
    HAL_Delay(5);
}

void LCD_Clear(void)
{
    LCD_Send_Cmd(0x01);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t row_offsets[] = {0x00, 0x40};
    LCD_Send_Cmd(0x80 | (col + row_offsets[row & 1]));
}

void LCD_Print(char *str)
{
    while (*str) LCD_Send_Data((uint8_t)*str++);
}

/**
 * @brief ADC1 Initialization (PA2 -> ADC1_IN3)
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
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    HAL_ADC_Init(&hadc1);

    sConfig.Channel = ADC_CHANNEL_3; // PA2
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5; // High sampling stability
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

/**
 * @brief I2C1 Initialization (PB8 -> SCL, PB9 -> SDA)
 */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00707CBB; // 100 kHz Standard Mode
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PA2 Analog Input (LM35) */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB8 (SCL), PB9 (SDA) -> I2C1 Alternate Function */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
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
