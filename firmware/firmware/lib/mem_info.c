#include <stdio.h>

void mem_info(void) {
    extern char __ram_start;
    extern char __ram_size;
    extern char __ram_end;

    extern char __bss_start;
    extern char __bss_end;

    extern char __fiq_stack_top;
    extern char __irq_stack_top;
    extern char __svc_stack_top;

    extern char heap_low;
    extern char heap_top;

    printf("__ram_start     0x%08X\n", (unsigned int)&__ram_start);
    printf("__ram_size      0x%08X\n", (unsigned int)&__ram_size);
    printf("__ram_end       0x%08X\n", (unsigned int)&__ram_end);
    printf("__bss_start     0x%08X\n", (unsigned int)&__bss_start);
    printf("__bss_end       0x%08X\n", (unsigned int)&__bss_end);
    printf("__fiq_stack_top 0x%08X\n", (unsigned int)&__fiq_stack_top);
    printf("__irq_stack_top 0x%08X\n", (unsigned int)&__irq_stack_top);
    printf("__svc_stack_top 0x%08X\n", (unsigned int)&__svc_stack_top);
    printf("heap_low        0x%08X\n", (unsigned int)&heap_low);
    printf("heap_top        0x%08X\n", (unsigned int)&heap_top);
}
