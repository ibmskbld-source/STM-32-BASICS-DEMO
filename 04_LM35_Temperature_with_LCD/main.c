/**
 ******************************************************************************
 * @file           : 05_LM35_Temperature_with_LCD/main.c
 * @brief          : LM75 Digital Temperature Sensor (I2C2) with RG1602A-I2C(P)
 *                   LCD Display (I2C1) on Separate Hardware Pins (No Breadboard Needed!).
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Description & Dedicated Pin Connections:
 *
 *   1. RG1602A-I2C(P) LCD Display (Uses I2C1):
 *      - VCC -> 5V (or 3.3V)
 *      - GND -> GND
 *      - SCL -> PB6 (I2C1_SCL)
 *      - SDA -> PB7 (I2C1_SDA)
 *      - Default Address: 0x27 (0x4E) or 0x3F (0x7E)
 *
 *   2. LM75 Digital Temperature Sensor (Uses I2C2):
 *      - VCC -> 3.3V (or 5V)
 *      - GND -> GND
 *      - SCL -> PB10 (I2C2_SCL)
 *      - SDA -> PB11 (I2C2_SDA)
 *      - OS  -> Not connected
 *      - Default Address: 0x48 (0x90)
 *
 *   NOTE: Because I2C1 and I2C2 are separate hardware controllers on the STM32,
 *         each device plugs directly into its own header pins without needing
 *         a breadboard to split lines!
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"
#include <stdio.h>
#include <stdbool.h>

/* ==========================================================================
 * Configuration & Address Definitions
 * ========================================================================== */
#define LCD_DEFAULT_ADDR        (0x27 << 1) // 7-bit 0x27 -> 8-bit 0x4E (PCF8574T)
#define LCD_ALT_ADDR            (0x3F << 1) // 7-bit 0x3F -> 8-bit 0x7E (PCF8574AT)

#define LM75_DEFAULT_ADDR       (0x48 << 1) // 7-bit 0x48 -> 8-bit 0x90 (A0=A1=A2=GND)
#define LM75_TEMP_REG_POINTER   0x00        // Temperature register (read-only, 16-bit)
#define LM75_CONF_REG_POINTER   0x01        // Configuration register

/* Peripheral Handles for Separate I2C Buses */
I2C_HandleTypeDef hi2c1; // I2C1 for RG1602A LCD (PB6, PB7)
I2C_HandleTypeDef hi2c2; // I2C2 for LM75 Sensor (PB10, PB11)

/* Active I2C Addresses */
static uint8_t g_lcd_addr = LCD_DEFAULT_ADDR;
static uint8_t g_lm75_addr = LM75_DEFAULT_ADDR;

/* Private Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);

/* RG1602A-I2C(P) LCD Driver Prototypes (on I2C1) */
static void LCD_I2C_Write_Nibbles(uint8_t data, uint8_t rs_bit);
void LCD_Send_Cmd(uint8_t cmd);
void LCD_Send_Data(uint8_t data);
bool LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);

