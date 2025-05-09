/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>

#include "psp_types.h"
#include "bsp_mem_map.h"
#include "psp_interrupts_eh1.h"
#include "psp_api.h"
#include "bsp_printf.h"

// Enable RT_USING_HEAP
#define RT_USING_HEAP

// Enable memory management components
#define RT_USING_SMALL_MEM
#define RT_USING_MEMPOOL

// Enable IPC mechanisms
#define RT_USING_MESSAGEQUEUE
#define RT_USING_MUTEX
#define RT_USING_SEMAPHORE

// Enable software timers
#define RT_USING_TIMER_SOFT

// Enable console and set buffer size
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128

// Disable RT_USING_FINSH for small footprint
#undef RT_USING_FINSH

// Define main thread stack size and priority
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

// Set maximum thread priority
#define RT_THREAD_PRIORITY_MAX 32

// Set tick per second
#define RT_TICK_PER_SECOND 1000

// Enable floating point printf
#define RT_PRINTF_FLOAT

// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);

// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024 * 10
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 10(1024 * 10)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    /* Initialize system clock */

    // uart Init 
    uartInit();

    // rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
    
    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

void SysTick_Handler(void)
{
    /* enter interrupt */
    // rt_interrupt_enter();
    pspDisableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);

    rt_tick_increase();

    /* leave interrupt */
    //rt_interrupt_leave();
    pspEnableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);
}


void tick_init()
{
    pspInterruptsSetVectorTableAddress(&psp_vect_table);

    pspRegisterInterruptHandler(SysTick_Handler, E_MACHINE_TIMER_CAUSE);

    pspEnableInterruptNumberMachineLevel(D_PSP_INTERRUPTS_MACHINE_TIMER);
}