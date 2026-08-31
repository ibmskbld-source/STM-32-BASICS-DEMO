/**
 ******************************************************************************
 * @file           : 07_Parallel_LCD_16x4/main.c
 * @brief          : 16x4 Character LCD (HD44780 4-Bit Parallel) on STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Connections:
 *   LCD Pin 1  (VSS) -> GND
 *   LCD Pin 2  (VDD) -> 5V
 *   LCD Pin 3  (V0)  -> 10k Potentiometer wiper (Contrast adjustment)
 *   LCD Pin 4  (RS)  -> PC0 (Register Select: 0=Cmd, 1=Data)
 *   LCD Pin 5  (RW)  -> GND (Always Write Mode)
 *   LCD Pin 6  (E)   -> PC1 (Enable Clock Strobe)
 *   LCD Pins 7..10   -> (D0..D3) Leave Unconnected (in 4-bit mode)
 *   LCD Pin 11 (D4)  -> PC2
 *   LCD Pin 12 (D5)  -> PC3
 *   LCD Pin 13 (D6)  -> PC4
 *   LCD Pin 14 (D7)  -> PC5
 *   LCD Pin 15 (A)   -> 5V (Backlight Anode via 220 ohm resistor)
 *   LCD Pin 16 (K)   -> GND (Backlight Cathode)
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdio.h>

/* Pin Definitions on GPIOC */
#define LCD_PORT    GPIOC
#define PIN_RS      GPIO_PIN_0
#define PIN_EN      GPIO_PIN_1
#define PIN_D4      GPIO_PIN_2
#define PIN_D5      GPIO_PIN_3
#define PIN_D6      GPIO_PIN_4
#define PIN_D7      GPIO_PIN_5

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

void LCD_Parallel_Init(void);
void LCD_Parallel_Clear(void);
void LCD_Parallel_SetCursor(uint8_t row, uint8_t col);
void LCD_Parallel_Print(char *str);
void LCD_Parallel_Send(uint8_t value, uint8_t is_data);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* Initialize LCD in 4-bit HD44780 mode */
    LCD_Parallel_Init();
    LCD_Parallel_Clear();

    /* Screen 1: Master Classroom Header (All 4 Lines) */
    LCD_Parallel_SetCursor(0, 0);
    LCD_Parallel_Print("* STM32G431 DEV *");
    LCD_Parallel_SetCursor(1, 0);
    LCD_Parallel_Print("CLASS DEMO 2026 ");
    LCD_Parallel_SetCursor(2, 0);
    LCD_Parallel_Print("HD44780 4-BIT OK");
    LCD_Parallel_SetCursor(3, 0);
    LCD_Parallel_Print("READY FOR LAB!  ");
    HAL_Delay(4000);

    uint32_t count = 0;
    char line4_buf[17];

    /* Screen 2 & Running Loop */
    LCD_Parallel_Clear();
    LCD_Parallel_SetCursor(0, 0);
    LCD_Parallel_Print("EMBEDDED LAB #6 ");
    LCD_Parallel_SetCursor(1, 0);
    LCD_Parallel_Print("16x4 PARALLEL   ");
    LCD_Parallel_SetCursor(2, 0);
    LCD_Parallel_Print("DIRECT GPIO CTRL");

    while (1)
    {
        sprintf(line4_buf, "COUNT: %04lu SEC  ", count++);
        LCD_Parallel_SetCursor(3, 0);
        LCD_Parallel_Print(line4_buf);

        HAL_Delay(1000);
    }
}

/* ==========================================================================
 * HD44780 4-Bit Parallel Driver Functions
 * ========================================================================== */
static void LCD_Pulse_Enable(void)
{
    HAL_GPIO_WritePin(LCD_PORT, PIN_EN, GPIO_PIN_SET);
    for (volatile int i = 0; i < 300; i++); // ~2 microsecond strobe pulse
    HAL_GPIO_WritePin(LCD_PORT, PIN_EN, GPIO_PIN_RESET);
    for (volatile int i = 0; i < 300; i++);
}

static void LCD_Write4Bits(uint8_t nibble)
{
    HAL_GPIO_WritePin(LCD_PORT, PIN_D4, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_PORT, PIN_D5, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_PORT, PIN_D6, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_PORT, PIN_D7, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_Pulse_Enable();
}

void LCD_Parallel_Send(uint8_t value, uint8_t is_data)
{
    HAL_GPIO_WritePin(LCD_PORT, PIN_RS, is_data ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_Write4Bits(value >> 4);   // High Nibble
    LCD_Write4Bits(value & 0x0F); // Low Nibble
    HAL_Delay(1);
}

void LCD_Parallel_Init(void)
{
    HAL_Delay(50); // Power-on wait (>40ms)

    // HD44780 standard reset sequence
    LCD_Write4Bits(0x03);
    HAL_Delay(5);
    LCD_Write4Bits(0x03);
    HAL_Delay(1);
    LCD_Write4Bits(0x03);
    HAL_Delay(1);
    LCD_Write4Bits(0x02); // Enable 4-Bit Mode

    // Function Set: 4-bit, 2/4 display lines, 5x8 font
    LCD_Parallel_Send(0x28, 0);

    // Display ON, Cursor OFF, Blink OFF
    LCD_Parallel_Send(0x0C, 0);

    // Entry Mode: Auto increment cursor
    LCD_Parallel_Send(0x06, 0);

    // Clear Display
    LCD_Parallel_Send(0x01, 0);
    HAL_Delay(3);
}

void LCD_Parallel_Clear(void)
{
    LCD_Parallel_Send(0x01, 0);
    HAL_Delay(2);
}

/**
 * @brief  Sets cursor on 16x4 / 20x4 LCD.
 * @note   DDRAM Row offsets for 16x4:
 *         Row 0 = 0x00, Row 1 = 0x40, Row 2 = 0x10, Row 3 = 0x50
 */
void LCD_Parallel_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t row_offsets[] = {0x00, 0x40, 0x10, 0x50};
    LCD_Parallel_Send(0x80 | (col + row_offsets[row & 3]), 0);
}

void LCD_Parallel_Print(char *str)
{
    while (*str)
    {
        LCD_Parallel_Send((uint8_t)*str++, 1);
    }
}

/**
 * @brief GPIO Initialization (PC0..PC5 Output Push-Pull)
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(LCD_PORT, PIN_RS | PIN_EN | PIN_D4 | PIN_D5 | PIN_D6 | PIN_D7, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = PIN_RS | PIN_EN | PIN_D4 | PIN_D5 | PIN_D6 | PIN_D7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LCD_PORT, &GPIO_InitStruct);
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
