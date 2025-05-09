#include "display_thread.h"
#include "board.h" // For bsp_seg_digit_write, bsp_led_write

#define DISPLAY_THREAD_PRIO      15 // Lower priority (higher number)
#define DISPLAY_THREAD_STACK_SIZE 512
#define DISPLAY_THREAD_TIMESLICE 10

static rt_uint32_t seven_segment_value = 0x02111254; // Default: Your student ID

static void display_thread_entry(void *parameter)
{
    rt_uint32_t count = 0;
    rt_kprintf("Display thread started.\n");

    bsp_seg_digit_write(seven_segment_value); // Initial display

    while (1)
    {
        // LED Toggling based on count
        if (count % 2 == 0)
        {
            bsp_led_write(0xAAAA);
        }
        else
        {
            bsp_led_write(0x5555);
        }
        count++;

        // Update 7-segment if needed (could be controlled by a message or global var)
        // bsp_seg_digit_write(seven_segment_value); // uncomment if it changes

        rt_thread_mdelay(1000); // Blink LEDs every 1 second
    }
}

// Optional: function for other threads to change the 7-seg display
void display_update_seg(rt_uint32_t seg_val)
{
    // Consider using a mutex if `seven_segment_value` is accessed by display_thread_entry too
    seven_segment_value = seg_val;
    bsp_seg_digit_write(seven_segment_value); // Update immediately or let thread pick it up
}

int display_thread_init(void)
{
    rt_thread_t tid = rt_thread_create("display",
                                       display_thread_entry,
                                       RT_NULL,
                                       DISPLAY_THREAD_STACK_SIZE,
                                       DISPLAY_THREAD_PRIO,
                                       DISPLAY_THREAD_TIMESLICE);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    rt_kprintf("Failed to create display thread.\n");
    return -RT_ERROR;
}