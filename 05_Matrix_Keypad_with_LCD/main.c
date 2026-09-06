/**
 ******************************************************************************
 * @file           : 06_Matrix_Keypad_with_LCD/main.c
 * @brief          : 4x4 Matrix Keypad (Header P1: C13..B1) with I2C LCD
 * (PB2/PB10) on SieCore 32 G431 (STM32G431CBT6).
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Pin Connections on Header P1:
 *
 *   1. 4x4 Matrix Keypad (8-Pin Ribbon Connector on top 8 pins of Header P1):
 *      - Pin 1 -> PC13 (Row 1)
 *      - Pin 2 -> PC15 (Row 2)
 *      - Pin 3 -> PF1  (Row 3)
 *      - Pin 4 -> PA1  (Row 4)
 *      - Pin 5 -> PA3  (Col 1)
 *      - Pin 6 -> PA5  (Col 2)
 *      - Pin 7 -> PA7  (Col 3)
 *      - Pin 8 -> PB1  (Col 4)
 *
 *   2. RG1602A-I2C(P) LCD Display:
 *      - SCL -> PB2  (Header P1, right column pin 16 labeled 'B2')
 *      - SDA -> PB10 (Header P1, left column pin 17 labeled 'B10')
 *      - VCC -> +5V  (Header P3 top header labeled '+5V')
 *      - GND -> GND  (Header P1 bottom pin)
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LCD_DEFAULT_ADDR (0x27 << 1)

static uint8_t g_lcd_addr = LCD_DEFAULT_ADDR;

/* Keypad Matrix Definitions */
const char KEY_MAP[4][4] = {
    {'1', '2', '3', 'A'}, // Row 1 (PC13)
    {'4', '5', '6', 'B'}, // Row 2 (PC15)
    {'7', '8', '9', 'C'}, // Row 3 (PF1)
    {'*', '0', '#', 'D'}  // Row 4 (PA1)
    // Col1  Col2 Col3 Col4
    //(PA3) (PA5) (PA7) (PB1)
};

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
char Keypad_Scan(void);

void LCD_Send_Cmd(uint8_t cmd);
void LCD_Send_Data(uint8_t data);
bool LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);

/* Microsecond delay for software I2C */
static inline void i2c_delay(void) {
  for (volatile int i = 0; i < 40; i++) {
    __asm__("nop");
  }
}

/* ==========================================================================
 * Open-Drain Bit-Bang I2C Driver for LCD on PB2 (SCL) and PB10 (SDA)
 * ========================================================================== */
#define LCD_SCL_PIN 2
#define LCD_SDA_PIN 10

static inline void lcd_scl_high(void) { GPIOB->BSRR = (1UL << LCD_SCL_PIN); }
static inline void lcd_scl_low(void) {
  GPIOB->BSRR = (1UL << (LCD_SCL_PIN + 16));
}
static inline void lcd_sda_high(void) { GPIOB->BSRR = (1UL << LCD_SDA_PIN); }
static inline void lcd_sda_low(void) {
  GPIOB->BSRR = (1UL << (LCD_SDA_PIN + 16));
}
static inline int lcd_sda_read(void) {
  return (GPIOB->IDR & (1UL << LCD_SDA_PIN)) ? 1 : 0;
}

static void lcd_i2c_start(void) {
  lcd_sda_high();
  lcd_scl_high();
  i2c_delay();
  lcd_sda_low();
  i2c_delay();
  lcd_scl_low();
  i2c_delay();
}

static void lcd_i2c_stop(void) {
  lcd_sda_low();
  lcd_scl_low();
  i2c_delay();
  lcd_scl_high();
  i2c_delay();
  lcd_sda_high();
  i2c_delay();
}

static int lcd_i2c_write_byte(uint8_t byte) {
  for (int i = 7; i >= 0; i--) {
    if (byte & (1 << i))
      lcd_sda_high();
    else
      lcd_sda_low();
    i2c_delay();
    lcd_scl_high();
    i2c_delay();
    lcd_scl_low();
    i2c_delay();
  }

  lcd_sda_high();
  i2c_delay();
  lcd_scl_high();
  i2c_delay();
  int ack = (lcd_sda_read() == 0) ? 0 : 1;
  lcd_scl_low();
  i2c_delay();

  return ack;
}

static int lcd_pcf8574_write(uint8_t data) {
  lcd_i2c_start();
  if (lcd_i2c_write_byte(g_lcd_addr) != 0) {
    lcd_i2c_stop();
    return -1;
  }
  lcd_i2c_write_byte(data);
  lcd_i2c_stop();
  return 0;
}

static void lcd_write_nibble(uint8_t nibble, uint8_t rs) {
  uint8_t base = (nibble & 0xF0) | 0x08 | (rs & 0x01);
  lcd_pcf8574_write(base | 0x04);
  i2c_delay();
  lcd_pcf8574_write(base);
  i2c_delay();
}

static void lcd_write_byte(uint8_t data, uint8_t rs) {
  lcd_write_nibble(data & 0xF0, rs);
  lcd_write_nibble((data << 4) & 0xF0, rs);
}

void LCD_Send_Cmd(uint8_t cmd) { lcd_write_byte(cmd, 0); }
void LCD_Send_Data(uint8_t data) { lcd_write_byte(data, 1); }

