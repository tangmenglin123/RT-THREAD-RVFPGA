#include <rtthread.h>
#include "board.h" // For BSP functions like bsp_sw_read(), bsp_led_write()

// Include application thread headers
#include "application/display_thread.h"
#include "application/sensor_thread.h"
#include "application/processing_thread.h"

// External declarations for sample functions
extern int msgq_sample(void);
extern int thread_sample(void);
extern int mempool_sample(void);
extern int interrupt_sample(void);
extern int mutex_sample(void);
extern int kalman_sample(void);

// Global handles for dynamically created sample threads and demo threads
static rt_thread_t active_sample_thread = RT_NULL;
static rt_thread_t demo_sensor_thread_handle = RT_NULL;     // Store handle if sensor_thread_init returns it
static rt_thread_t demo_processing_thread_handle = RT_NULL; // Store handle

// Message Queue for the DEMO
#define SENSOR_MQ_MAX_MSGS 5
static rt_uint8_t sensor_mq_pool[sizeof(struct sensor_data) * SENSOR_MQ_MAX_MSGS];

// Helper to stop any currently running dynamic sample or demo threads
static void stop_active_threads_and_ipc(void)
{
    if (active_sample_thread != RT_NULL)
    {
        rt_kprintf("Stopping sample thread: %s\n", active_sample_thread->name);
        if (rt_thread_delete(active_sample_thread) != RT_EOK) { // Or rt_thread_detach if preferred and safe
            rt_kprintf("Failed to delete sample thread.\n"); // Deletion can be complex if thread is blocked on IPC
        }
        active_sample_thread = RT_NULL;
    }

    // For the demo threads, we assume sensor_thread_init and processing_thread_init
    // create and start them. To stop them, we'd need their handles.
    // For simplicity, this example doesn't store their handles to delete them.
    // A more robust system would store handles from _init functions and delete them.
    // For now, we'll just detach the message queue if it was for the demo.
    // Proper cleanup requires deleting/detaching threads that might use the MQ *before* detaching MQ.

    if (sensor_data_mq != RT_NULL)
    {
        rt_kprintf("Detaching DEMO sensor message queue.\n");
        // Ensure sensor_thread and processing_thread are stopped/deleted first if they are running!
        // This basic example doesn't track them for deletion properly from main.
        rt_err_t detach_res = rt_mq_detach(sensor_data_mq);
        if (detach_res != RT_EOK) {
            rt_kprintf("Failed to detach sensor_data_mq, error: %d\n", detach_res);
        }
        sensor_data_mq = RT_NULL;
    }
    // To properly stop demo_sensor_thread and demo_processing_thread, you'd need their tids
    // and delete them here. For instance, if _init functions returned tids:
    // if(demo_sensor_thread_handle) rt_thread_delete(demo_sensor_thread_handle);
    // if(demo_processing_thread_handle) rt_thread_delete(demo_processing_thread_handle);
}

