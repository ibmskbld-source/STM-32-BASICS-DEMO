/**
 ******************************************************************************
 * @file           : 05_LM35_Temperature_with_LCD/main_direct.c
 * @brief          : Standalone Direct GPIO Bit-Bang Driver for RG1602A LCD (PB6/PB7)
 *                   and LM75 Sensor (PB10/PB11) on SieCore 32 G431 Board.
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/* ==========================================================================
 * Hardware Register Definitions (STM32G431)
 * ========================================================================== */
#define RCC_BASE            0x40021000UL
#define GPIOA_BASE          0x48000000UL
#define GPIOB_BASE          0x48000400UL
#define GPIOC_BASE          0x48000800UL

#define RCC_AHB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0x4CUL))

#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_BSRR          (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

#define GPIOB_MODER         (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_OTYPER        (*(volatile uint32_t *)(GPIOB_BASE + 0x04UL))
#define GPIOB_OSPEEDR       (*(volatile uint32_t *)(GPIOB_BASE + 0x08UL))
#define GPIOB_PUPDR         (*(volatile uint32_t *)(GPIOB_BASE + 0x0CUL))
#define GPIOB_IDR           (*(volatile uint32_t *)(GPIOB_BASE + 0x10UL))
#define GPIOB_BSRR          (*(volatile uint32_t *)(GPIOB_BASE + 0x18UL))

#define GPIOC_MODER         (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_BSRR          (*(volatile uint32_t *)(GPIOC_BASE + 0x18UL))

extern void delay_ms(uint32_t ms);

/* Device I2C Addresses */
static uint8_t s_lcd_addr = 0x27;
static uint8_t s_lm75_addr = 0x48;

/* Microsecond delay for ~40-80 kHz I2C bus clock */
static inline void i2c_delay(void)
{
    for (volatile int i = 0; i < 50; i++)
    {
        __asm__("nop");
    }
}

/* ==========================================================================
 * Bit-Bang I2C Driver for RG1602A LCD on PB6 (SCL) and PB7 (SDA)
 * ========================================================================== */

#define LCD_SCL_HIGH()      GPIOB_BSRR = (1UL << 6)
#define LCD_SCL_LOW()       GPIOB_BSRR = (1UL << (6 + 16))

#define LCD_SDA_HIGH()      GPIOB_BSRR = (1UL << 7)
#define LCD_SDA_LOW()       GPIOB_BSRR = (1UL << (7 + 16))
#define LCD_SDA_READ()      ((GPIOB_IDR & (1UL << 7)) != 0)

static void lcd_i2c_start(void)
{
    LCD_SDA_HIGH();
    LCD_SCL_HIGH();
    i2c_delay();
    LCD_SDA_LOW();
    i2c_delay();
    LCD_SCL_LOW();
    i2c_delay();
}

static void lcd_i2c_stop(void)
{
    LCD_SDA_LOW();
    LCD_SCL_LOW();
    i2c_delay();
    LCD_SCL_HIGH();
    i2c_delay();
    LCD_SDA_HIGH();
    i2c_delay();
}

static int lcd_i2c_write_byte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        if (byte & (1 << i)) {
            LCD_SDA_HIGH();
        } else {
            LCD_SDA_LOW();
        }
        i2c_delay();
        LCD_SCL_HIGH();
        i2c_delay();
        LCD_SCL_LOW();
        i2c_delay();
    }

    // Release SDA and read ACK
    LCD_SDA_HIGH();
    i2c_delay();
    LCD_SCL_HIGH();
    i2c_delay();
    int ack = LCD_SDA_READ() ? 1 : 0; // 0 = ACK, 1 = NACK
    LCD_SCL_LOW();
    i2c_delay();

    return ack;
}

static int lcd_pcf8574_write(uint8_t addr_7bit, uint8_t data)
{
    lcd_i2c_start();
    if (lcd_i2c_write_byte(addr_7bit << 1) != 0)
    {
        lcd_i2c_stop();
        return -1;
    }
    lcd_i2c_write_byte(data);
    lcd_i2c_stop();
    return 0;
}

