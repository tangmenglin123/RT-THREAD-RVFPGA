#ifndef BOARD_H__
#define BOARD_H__

#include <rtthread.h>

// Hardware Address Definitions
#define SegEn_ADDR      0x80001038
#define SegDig_ADDR     0x8000103C

#define GPIO_SWs        0x80001400
#define GPIO_LEDs       0x80001404
#define GPIO_INOUT      0x80001408 // Assuming this sets direction: 0 for input, 1 for output
// #define RGPIO_INTE      0x8000140C // Not used in this basic demo
// #define RGPIO_PTRIG     0x80001410 // Not used
// #define RGPIO_CTRL      0x80001418 // Not used
// #define RGPIO_INTS      0x8000141C // Not used

// #define RPTC_CNTR       0x80001200 // For OS Tick - Platform Specific
// #define RPTC_HRC        0x80001204 // For OS Tick
// #define RPTC_LRC        0x80001208 // For OS Tick
// #define RPTC_CTRL       0x8000120c // For OS Tick

// #define Select_INT      0x80001018 // Not used

// Helper Macros (optional, can be inlined in functions)
#define BOARD_READ_REG(dir) (*(volatile unsigned int *)(dir))
#define BOARD_WRITE_REG(dir, value) ((*(volatile unsigned int *)(dir)) = (value))

// BSP Function Prototypes
void rt_hw_board_init(void); // Standard RT-Thread board init function

// GPIO and Peripheral Control Functions
void bsp_led_write(rt_uint16_t value);
rt_uint16_t bsp_sw_read(void);
void bsp_seg_digit_write(rt_uint32_t value);
void bsp_seg_enable_write(rt_uint32_t value); // To control which 7-seg displays are active
void bsp_hw_uart_init(void); // Specific UART init for console

// This function is critical for RT-Thread's scheduler
// It needs to be called by the system's tick timer ISR
// void rt_tick_increase(void); // This is declared in rthw.h, you call it from ISR

#endif // BOARD_H__