int main(void)
{
    // rt_hw_board_init() is called by RT-Thread startup (_rt_hw_board_init).
    // No need to call it explicitly here if using standard RT-Thread startup.
    rt_kprintf("\n--- RT-Thread Nano DEMO for Nexys A7 ---\n");
    rt_kprintf("System Clock: %d Hz, Tick: %d Hz\n", RT_CPU_CLOCK_HZ, RT_TICK_PER_SECOND);


    // Initialize and start the permanent display thread
    if (display_thread_init() != RT_EOK)
    {
        rt_kprintf("FATAL: Failed to initialize display thread. Halting.\n");
        while(1);
    }

    rt_uint16_t sw_value = 0xFFFF, last_sw_value = 0xFFFF; // Initialize to ensure first read is processed

    while (1)
    {
        sw_value = bsp_sw_read();

        if (sw_value != last_sw_value)
        {
            rt_kprintf("Switch value changed: 0x%02X\n", sw_value);
            stop_active_threads_and_ipc(); // Stop previous dynamic threads/demo IPC

            bsp_led_write(0x0000); // Clear LEDs on mode change

            switch (sw_value)
            {
                case 0: // No sample/demo running
                    rt_kprintf("SW=0: All dynamic samples/demo stopped.\n");
                    // Display thread keeps running. LEDs will be controlled by it.
                    // To make LEDs fully off here, display_thread would need to be signaled.
                    break;

                case 1: // Smart Environment DEMO
                    rt_kprintf("SW=1: Starting Smart Environment DEMO...\n");
                    // Initialize message queue for the demo
                    if (rt_mq_init(&sensor_data_mq,
                                   "sensorMQ",
                                   &sensor_mq_pool[0],
                                   sizeof(struct sensor_data),
                                   sizeof(sensor_mq_pool),
                                   RT_IPC_FLAG_PRIO) != RT_EOK)
                    {
                        rt_kprintf("Error: Failed to initialize sensor_data_mq for DEMO.\n");
                        sensor_data_mq = RT_NULL; // Ensure it's marked as unusable
                    }
                    else
                    {
                        rt_kprintf("DEMO sensor_data_mq initialized.\n");
                        // Start demo threads (these functions create & start threads)
                        // A more robust way would be for _init to return thread handles for later management.
                        sensor_thread_init();
                        processing_thread_init();
                    }
                    break;

                case 2: // msgq_sample
                    rt_kprintf("SW=2: Starting Message Queue Sample...\n");
                    // The sample function itself creates threads. We need a wrapper or run it in a new thread.
                    // For simplicity, let's assume sample functions are designed to be called and they manage their own lifecycle or run to completion.
                    // If they create persistent threads, `active_sample_thread` should capture the main control thread if any.
                    // For samples that create multiple threads like msgq_sample, we can't easily capture "the" thread.
                    // A better approach for samples is that `xxx_sample()` is the entry point of ONE thread.
                    active_sample_thread = rt_thread_create("s_msgq", (void (*)(void*))msgq_sample, RT_NULL, 2048, 12, 10);
                    if (active_sample_thread) rt_thread_startup(active_sample_thread); else rt_kprintf("Failed to create msgq_sample thread\n");
                    break;

                case 4: // thread_sample (uses 0x02 for SWs)
                    rt_kprintf("SW=4: Starting Thread Sample...\n");
                    active_sample_thread = rt_thread_create("s_thrd", (void (*)(void*))thread_sample, RT_NULL, 1024, 12, 10);
                     if (active_sample_thread) rt_thread_startup(active_sample_thread); else rt_kprintf("Failed to create thread_sample thread\n");
                    break;

                case 8: // mempool_sample (uses 0x04 for SWs)
                    rt_kprintf("SW=8: Starting Memory Pool Sample...\n");
                    active_sample_thread = rt_thread_create("s_pool", (void (*)(void*))mempool_sample, RT_NULL, 1024, 12, 10);
                    if (active_sample_thread) rt_thread_startup(active_sample_thread); else rt_kprintf("Failed to create mempool_sample thread\n");
                    break;

                case 16: // interrupt_sample (uses 0x08 for SWs)
                    rt_kprintf("SW=16: Starting Interrupt Sample...\n");
                    active_sample_thread = rt_thread_create("s_intr", (void (*)(void*))interrupt_sample, RT_NULL, 1024, 12, 10);
                    if (active_sample_thread) rt_thread_startup(active_sample_thread); else rt_kprintf("Failed to create interrupt_sample thread\n");
                    break;

                case 32: // mutex_sample (uses 0x10 for SWs)
                    rt_kprintf("SW=32: Starting Mutex Sample...\n");
                    active_sample_thread = rt_thread_create("s_mutx", (void (*)(void*))mutex_sample, RT_NULL, 2048, 12, 10);
                    if (active_sample_thread) rt_thread_startup(active_sample_thread); else rt_kprintf("Failed to create mutex_sample thread\n");
                    break;

                case 64: // kalman_sample (uses 0x20 for SWs)
                    rt_kprintf("SW=64: Starting Kalman Filter Sample...\n");
                    active_sample_thread = rt_thread_create("s_klmn", (void (*)(void*))kalman_sample, RT_NULL, 2048, 12, 10);
                    if (active_sample_thread) rt_thread_startup(active_sample_thread); else rt_kprintf("Failed to create kalman_sample thread\n");
                    break;

                default:
                    rt_kprintf("SW=0x%02X: No action defined.\n", sw_value);
                    break;
            }
            last_sw_value = sw_value;
        }
        rt_thread_mdelay(200); // Poll switches every 200ms
    }
    // return 0; // Main should not return in an RTOS environment
}