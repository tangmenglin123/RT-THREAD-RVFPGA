#include "processing_thread.h"
#include "sensor_thread.h" // For struct sensor_data and sensor_data_mq
#include "board.h"         // For bsp_led_write
#include "display_thread.h" // For display_update_seg (optional)

#define PROCESSING_THREAD_PRIO      9 // Higher priority than sensor
#define PROCESSING_THREAD_STACK_SIZE 1024 // If doing more complex math
#define PROCESSING_THREAD_TIMESLICE 10

// If using Kalman from kalman_sample.c, you'd include its structures and functions here
// extern void kalman_init(KalmanFilter *kf);
// extern float kalman_update(KalmanFilter *kf, float measurement);
// static KalmanFilter kf_temp;

static rt_uint16_t current_led_state = 0x0000;

static void processing_thread_entry(void *parameter)
{
    struct sensor_data received_data;
    rt_kprintf("Processing thread started.\n");

    // kalman_init(&kf_temp); // Initialize if using Kalman

    while (1)
    {
        if (sensor_data_mq != RT_NULL)
        {
            rt_err_t result = rt_mq_recv(sensor_data_mq,
                                         &received_data,
                                         sizeof(struct sensor_data),
                                         RT_WAITING_FOREVER); // Block indefinitely
            if (result == RT_EOK)
            {
                // rt_kprintf("Processing: RX Temp %d, Hum %d\n", received_data.temperature, received_data.humidity);

                // Simple decision logic for LEDs
                // LED0 for high temperature
                if (received_data.temperature > 250) // If temp > 25.0 C
                {
                    current_led_state |= (1 << 0); // Turn on LED0
                }
                else
                {
                    current_led_state &= ~(1 << 0); // Turn off LED0
                }

                // LED1 for high humidity
                if (received_data.humidity > 70) // If humidity > 70%
                {
                    current_led_state |= (1 << 1); // Turn on LED1
                }
                else
                {
                    current_led_state &= ~(1 << 1); // Turn off LED1
                }
                bsp_led_write(current_led_state);

                // Update 7-segment display (optional)
                // Example: display temperature (integer part) on two 7-seg digits
                // This requires a function to convert int to 7-seg pattern
                // rt_uint8_t temp_display_val = (received_data.temperature / 10);
                // display_update_seg(some_conversion_to_7seg_pattern(temp_display_val));
            }
            else
            {
                rt_kprintf("Processing: Failed to receive from MQ, error %d\n", result);
            }
        }
        else
        {
            rt_thread_mdelay(1000); // MQ not ready, wait
        }
    }
}

int processing_thread_init(void)
{
    rt_thread_t tid = rt_thread_create("process",
                                       processing_thread_entry,
                                       RT_NULL,
                                       PROCESSING_THREAD_STACK_SIZE,
                                       PROCESSING_THREAD_PRIO,
                                       PROCESSING_THREAD_TIMESLICE);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    rt_kprintf("Failed to create processing thread.\n");
    return -RT_ERROR;
}