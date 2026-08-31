/**
 ******************************************************************************
 * @file           : 04_I2C_LCD_Display/main.c
 * @brief          : I2C Character LCD (16x2 / 20x4 PCF8574) Demo for STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Connections:
 *   I2C LCD VCC -> 5V
 *   I2C LCD GND -> GND
 *   I2C LCD SCL -> PB8 (I2C1_SCL)
 *   I2C LCD SDA -> PB9 (I2C1_SDA)
 *
 * Messages Displayed in Code:
 *   Screen 1: "STM32G431 EMBEDDED" / "WELCOME STUDENTS"
 *   Screen 2: "DEPT OF ECE / CS"   / "MICROCONTROLLERS"
 *   Screen 3: "RUNTIME COUNTER"     / "Uptime: XX sec"
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdio.h>

#define LCD_I2C_ADDR (0x27 << 1) // Default PCF8574 Address (0x27 shifted = 0x4E)

/* Peripheral Handles */
I2C_HandleTypeDef hi2c1;

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

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

    /* Initialize LCD */
    LCD_Init();
    LCD_Clear();

    /* Demo Screen 1: Welcome */
    LCD_SetCursor(0, 0);
    LCD_Print("STM32G431 DEMO");
    LCD_SetCursor(1, 0);
    LCD_Print("WELCOME STUDENTS");
    HAL_Delay(3000);

    /* Demo Screen 2: Course Info */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("EMBEDDED SYSTEMS");
    LCD_SetCursor(1, 0);
    LCD_Print("I2C LCD RUNNING!");
    HAL_Delay(3000);

    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("SYSTEM STATUS:OK");

    uint32_t seconds = 0;
    char time_str[17];

    /* Infinite loop: Live Uptime Counter */
    while (1)
    {
        sprintf(time_str, "Uptime: %02lu:%02lu s ", seconds / 60, seconds % 60);
        LCD_SetCursor(1, 0);
        LCD_Print(time_str);

        seconds++;
        HAL_Delay(1000);
    }
}

/* ==========================================================================
 * I2C PCF8574 LCD Driver Functions
 * ========================================================================== */
static void LCD_I2C_Write_Nibbles(uint8_t data, uint8_t rs_bit)
{
    uint8_t high_nibble = data & 0xF0;
    uint8_t low_nibble = (data << 4) & 0xF0;
    uint8_t buf[4];

    // [D7 D6 D5 D4 Backlight(0x08) EN(0x04) RW(0) RS(rs_bit)]
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
    LCD_Send_Cmd(0x32); // 4-bit mode
    LCD_Send_Cmd(0x28); // 2 lines, 5x8 font
    LCD_Send_Cmd(0x0C); // Display ON, Cursor OFF
    LCD_Send_Cmd(0x06); // Auto increment cursor
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
    // Row 0 start: 0x80, Row 1 start: 0xC0 (0x80 + 0x40)
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    LCD_Send_Cmd(0x80 | (col + row_offsets[row & 3]));
}

void LCD_Print(char *str)
{
    while (*str) LCD_Send_Data((uint8_t)*str++);
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00707CBB; // 100kHz standard mode
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

    /* PB8 (SCL), PB9 (SDA) -> I2C1 AF4 */
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