/* LM75 Sensor Driver Prototypes (on I2C2) */
bool LM75_Init(void);
bool LM75_Read_Temperature(float *temp_c);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock to 16 MHz (HSI) */
    SystemClock_Config();

    /* Initialize configured GPIO, I2C1 (LCD), and I2C2 (Sensor) */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();

    /* 1. Initialize RG1602A-I2C(P) LCD Display on I2C1 */
    bool lcd_found = LCD_Init();
    if (lcd_found)
    {
        LCD_Clear();
        LCD_SetCursor(0, 0);
        LCD_Print("STM32G431 DEMO");
        LCD_SetCursor(1, 0);
        LCD_Print("LM75 + RG1602A");
        HAL_Delay(2000);
        LCD_Clear();
    }

    /* 2. Initialize LM75 Digital Sensor on I2C2 */
    bool sensor_found = LM75_Init();
    if (lcd_found && !sensor_found)
    {
        LCD_SetCursor(0, 0);
        LCD_Print("LM75 NOT DETECT!");
        LCD_SetCursor(1, 0);
        LCD_Print("CHECK PB10/PB11");
        HAL_Delay(3000);
        LCD_Clear();
    }

    char line1_buf[17];
    char line2_buf[17];
    float temp_c = 0.0f;

    /* Infinite loop */
    while (1)
    {
        /* Read digital temperature from LM75 over I2C2 */
        if (LM75_Read_Temperature(&temp_c))
        {
            /* Line 1: "Temp: 25.50 \xDF C" (\xDF is HD44780 degree symbol) */
            snprintf(line1_buf, sizeof(line1_buf), "Temp: %.2f%cC   ", temp_c, 0xDF);
            LCD_SetCursor(0, 0);
            LCD_Print(line1_buf);

            /* Line 2: Thermal Status Descriptor */
            if (temp_c >= 35.0f) {
                snprintf(line2_buf, sizeof(line2_buf), "Status: HOT!   ");
            } else if (temp_c >= 28.0f) {
                snprintf(line2_buf, sizeof(line2_buf), "Status: WARM   ");
            } else if (temp_c <= 18.0f) {
                snprintf(line2_buf, sizeof(line2_buf), "Status: COLD   ");
            } else {
                snprintf(line2_buf, sizeof(line2_buf), "Status: NORMAL ");
            }
            LCD_SetCursor(1, 0);
            LCD_Print(line2_buf);
        }
        else
        {
            /* Sensor communication error display */
            LCD_SetCursor(0, 0);
            LCD_Print("Temp: --.- \xDF C  ");
            LCD_SetCursor(1, 0);
            LCD_Print("Status: COMM ERR");

            /* Attempt to re-probe the sensor */
            LM75_Init();
        }

        /* Refresh reading every 500 ms */
        HAL_Delay(500);
    }
}

/* ==========================================================================
 * LM75 Digital Temperature Sensor Driver Implementation (on I2C2)
 * ========================================================================== */

/**
 * @brief  Initializes and checks presence of LM75 on I2C2 (PB10/PB11).
 *         Auto-scans address range (0x48 to 0x4F) if default address does not respond.
 * @retval true if LM75 is responding, false otherwise.
 */
bool LM75_Init(void)
{
    /* Check default address on I2C2 */
    if (HAL_I2C_IsDeviceReady(&hi2c2, g_lm75_addr, 2, 50) == HAL_OK)
    {
        return true;
    }

    /* Auto-scan all possible LM75 addresses (0x48 .. 0x4F) */
    for (uint8_t addr = 0x48; addr <= 0x4F; addr++)
    {
        uint8_t shifted_addr = (addr << 1);
        if (HAL_I2C_IsDeviceReady(&hi2c2, shifted_addr, 2, 50) == HAL_OK)
        {
            g_lm75_addr = shifted_addr;
            return true;
        }
    }

    return false;
}

/**
 * @brief  Reads 16-bit temperature from LM75/LM75A sensor via I2C2.
 * @param  temp_c: Pointer to store calculated temperature in Celsius.
 * @retval true on successful I2C read, false on failure.
 */
bool LM75_Read_Temperature(float *temp_c)
{
    uint8_t raw_buf[2] = {0};

    /* Read 2 bytes from Pointer Register 0x00 (Temperature) over I2C2 */
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2,
                                                g_lm75_addr,
                                                LM75_TEMP_REG_POINTER,
                                                I2C_MEMADD_SIZE_8BIT,
                                                raw_buf,
                                                2,
                                                100);

    if (status != HAL_OK)
    {
        return false;
    }

    /* Combine MSB and LSB into a signed 16-bit value */
    int16_t raw_val = (int16_t)(((uint16_t)raw_buf[0] << 8) | raw_buf[1]);

    /* Convert to Celsius: raw_val / 256.0f */
    *temp_c = (float)raw_val / 256.0f;

    return true;
}

