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

void delay_ms(uint32_t ms)
{
    uint32_t start = s_ticks;
    while ((s_ticks - start) < ms);
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

    /* Configure SysTick for 1ms interrupts at 16 MHz HSI default clock */
    /* 16000000 / 1000 = 16000 */
    volatile uint32_t *SYST_CSR = (volatile uint32_t *)0xE000E010;
    volatile uint32_t *SYST_RVR = (volatile uint32_t *)0xE000E014;
    volatile uint32_t *SYST_CVR = (volatile uint32_t *)0xE000E018;

    *SYST_RVR = 16000 - 1;
    *SYST_CVR = 0;
    *SYST_CSR = 7; /* ENABLE (bit 0) | TICKINT (bit 1) | CLKSOURCE (bit 2) */

    /* Run main application */
    main();

    while (1);
}

void Default_Handler(void)
{
    while (1);
}