static void lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t base = (nibble & 0xF0) | 0x08 | (rs & 0x01);

    // Pulse EN: HIGH then LOW
    lcd_pcf8574_write(s_lcd_addr, base | 0x04);
    i2c_delay();
    i2c_delay();
    lcd_pcf8574_write(s_lcd_addr, base);
    i2c_delay();
    i2c_delay();
}

static void lcd_write_byte(uint8_t data, uint8_t rs)
{
    lcd_write_nibble(data & 0xF0, rs);
    lcd_write_nibble((data << 4) & 0xF0, rs);
}

void lcd_cmd(uint8_t cmd)    { lcd_write_byte(cmd, 0); }
void lcd_data(uint8_t data)  { lcd_write_byte(data, 1); }

void lcd_init(void)
{
    delay_ms(100);

    // Auto-detect PCF8574 address
    uint8_t test_addrs[] = {0x27, 0x3F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E};
    s_lcd_addr = 0x27;
    for (uint8_t i = 0; i < sizeof(test_addrs); i++)
    {
        if (lcd_pcf8574_write(test_addrs[i], 0x08) == 0)
        {
            s_lcd_addr = test_addrs[i];
            break;
        }
    }

    // HD44780 4-bit Initialization Sequence
    delay_ms(50);
    lcd_write_nibble(0x30, 0);
    delay_ms(10);
    lcd_write_nibble(0x30, 0);
    delay_ms(5);
    lcd_write_nibble(0x30, 0);
    delay_ms(5);

    // Switch to 4-bit mode
    lcd_write_nibble(0x20, 0);
    delay_ms(5);

    lcd_cmd(0x28); // 4-bit mode, 2 lines, 5x8 font
    delay_ms(2);
    lcd_cmd(0x08); // Display OFF
    delay_ms(2);
    lcd_cmd(0x01); // Display Clear
    delay_ms(5);
    lcd_cmd(0x06); // Auto-increment cursor
    delay_ms(2);
    lcd_cmd(0x0C); // Display ON, Cursor OFF
    delay_ms(5);
}

void lcd_clear(void)
{
    lcd_cmd(0x01);
    delay_ms(3);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t offsets[] = {0x00, 0x40};
    lcd_cmd(0x80 | (col + offsets[row & 1]));
    delay_ms(1);
}

void lcd_print(const char *str)
{
    while (*str)
    {
        lcd_data((uint8_t)*str++);
    }
}

/* ==========================================================================
 * Open-Drain Bit-Bang I2C Driver for LM75 on PB10 (SCL) and PB11 (SDA)
 * ========================================================================== */

#define LM75_SCL_HIGH()     GPIOB_BSRR = (1UL << 10)
#define LM75_SCL_LOW()      GPIOB_BSRR = (1UL << (10 + 16))

#define LM75_SDA_HIGH()     GPIOB_BSRR = (1UL << 11)
#define LM75_SDA_LOW()      GPIOB_BSRR = (1UL << (11 + 16))
#define LM75_SDA_READ()     ((GPIOB_IDR & (1UL << 11)) != 0)

static void lm75_i2c_start(void)
{
    LM75_SDA_HIGH();
    LM75_SCL_HIGH();
    i2c_delay();
    LM75_SDA_LOW();
    i2c_delay();
    LM75_SCL_LOW();
    i2c_delay();
}

static void lm75_i2c_stop(void)
{
    LM75_SDA_LOW();
    LM75_SCL_LOW();
    i2c_delay();
    LM75_SCL_HIGH();
    i2c_delay();
    LM75_SDA_HIGH();
    i2c_delay();
}

static int lm75_i2c_write_byte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        if (byte & (1 << i)) {
            LM75_SDA_HIGH();
        } else {
            LM75_SDA_LOW();
        }
        i2c_delay();
        LM75_SCL_HIGH();
        i2c_delay();
        LM75_SCL_LOW();
        i2c_delay();
    }

    // Release SDA to let slave pull it LOW for ACK
    LM75_SDA_HIGH();
    i2c_delay();
    LM75_SCL_HIGH();
    i2c_delay();
    int ack = LM75_SDA_READ() ? 1 : 0; // 0 = ACK, 1 = NACK
    LM75_SCL_LOW();
    i2c_delay();

    return ack;
}

