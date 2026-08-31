/**
 ******************************************************************************
 * @file           : 06_Matrix_Keypad_with_LCD/main.c
 * @brief          : 4x4 Matrix Keypad with I2C LCD Display on STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Connections:
 *   Keypad Rows (R1..R4) -> PB0, PB1, PB2, PB3 (GPIO Output Push-Pull)
 *   Keypad Cols (C1..C4) -> PB4, PB5, PB6, PB7 (GPIO Input with Pull-Up)
 *
 *   I2C LCD VCC          -> 5V
 *   I2C LCD GND          -> GND
 *   I2C LCD SCL          -> PB8 (I2C1_SCL)
 *   I2C LCD SDA          -> PB9 (I2C1_SDA)
 *
 * Interactive Features:
 *   - Line 1 shows: "Last Key: ['X']"
 *   - Line 2 shows: "Input: [Typed text...]"
 *   - Pressing '*' clears the typed input buffer!
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdio.h>
#include <string.h>

#define LCD_I2C_ADDR (0x27 << 1)

/* Keypad Port & Pins */
#define KEYPAD_PORT GPIOB
const uint16_t ROW_PINS[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
const uint16_t COL_PINS[4] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};

const char KEY_MAP[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/* Peripheral Handles */
I2C_HandleTypeDef hi2c1;

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
void Keypad_Init(void);
char Keypad_Scan(void);

void LCD_Send_Cmd(uint8_t cmd);
void LCD_Send_Data(uint8_t data);
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(char *str);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    Keypad_Init();

    LCD_Init();
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("4x4 KEYPAD DEMO");
    LCD_SetCursor(1, 0);
    LCD_Print("Press Any Key...");
    HAL_Delay(2000);
    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_Print("Key: None");
    LCD_SetCursor(1, 0);
    LCD_Print("Input: ");

    char input_buffer[11] = ""; // Up to 10 typed characters
    uint8_t input_idx = 0;
    char line1[17];

    while (1)
    {
        char key = Keypad_Scan();
        if (key != '\0')
        {
            /* 1. Update Line 1 with pressed key */
            sprintf(line1, "Key: '%c'       ", key);
            LCD_SetCursor(0, 0);
            LCD_Print(line1);

            /* 2. Interactive typing behavior */
            if (key == '*')
            {
                /* Clear input on '*' */
                memset(input_buffer, 0, sizeof(input_buffer));
                input_idx = 0;
                LCD_SetCursor(1, 7);
                LCD_Print("         "); // Clear line 2 input area
            }
            else
            {
                if (input_idx < 9)
                {
                    input_buffer[input_idx++] = key;
                    input_buffer[input_idx] = '\0';
                }
            }

            /* 3. Refresh input line */
            LCD_SetCursor(1, 7);
            LCD_Print(input_buffer);
        }

        HAL_Delay(50);
    }
}

/**
 * @brief  Scans the 4x4 keypad matrix.
 * @return ASCII character of pressed key, or '\0' if none.
 */
char Keypad_Scan(void)
{
    for (int r = 0; r < 4; r++)
    {
        /* Drive current row LOW */
        HAL_GPIO_WritePin(KEYPAD_PORT, ROW_PINS[r], GPIO_PIN_RESET);

        for (int c = 0; c < 4; c++)
        {
            /* Check if column went LOW */
            if (HAL_GPIO_ReadPin(KEYPAD_PORT, COL_PINS[c]) == GPIO_PIN_RESET)
            {
                HAL_Delay(25); // Debounce
                while (HAL_GPIO_ReadPin(KEYPAD_PORT, COL_PINS[c]) == GPIO_PIN_RESET); // Wait for release
                HAL_GPIO_WritePin(KEYPAD_PORT, ROW_PINS[r], GPIO_PIN_SET); // Reset row
                return KEY_MAP[r][c];
            }
        }

        /* Drive row back HIGH */
        HAL_GPIO_WritePin(KEYPAD_PORT, ROW_PINS[r], GPIO_PIN_SET);
    }

    return '\0';
}

/**
 * @brief Keypad GPIO Initialization
 */
void Keypad_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure Row Pins (PB0..PB3) as Output */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(KEYPAD_PORT, &GPIO_InitStruct);

    /* Set all rows HIGH initially */
    for (int r = 0; r < 4; r++)
    {
        HAL_GPIO_WritePin(KEYPAD_PORT, ROW_PINS[r], GPIO_PIN_SET);
    }

    /* Configure Column Pins (PB4..PB7) as Inputs with Pull-Up */
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEYPAD_PORT, &GPIO_InitStruct);
}

/* ==========================================================================
 * I2C PCF8574 LCD Driver Implementation
 * ========================================================================== */
static void LCD_I2C_Write_Nibbles(uint8_t data, uint8_t rs_bit)
{
    uint8_t high_nibble = data & 0xF0;
    uint8_t low_nibble = (data << 4) & 0xF0;
    uint8_t buf[4];

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
    LCD_Send_Cmd(0x28);
    LCD_Send_Cmd(0x0C);
    LCD_Send_Cmd(0x06);
    LCD_Send_Cmd(0x01);
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

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00707CBB;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PB8 (SCL), PB9 (SDA) -> I2C1 Alternate Function */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
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
