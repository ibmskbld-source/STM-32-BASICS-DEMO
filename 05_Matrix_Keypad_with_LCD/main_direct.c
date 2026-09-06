/**
 ******************************************************************************
 * @file           : 06_Matrix_Keypad_with_LCD/main_direct.c
 * @brief          : Standalone 4x4 Matrix Keypad (C13..B1) with I2C LCD (PB2/PB10)
 *                   on SieCore 32 G431 (STM32G431CBT6).
 ******************************************************************************
 * Hardware Pin Connections on Header P1:
 *
 *   1. 4x4 Matrix Keypad (8-Pin Ribbon Connector plugged into top 8 pins of P1):
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

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 * Hardware Register Definitions (STM32G431)
 * ========================================================================== */
#define RCC_BASE            0x40021000UL
#define GPIOA_BASE          0x48000000UL
#define GPIOB_BASE          0x48000400UL
#define GPIOC_BASE          0x48000800UL
#define GPIOF_BASE          0x48001400UL

#define RCC_AHB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0x4CUL))

/* GPIOA */
#define GPIOA_MODER         (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_OTYPER        (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_OSPEEDR       (*(volatile uint32_t *)(GPIOA_BASE + 0x08UL))
#define GPIOA_PUPDR         (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))
#define GPIOA_IDR           (*(volatile uint32_t *)(GPIOA_BASE + 0x10UL))
#define GPIOA_BSRR          (*(volatile uint32_t *)(GPIOA_BASE + 0x18UL))

/* GPIOB */
#define GPIOB_MODER         (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_OTYPER        (*(volatile uint32_t *)(GPIOB_BASE + 0x04UL))
#define GPIOB_OSPEEDR       (*(volatile uint32_t *)(GPIOB_BASE + 0x08UL))
#define GPIOB_PUPDR         (*(volatile uint32_t *)(GPIOB_BASE + 0x0CUL))
#define GPIOB_IDR           (*(volatile uint32_t *)(GPIOB_BASE + 0x10UL))
#define GPIOB_BSRR          (*(volatile uint32_t *)(GPIOB_BASE + 0x18UL))

/* GPIOC */
#define GPIOC_MODER         (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_OTYPER        (*(volatile uint32_t *)(GPIOC_BASE + 0x04UL))
#define GPIOC_OSPEEDR       (*(volatile uint32_t *)(GPIOC_BASE + 0x08UL))
#define GPIOC_PUPDR         (*(volatile uint32_t *)(GPIOC_BASE + 0x0CUL))
#define GPIOC_IDR           (*(volatile uint32_t *)(GPIOC_BASE + 0x10UL))
#define GPIOC_BSRR          (*(volatile uint32_t *)(GPIOC_BASE + 0x18UL))

/* GPIOF */
#define GPIOF_MODER         (*(volatile uint32_t *)(GPIOF_BASE + 0x00UL))
#define GPIOF_OTYPER        (*(volatile uint32_t *)(GPIOF_BASE + 0x04UL))
#define GPIOF_OSPEEDR       (*(volatile uint32_t *)(GPIOF_BASE + 0x08UL))
#define GPIOF_PUPDR         (*(volatile uint32_t *)(GPIOF_BASE + 0x0CUL))
#define GPIOF_IDR           (*(volatile uint32_t *)(GPIOF_BASE + 0x10UL))
#define GPIOF_BSRR          (*(volatile uint32_t *)(GPIOF_BASE + 0x18UL))

extern void delay_ms(uint32_t ms);

/* LCD Bit-Bang Pins on GPIOB */
#define LCD_SCL_PIN         2   // PB2
#define LCD_SDA_PIN         10  // PB10

static uint8_t s_lcd_addr = 0x27;