/* ==========================================================================
 * RG1602A-I2C(P) Character LCD Driver Implementation (on I2C1)
 * ========================================================================== */

/**
 * @brief  Sends a byte as two 4-bit nibbles over I2C1 (PB6/PB7) to PCF8574 backpack.
 */
static void LCD_I2C_Write_Nibbles(uint8_t data, uint8_t rs_bit)
{
    uint8_t high_nibble = data & 0xF0;
    uint8_t low_nibble  = (data << 4) & 0xF0;
    uint8_t buf[4];

    /* Backlight bit = 0x08, EN pulse high = 0x04, RS = rs_bit */
    buf[0] = high_nibble | 0x08 | rs_bit | 0x04; // EN HIGH
    buf[1] = high_nibble | 0x08 | rs_bit;        // EN LOW
    buf[2] = low_nibble  | 0x08 | rs_bit | 0x04; // EN HIGH
    buf[3] = low_nibble  | 0x08 | rs_bit;        // EN LOW

    HAL_I2C_Master_Transmit(&hi2c1, g_lcd_addr, buf, 4, 100);
}

void LCD_Send_Cmd(uint8_t cmd)
{
    LCD_I2C_Write_Nibbles(cmd, 0x00);
}

void LCD_Send_Data(uint8_t data)
{
    LCD_I2C_Write_Nibbles(data, 0x01);
}

/**
 * @brief  Initializes the RG1602A LCD in 4-bit mode using I2C1.
 */
bool LCD_Init(void)
{
    /* Check default LCD address on I2C1 (0x27) */
    if (HAL_I2C_IsDeviceReady(&hi2c1, g_lcd_addr, 2, 50) != HAL_OK)
    {
        /* Check alternate LCD address on I2C1 (0x3F) */
        if (HAL_I2C_IsDeviceReady(&hi2c1, LCD_ALT_ADDR, 2, 50) == HAL_OK)
        {
            g_lcd_addr = LCD_ALT_ADDR;
        }
        else
        {
            return false;
        }
    }

    HAL_Delay(50); // Power-on delay

    /* HD44780 4-bit Initialization Sequence */
    LCD_Send_Cmd(0x33);
    HAL_Delay(5);
    LCD_Send_Cmd(0x32);
    HAL_Delay(1);
    LCD_Send_Cmd(0x28); // 4-bit mode, 2-line display, 5x8 font
    HAL_Delay(1);
    LCD_Send_Cmd(0x0C); // Display ON, Cursor OFF, Blink OFF
    HAL_Delay(1);
    LCD_Send_Cmd(0x06); // Auto-increment cursor
    HAL_Delay(1);
    LCD_Send_Cmd(0x01); // Clear screen
    HAL_Delay(5);

    return true;
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

void LCD_Print(const char *str)
{
    while (*str)
    {
        LCD_Send_Data((uint8_t)*str++);
    }
}

/* ==========================================================================
 * STM32 Peripheral Initializations (I2C1 + I2C2)
 * ========================================================================== */

/**
 * @brief I2C1 Initialization (PB6 -> SCL, PB7 -> SDA, 100 kHz Standard Mode for LCD)
 */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00707CBB; // 100 kHz Standard Mode for 16MHz HSI
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

/**
 * @brief I2C2 Initialization (PB10 -> SCL, PB11 -> SDA, 100 kHz Standard Mode for LM75)
 */
static void MX_I2C2_Init(void)
{
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x00707CBB; // 100 kHz Standard Mode for 16MHz HSI
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c2);
}

/**
 * @brief GPIO Initialization
 *        - PB6 (I2C1_SCL) & PB7 (I2C1_SDA) -> LCD
 *        - PB10 (I2C2_SCL) & PB11 (I2C2_SDA) -> LM75 Sensor
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* I2C1: PB6 (SCL), PB7 (SDA) -> RG1602A LCD */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C2: PB10 (SCL), PB11 (SDA) -> LM75 Sensor */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
 * @brief System Clock Configuration (HSI 16 MHz)
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