static uint8_t lm75_i2c_read_byte(int send_ack)
{
    uint8_t byte = 0;
    LM75_SDA_HIGH(); // Release SDA so slave can transmit

    for (int i = 7; i >= 0; i--)
    {
        i2c_delay();
        LM75_SCL_HIGH();
        i2c_delay();
        if (LM75_SDA_READ())
        {
            byte |= (1 << i);
        }
        LM75_SCL_LOW();
        i2c_delay();
    }

    // Master sends ACK (0) or NACK (1) to slave
    if (send_ack) {
        LM75_SDA_LOW();
    } else {
        LM75_SDA_HIGH();
    }
    i2c_delay();
    LM75_SCL_HIGH();
    i2c_delay();
    LM75_SCL_LOW();
    i2c_delay();
    LM75_SDA_HIGH(); // Release SDA line

    return byte;
}

/**
 * @brief Scans all 127 I2C addresses on PB10/PB11 to locate the sensor.
 * @retval Detected address, or 0 if no device found.
 */
uint8_t lm75_scan_bus(void)
{
    // First test preferred LM75 address range (0x48 .. 0x4F)
    for (uint8_t a = 0x48; a <= 0x4F; a++)
    {
        lm75_i2c_start();
        if (lm75_i2c_write_byte(a << 1) == 0)
        {
            lm75_i2c_stop();
            return a;
        }
        lm75_i2c_stop();
    }

    // Full scan of standard I2C addresses (0x08 .. 0x77)
    for (uint8_t a = 0x08; a <= 0x77; a++)
    {
        lm75_i2c_start();
        if (lm75_i2c_write_byte(a << 1) == 0)
        {
            lm75_i2c_stop();
            return a;
        }
        lm75_i2c_stop();
    }

    return 0; // No device responded
}

int lm75_read_temperature(float *temp_out)
{
    // If address not found, perform a quick probe
    lm75_i2c_start();
    if (lm75_i2c_write_byte(s_lm75_addr << 1) != 0)
    {
        lm75_i2c_stop();
        // Probe bus
        uint8_t detected = lm75_scan_bus();
        if (detected == 0) {
            return -1; // No device on bus
        }
        s_lm75_addr = detected;
        lm75_i2c_start();
        if (lm75_i2c_write_byte(s_lm75_addr << 1) != 0) {
            lm75_i2c_stop();
            return -1;
        }
    }

    // Set pointer to temperature register 0x00
    lm75_i2c_write_byte(0x00);
    lm75_i2c_stop();

    delay_ms(1);

    // Read 2 bytes
    lm75_i2c_start();
    if (lm75_i2c_write_byte((s_lm75_addr << 1) | 1) != 0)
    {
        lm75_i2c_stop();
        return -1;
    }

    uint8_t msb = lm75_i2c_read_byte(1); // Send ACK
    uint8_t lsb = lm75_i2c_read_byte(0); // Send NACK
    lm75_i2c_stop();

    int16_t raw = (int16_t)(((uint16_t)msb << 8) | lsb);
    *temp_out = (float)raw / 256.0f;
    return 0;
}

/* ==========================================================================
 * Application Entry Point
 * ========================================================================== */