/* Keypad Layout: 4 Rows x 4 Columns */
static const char KEY_MAP[4][4] = {
    {'1', '2', '3', 'A'}, // Row 1 (PC13)
    {'4', '5', '6', 'B'}, // Row 2 (PC15)
    {'7', '8', '9', 'C'}, // Row 3 (PF1)
    {'*', '0', '#', 'D'}  // Row 4 (PA1)
  // Col1  Col2 Col3 Col4
  //(PA3) (PA5) (PA7) (PB1)
};

static inline void i2c_delay(void)
{
    for (volatile int i = 0; i < 40; i++)
    {
        __asm__("nop");
    }
}

/* ==========================================================================
 * True Open-Drain Bit-Bang I2C for LCD (PB2 = SCL, PB10 = SDA)
 * ========================================================================== */

static inline void lcd_scl_high(void) { GPIOB_BSRR = (1UL << LCD_SCL_PIN); }
static inline void lcd_scl_low(void)  { GPIOB_BSRR = (1UL << (LCD_SCL_PIN + 16)); }
static inline void lcd_sda_high(void) { GPIOB_BSRR = (1UL << LCD_SDA_PIN); }
static inline void lcd_sda_low(void)  { GPIOB_BSRR = (1UL << (LCD_SDA_PIN + 16)); }
static inline int  lcd_sda_read(void) { return (GPIOB_IDR & (1UL << LCD_SDA_PIN)) ? 1 : 0; }

static void lcd_i2c_start(void)
{
    lcd_sda_high();
    lcd_scl_high();
    i2c_delay();
    lcd_sda_low();
    i2c_delay();
    lcd_scl_low();
    i2c_delay();
}

static void lcd_i2c_stop(void)
{
    lcd_sda_low();
    lcd_scl_low();
    i2c_delay();
    lcd_scl_high();
    i2c_delay();
    lcd_sda_high();
    i2c_delay();
}

static int lcd_i2c_write_byte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--)
    {
        if (byte & (1 << i)) {
            lcd_sda_high();
        } else {
            lcd_sda_low();
        }
        i2c_delay();
        lcd_scl_high();
        i2c_delay();
        lcd_scl_low();
        i2c_delay();
    }

    // Read ACK
    lcd_sda_high();
    i2c_delay();
    lcd_scl_high();
    i2c_delay();
    int ack = (lcd_sda_read() == 0) ? 0 : 1;
    lcd_scl_low();
    i2c_delay();

    return ack;
}