bool LCD_Init(void) {
  HAL_Delay(100);

  // Probe 0x27 / 0x3F
  uint8_t addrs[] = {0x27, 0x3F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                     0x26, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E};
  for (uint8_t i = 0; i < sizeof(addrs); i++) {
    lcd_i2c_start();
    if (lcd_i2c_write_byte(addrs[i] << 1) == 0) {
      g_lcd_addr = (addrs[i] << 1);
      lcd_i2c_stop();
      break;
    }
    lcd_i2c_stop();
  }

  HAL_Delay(50);
  lcd_write_nibble(0x30, 0);
  HAL_Delay(5);
  lcd_write_nibble(0x30, 0);
  HAL_Delay(1);
  lcd_write_nibble(0x30, 0);
  HAL_Delay(1);
  lcd_write_nibble(0x20, 0);
  HAL_Delay(1);

  LCD_Send_Cmd(0x28);
  HAL_Delay(1);
  LCD_Send_Cmd(0x0C);
  HAL_Delay(1);
  LCD_Send_Cmd(0x06);
  HAL_Delay(1);
  LCD_Send_Cmd(0x01);
  HAL_Delay(5);

  return true;
}

void LCD_Clear(void) {
  LCD_Send_Cmd(0x01);
  HAL_Delay(3);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
  uint8_t offsets[] = {0x00, 0x40};
  LCD_Send_Cmd(0x80 | (col + offsets[row & 1]));
}

void LCD_Print(const char *str) {
  while (*str)
    LCD_Send_Data((uint8_t)*str++);
}

/* ==========================================================================
 * Keypad Functions
 * ========================================================================== */

static void set_row(int row, int val) {
  switch (row) {
  case 0: // PC13
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case 1: // PC15
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case 2: // PF1
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case 3: // PA1
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  }
}

static int read_col(int col) {
  switch (col) {
  case 0:
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET) ? 1 : 0;
  case 1:
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) ? 1 : 0;
  case 2:
    return (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_SET) ? 1 : 0;
  case 3:
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET) ? 1 : 0;
  }
  return 1;
}

char Keypad_Scan(void) {
  for (int r = 0; r < 4; r++)
    set_row(r, 1);
  i2c_delay();

  for (int r = 0; r < 4; r++) {
    set_row(r, 0); // Pull row LOW
    i2c_delay();

    for (int c = 0; c < 4; c++) {
      if (read_col(c) == 0) // Key pressed!
      {
        HAL_Delay(15); // Debounce
        if (read_col(c) == 0) {
          char key = KEY_MAP[r][c];
          while (read_col(c) == 0) {
            HAL_Delay(5);
          }
          HAL_Delay(15);
          set_row(r, 1);
          return key;
        }
      }
    }
    set_row(r, 1);
  }

  return '\0';
}

/* ==========================================================================
 * Application Entry Point
 * ========================================================================== */

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  LCD_Init();
  LCD_Clear();
  LCD_SetCursor(0, 0);
  LCD_Print("SIECORE32 G431");
  LCD_SetCursor(1, 0);
  LCD_Print("4x4 KEYPAD DEMO");
  HAL_Delay(2000);
  LCD_Clear();

  LCD_SetCursor(0, 0);
  LCD_Print("Key: [None]     ");
  LCD_SetCursor(1, 0);
  LCD_Print("Typed: ");

  char typed_buf[11] = "";
  uint8_t buf_idx = 0;

  while (1) {
    char key = Keypad_Scan();

    if (key != '\0') {
      char key_str[17] = "Key: [ ]";
      key_str[6] = key;
      LCD_SetCursor(0, 0);
      LCD_Print(key_str);
      LCD_Print("     ");

      if (key == '*') {
        buf_idx = 0;
        typed_buf[0] = '\0';
        LCD_SetCursor(1, 0);
        LCD_Print("Typed:          ");
      } else if (key == '#') {
        if (buf_idx < 9) {
          typed_buf[buf_idx++] = ' ';
          typed_buf[buf_idx] = '\0';
          LCD_SetCursor(1, 7);
          LCD_Print(typed_buf);
        }
      } else {
        if (buf_idx < 9) {
          typed_buf[buf_idx++] = key;
          typed_buf[buf_idx] = '\0';
        } else {
          for (int i = 0; i < 8; i++) {
            typed_buf[i] = typed_buf[i + 1];
          }
          typed_buf[8] = key;
          typed_buf[9] = '\0';
        }
        LCD_SetCursor(1, 7);
        LCD_Print(typed_buf);
      }
    }

    HAL_Delay(10);
  }
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /* 1. LCD Pins: PB2 (SCL), PB10 (SDA) -> Open-Drain Output */
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2 | GPIO_PIN_10, GPIO_PIN_SET);

  /* 2. Keypad Rows (Outputs, Default HIGH) */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  // Row 1 (PC13) & Row 2 (PC15)
  GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13 | GPIO_PIN_15, GPIO_PIN_SET);

  // Row 3 (PF1)
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_1, GPIO_PIN_SET);

  // Row 4 (PA1)
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);

  /* 3. Keypad Columns (Inputs with Internal Pull-up) */
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;

  // Col 1 (PA3), Col 2 (PA5), Col 3 (PA7)
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Col 4 (PB1)
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
}