int main(void)
{
    /* 1. Enable Clocks for GPIOA, GPIOB, GPIOC */
    RCC_AHB2ENR |= (1UL << 0) | (1UL << 1) | (1UL << 2);

    /* 2. Configure Indicator LEDs on PA1, PA5, PC13 as Output */
    GPIOA_MODER &= ~((3UL << 2) | (3UL << 10));
    GPIOA_MODER |=  ((1UL << 2) | (1UL << 10)); // PA1 & PA5 Output
    GPIOC_MODER &= ~(3UL << 26);
    GPIOC_MODER |=  (1UL << 26); // PC13 Output

    /* 3. Configure LCD Pins on PB6 (SCL) and PB7 (SDA) as Open-Drain Output with Pull-up */
    GPIOB_MODER &= ~((3UL << 12) | (3UL << 14));
    GPIOB_MODER |=  ((1UL << 12) | (1UL << 14)); // Output (01b)
    GPIOB_OTYPER |= ((1UL << 6) | (1UL << 7));   // Open-Drain
    GPIOB_OSPEEDR |= ((3UL << 12) | (3UL << 14)); // High Speed
    GPIOB_PUPDR &= ~((3UL << 12) | (3UL << 14));
    GPIOB_PUPDR |=  ((1UL << 12) | (1UL << 14)); // Internal Pull-up

    /* 4. Configure LM75 Pins on PB10 (SCL) and PB11 (SDA) as Open-Drain Output with Pull-up */
    GPIOB_MODER &= ~((3UL << 20) | (3UL << 22));
    GPIOB_MODER |=  ((1UL << 20) | (1UL << 22)); // Output (01b)
    GPIOB_OTYPER |= ((1UL << 10) | (1UL << 11)); // Open-Drain
    GPIOB_OSPEEDR |= ((3UL << 20) | (3UL << 22)); // High Speed
    GPIOB_PUPDR &= ~((3UL << 20) | (3UL << 22));
    GPIOB_PUPDR |=  ((1UL << 20) | (1UL << 22)); // Internal Pull-up

    /* Set default idle levels HIGH */
    GPIOB_BSRR = (1UL << 6) | (1UL << 7) | (1UL << 10) | (1UL << 11);

    /* 5. Initialize RG1602A LCD */
    lcd_init();
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("SIECORE32 G431");
    lcd_set_cursor(1, 0);
    lcd_print("LM75 + RG1602A");
    delay_ms(2000);
    lcd_clear();

    char line1[17];
    char line2[17];
    float temp_val = 0.0f;
    uint32_t toggle = 0;

    while (1)
    {
        // Blink indicator LEDs (Heartbeat)
        if (toggle & 1) {
            GPIOA_BSRR = (1UL << 1) | (1UL << 5);
            GPIOC_BSRR = (1UL << 13);
        } else {
            GPIOA_BSRR = (1UL << (1 + 16)) | (1UL << (5 + 16));
            GPIOC_BSRR = (1UL << (13 + 16));
        }
        toggle++;

        if (lm75_read_temperature(&temp_val) == 0)
        {
            // Format Line 1: Temp: XX.XX °C
            int int_part = (int)temp_val;
            int frac_part = (int)((temp_val >= 0 ? (temp_val - int_part) : (int_part - temp_val)) * 100);

            snprintf(line1, sizeof(line1), "Temp: %d.%02d%cC   ", int_part, frac_part, 0xDF);
            lcd_set_cursor(0, 0);
            lcd_print(line1);

            // Format Line 2: Status
            if (temp_val >= 35.0f) {
                snprintf(line2, sizeof(line2), "Status: HOT!   ");
            } else if (temp_val >= 28.0f) {
                snprintf(line2, sizeof(line2), "Status: WARM   ");
            } else if (temp_val <= 18.0f) {
                snprintf(line2, sizeof(line2), "Status: COLD   ");
            } else {
                snprintf(line2, sizeof(line2), "Status: NORMAL ");
            }
            lcd_set_cursor(1, 0);
            lcd_print(line2);
        }
        else
        {
            uint8_t probe = lm75_scan_bus();
            if (probe == 0) {
                lcd_set_cursor(0, 0);
                lcd_print("LM75 NOT FOUND  ");
                lcd_set_cursor(1, 0);
                lcd_print("CHECK B10/B11   ");
            } else {
                snprintf(line1, sizeof(line1), "Found: 0x%02X     ", probe);
                lcd_set_cursor(0, 0);
                lcd_print(line1);
                lcd_set_cursor(1, 0);
                lcd_print("READING ERROR   ");
            }
        }

        delay_ms(500);
    }

    return 0;
}
