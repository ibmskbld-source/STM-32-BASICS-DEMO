/**
 ******************************************************************************
 * @file           : 08_Seven_Segment_4Digit/main.c
 * @brief          : 4-Digit 7-Segment Multiplexed Display Demo on STM32G431
 * @author         : Embedded Systems Classroom Demo
 ******************************************************************************
 * Hardware Connections (Common Cathode Display):
 *   Segment A  -> PA4  (via 220 ohm resistor)
 *   Segment B  -> PA5  (via 220 ohm resistor)
 *   Segment C  -> PA6  (via 220 ohm resistor)
 *   Segment D  -> PA7  (via 220 ohm resistor)
 *   Segment E  -> PA8  (via 220 ohm resistor)
 *   Segment F  -> PA9  (via 220 ohm resistor)
 *   Segment G  -> PA10 (via 220 ohm resistor)
 *   Segment DP -> PA11 (via 220 ohm resistor)
 *
 *   Digit 1 (DIG1) -> PB12 (Common Cathode: Active-LOW)
 *   Digit 2 (DIG2) -> PB13 (Common Cathode: Active-LOW)
 *   Digit 3 (DIG3) -> PB14 (Common Cathode: Active-LOW)
 *   Digit 4 (DIG4) -> PB15 (Common Cathode: Active-LOW)
 *
 * Messages Displayed:
 *   - Pattern 1: "1234" (Fixed Number Test)
 *   - Pattern 2: "dE-0" (Text pattern: "Demo")
 *   - Pattern 3: Real-time Live 4-Digit Decimal Counter (0000 -> 9999)
 ******************************************************************************
 */

#include "stm32g4xx_hal.h"

/* Segment Port: GPIOA (Pins PA4 to PA11) */
#define SEG_PORT    GPIOA

/* Digit Port: GPIOB (Pins PB12 to PB15) */
#define DIG_PORT    GPIOB
const uint16_t DIG_PINS[4] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15};

/* Common Cathode Segment Lookup Table for Digits 0 to 9 (Bit order: DP G F E D C B A) */
const uint8_t DIGIT_LUT[10] = {
    0x3F, // 0: 0011 1111
    0x06, // 1: 0000 0110
    0x5B, // 2: 0101 1011
    0x4F, // 3: 0100 1111
    0x66, // 4: 0110 0110
    0x6D, // 5: 0110 1101
    0x7D, // 6: 0111 1101
    0x07, // 7: 0000 0111
    0x7F, // 8: 0111 1111
    0x6F  // 9: 0110 1111
};

/* Special Character Patterns: 'd', 'E', '-', '0' */
#define CHAR_d  0x5E
#define CHAR_E  0x79
#define CHAR_DASH 0x40
#define CHAR_0  0x3F

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void SevenSeg_Display_Raw(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint16_t duration_ms);
void SevenSeg_Display_Number(uint16_t num, uint16_t duration_ms);

/**
 * @brief  The application entry point.
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* Demo 1: Show "1234" for 3 seconds */
    SevenSeg_Display_Number(1234, 3000);

    /* Demo 2: Show "dE-0" (DEMO) for 3 seconds */
    SevenSeg_Display_Raw(CHAR_d, CHAR_E, CHAR_DASH, CHAR_0, 3000);

    uint16_t counter = 0;

    /* Demo 3: Live 4-digit counting loop */
    while (1)
    {
        /* Refresh current number for ~200ms per count step */
        SevenSeg_Display_Number(counter, 200);

        counter++;
        if (counter > 9999) counter = 0;
    }
}

/**
 * @brief  Displays raw 8-bit segment patterns on the 4 digits using POV multiplexing.
 * @param  d0..d3: 8-bit segment bytes for digits 0 to 3.
 * @param  duration_ms: Total time to sustain display in milliseconds.
 */
void SevenSeg_Display_Raw(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint16_t duration_ms)
{
    uint8_t patterns[4] = {d0, d1, d2, d3};
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < duration_ms)
    {
        for (int d = 0; d < 4; d++)
        {
            /* 1. Turn OFF all digits to eliminate ghosting (Set HIGH for Common Cathode) */
            for (int i = 0; i < 4; i++)
            {
                HAL_GPIO_WritePin(DIG_PORT, DIG_PINS[i], GPIO_PIN_SET);
            }

            /* 2. Write 8 segment bits to PA4..PA11 */
            uint8_t seg = patterns[d];
            for (int b = 0; b < 8; b++)
            {
                HAL_GPIO_WritePin(SEG_PORT, (GPIO_PIN_4 << b), (seg & (1 << b)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }

            /* 3. Turn ON active digit (Drive LOW for Common Cathode) */
            HAL_GPIO_WritePin(DIG_PORT, DIG_PINS[d], GPIO_PIN_RESET);

            /* 4. Persistence of vision delay (~2ms per digit = ~125 Hz refresh rate) */
            HAL_Delay(2);
        }
    }
}

/**
 * @brief  Decomposes a 4-digit decimal number (0-9999) and displays it.
 */
void SevenSeg_Display_Number(uint16_t num, uint16_t duration_ms)
{
    uint8_t d0 = DIGIT_LUT[(num / 1000) % 10]; // Thousands
    uint8_t d1 = DIGIT_LUT[(num / 100) % 10];  // Hundreds
    uint8_t d2 = DIGIT_LUT[(num / 10) % 10];   // Tens
    uint8_t d3 = DIGIT_LUT[num % 10];          // Units

    SevenSeg_Display_Raw(d0, d1, d2, d3, duration_ms);
}

/**
 * @brief GPIO Initialization Function
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure Segments PA4..PA11 as Output Push-Pull */
    HAL_GPIO_WritePin(SEG_PORT, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
                                GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
                          GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SEG_PORT, &GPIO_InitStruct);

    /* Configure Digits PB12..PB15 as Output Push-Pull (Default HIGH / OFF) */
    HAL_GPIO_WritePin(DIG_PORT, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(DIG_PORT, &GPIO_InitStruct);
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
