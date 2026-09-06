#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
void Default_Handler(void);
int main(void);

void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms; i++)
    {
        for (volatile uint32_t j = 0; j < 1600; j++)
        {
            __asm__("nop");
        }
    }
}

__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    0, 0, 0, 0,
    Default_Handler,
    Default_Handler,
    0,
    Default_Handler,
    Default_Handler,
};

void Reset_Handler(void)
{
    uint32_t *pSrc = &_sidata;
    uint32_t *pDst = &_sdata;
    while (pDst < &_edata) {
        *pDst++ = *pSrc++;
    }

    pDst = &_sbss;
    while (pDst < &_ebss) {
        *pDst++ = 0;
    }

    volatile uint32_t *SCB_VTOR = (volatile uint32_t *)0xE000ED08;
    *SCB_VTOR = 0x08000000;

    main();

    while (1);
}

void Default_Handler(void)
{
    while (1);
}
