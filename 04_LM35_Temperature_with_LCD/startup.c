#include <stdint.h>

/* Linker symbols */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
void Default_Handler(void);
void SysTick_Handler(void);
int main(void);

/* SysTick Counter */
static volatile uint32_t s_ticks = 0;

void SysTick_Handler(void)
{
    s_ticks++;
}

/* Reliable calibrated cycle delay for 16 MHz HSI clock (approx 1ms per loop) */
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

/* Vector Table */
__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler, /* NMI */
    Default_Handler, /* HardFault */
    Default_Handler, /* MemManage */
    Default_Handler, /* BusFault */
    Default_Handler, /* UsageFault */
    0, 0, 0, 0,
    Default_Handler, /* SVCall */
    Default_Handler, /* DebugMon */
    0,
    Default_Handler, /* PendSV */
    SysTick_Handler, /* SysTick */
};

void Reset_Handler(void)
{
    /* Copy data section from Flash to RAM */
    uint32_t *pSrc = &_sidata;
    uint32_t *pDst = &_sdata;
    while (pDst < &_edata) {
        *pDst++ = *pSrc++;
    }

    /* Zero out BSS section */
    pDst = &_sbss;
    while (pDst < &_ebss) {
        *pDst++ = 0;
    }

    /* Set Vector Table Offset Register (VTOR) to Flash base */
    volatile uint32_t *SCB_VTOR = (volatile uint32_t *)0xE000ED08;
    *SCB_VTOR = 0x08000000;

    /* Enable FPU (Full Access to CP10 and CP11) in CPACR */
    volatile uint32_t *SCB_CPACR = (volatile uint32_t *)0xE000ED88;
    *SCB_CPACR |= (0xF << 20);

    /* Run main application */
    main();

    while (1);
}

void Default_Handler(void)
{
    while (1);
}