static int lcd_pcf8574_write(uint8_t data)
{
    lcd_i2c_start();
    if (lcd_i2c_write_byte(s_lcd_addr << 1) != 0)
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
    uint8_t base = (nibble & 0xF0) | 0x08 | (rs & 0x01); // Backlight ON = 0x08

    lcd_pcf8574_write(base | 0x04); // EN = HIGH
    i2c_delay();
    lcd_pcf8574_write(base);        // EN = LOW
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
    uint8_t addrs[] = {0x27, 0x3F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E};
    for (uint8_t i = 0; i < sizeof(addrs); i++)
    {
        lcd_i2c_start();
        if (lcd_i2c_write_byte(addrs[i] << 1) == 0)
        {
            s_lcd_addr = addrs[i];
            lcd_i2c_stop();
            break;
        }
        lcd_i2c_stop();
    }

    // HD44780 4-bit Initialization Sequence
    delay_ms(50);
    lcd_write_nibble(0x30, 0);
    delay_ms(10);
    lcd_write_nibble(0x30, 0);
    delay_ms(5);
    lcd_write_nibble(0x30, 0);
    delay_ms(5);
    lcd_write_nibble(0x20, 0); // 4-bit mode
    delay_ms(5);

    lcd_cmd(0x28); // 4-bit, 2 lines, 5x8
    delay_ms(2);
    lcd_cmd(0x08); // Display OFF
    delay_ms(2);
    lcd_cmd(0x01); // Display Clear
    delay_ms(5);
    lcd_cmd(0x06); // Entry mode: increment cursor
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
 * 4x4 Matrix Keypad Driver (Rows: PC13, PC15, PF1, PA1 | Cols: PA3, PA5, PA7, PB1)
 * ========================================================================== */

static void set_row(int row, int val)
{
    // val == 0 -> Drive LOW (Active)
    // val == 1 -> Drive HIGH (Idle)
    switch(row)
    {
        case 0: // PC13
            if (val) GPIOC_BSRR = (1UL << 13);
            else     GPIOC_BSRR = (1UL << (13 + 16));
            break;
        case 1: // PC15
            if (val) GPIOC_BSRR = (1UL << 15);
            else     GPIOC_BSRR = (1UL << (15 + 16));
            break;
        case 2: // PF1
            if (val) GPIOF_BSRR = (1UL << 1);
            else     GPIOF_BSRR = (1UL << (1 + 16));
            break;
        case 3: // PA1
            if (val) GPIOA_BSRR = (1UL << 1);
            else     GPIOA_BSRR = (1UL << (1 + 16));
            break;
    }
}

static int read_col(int col)
{
    // Returns 0 if pressed (LOW), 1 if released (HIGH)
    switch(col)
    {
        case 0: return (GPIOA_IDR & (1UL << 3)) ? 1 : 0; // PA3
        case 1: return (GPIOA_IDR & (1UL << 5)) ? 1 : 0; // PA5
        case 2: return (GPIOA_IDR & (1UL << 7)) ? 1 : 0; // PA7
        case 3: return (GPIOB_IDR & (1UL << 1)) ? 1 : 0; // PB1
    }
    return 1;
}

static char keypad_scan(void)
{
    // Ensure all rows are idle (HIGH)
    for (int r = 0; r < 4; r++) set_row(r, 1);
    i2c_delay();

    for (int r = 0; r < 4; r++)
    {
        set_row(r, 0); // Pull current row LOW
        i2c_delay();
        i2c_delay();

        for (int c = 0; c < 4; c++)
        {
            if (read_col(c) == 0) // Button contact detected!
            {
                delay_ms(15); // Debounce
                if (read_col(c) == 0)
                {
                    char key = KEY_MAP[r][c];
                    // Wait for release
                    while (read_col(c) == 0)
                    {
                        delay_ms(5);
                    }
                    delay_ms(15);
                    set_row(r, 1);
                    return key;
                }
            }
        }
        set_row(r, 1); // Restore row to HIGH
    }

    return '\0';
}

/* ==========================================================================
 * Application Entry Point
 * ========================================================================== */

int main(void)
{
    /* 1. Enable Clocks for GPIOA, GPIOB, GPIOC, GPIOF */
    RCC_AHB2ENR |= (1UL << 0) | (1UL << 1) | (1UL << 2) | (1UL << 5);

    /* 2. Configure LCD Pins (PB2 = SCL, PB10 = SDA) as Open-Drain Output with Pull-Up */
    GPIOB_MODER &= ~((3UL << 4) | (3UL << 20));
    GPIOB_MODER |=  ((1UL << 4) | (1UL << 20)); // Output (01b)
    GPIOB_OTYPER |= (1UL << 2) | (1UL << 10);    // Open-Drain
    GPIOB_PUPDR &= ~((3UL << 4) | (3UL << 20));
    GPIOB_PUPDR |=  ((1UL << 4) | (1UL << 20));  // Pull-up
    GPIOB_OSPEEDR |= ((3UL << 4) | (3UL << 20)); // High Speed
    GPIOB_BSRR = (1UL << 2) | (1UL << 10);       // Idle HIGH

    /* 3. Configure Keypad Rows (Outputs, Push-Pull, Default HIGH):
     *    - Row 1: PC13 (MODER[27:26] = 01b)
     *    - Row 2: PC15 (MODER[31:30] = 01b)
     *    - Row 3: PF1  (MODER[3:2]   = 01b)
     *    - Row 4: PA1  (MODER[3:2]   = 01b)
     */
    GPIOC_MODER &= ~((3UL << 26) | (3UL << 30));
    GPIOC_MODER |=  ((1UL << 26) | (1UL << 30));
    GPIOC_OTYPER &= ~((1UL << 13) | (1UL << 15));
    GPIOC_BSRR = (1UL << 13) | (1UL << 15);

    GPIOF_MODER &= ~(3UL << 2);
    GPIOF_MODER |=  (1UL << 2);
    GPIOF_OTYPER &= ~(1UL << 1);
    GPIOF_BSRR = (1UL << 1);

    GPIOA_MODER &= ~(3UL << 2);
    GPIOA_MODER |=  (1UL << 2);
    GPIOA_OTYPER &= ~(1UL << 1);
    GPIOA_BSRR = (1UL << 1);

    /* 4. Configure Keypad Columns (Inputs with Internal Pull-up):
     *    - Col 1: PA3 (MODER[7:6]   = 00b, PUPDR[7:6]   = 01b)
     *    - Col 2: PA5 (MODER[11:10] = 00b, PUPDR[11:10] = 01b)
     *    - Col 3: PA7 (MODER[15:14] = 00b, PUPDR[15:14] = 01b)
     *    - Col 4: PB1 (MODER[3:2]   = 00b, PUPDR[3:2]   = 01b)
     */
    GPIOA_MODER &= ~((3UL << 6) | (3UL << 10) | (3UL << 14)); // Inputs (00b)
    GPIOA_PUPDR &= ~((3UL << 6) | (3UL << 10) | (3UL << 14));
    GPIOA_PUPDR |=  ((1UL << 6) | (1UL << 10) | (1UL << 14)); // Pull-up (01b)

    GPIOB_MODER &= ~(3UL << 2); // Input (00b)
    GPIOB_PUPDR &= ~(3UL << 2);
    GPIOB_PUPDR |=  (1UL << 2);  // Pull-up (01b)

    /* 5. Initialize I2C LCD */
    lcd_init();
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("SIECORE32 G431");
    lcd_set_cursor(1, 0);
    lcd_print("4x4 KEYPAD DEMO");
    delay_ms(2000);
    lcd_clear();

    lcd_set_cursor(0, 0);
    lcd_print("Key: [None]     ");
    lcd_set_cursor(1, 0);
    lcd_print("Typed: ");

    char typed_buf[11] = "";
    uint8_t buf_idx = 0;

    /* 6. Main Scanning Loop */
    while (1)
    {
        char key = keypad_scan();

        if (key != '\0')
        {
            // Update Line 1: Last pressed key
            char key_str[17] = "Key: [' ']";
            key_str[6] = key;
            lcd_set_cursor(0, 0);
            lcd_print(key_str);
            lcd_print("     ");

            // Keypad Interaction
            if (key == '*')
            {
                // '*' Clears typed buffer
                buf_idx = 0;
                typed_buf[0] = '\0';
                lcd_set_cursor(1, 0);
                lcd_print("Typed:          ");
            }
            else if (key == '#')
            {
                // '#' adds a space or resets
                if (buf_idx < 9)
                {
                    typed_buf[buf_idx++] = ' ';
                    typed_buf[buf_idx] = '\0';
                    lcd_set_cursor(1, 7);
                    lcd_print(typed_buf);
                }
            }
            else
            {
                // Append character to buffer (up to 9 characters)
                if (buf_idx < 9)
                {
                    typed_buf[buf_idx++] = key;
                    typed_buf[buf_idx] = '\0';
                }
                else
                {
                    // Buffer full: shift left and append
                    for (int i = 0; i < 8; i++)
                    {
                        typed_buf[i] = typed_buf[i + 1];
                    }
                    typed_buf[8] = key;
                    typed_buf[9] = '\0';
                }
                lcd_set_cursor(1, 7);
                lcd_print(typed_buf);
            }
        }

        delay_ms(10);
    }

    return 0;
}
