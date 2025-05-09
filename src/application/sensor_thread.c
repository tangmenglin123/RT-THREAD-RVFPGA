#include "sensor_thread.h"
#include <stdlib.h> // For rand()

#define SENSOR_THREAD_PRIO      10
#define SENSOR_THREAD_STACK_SIZE 512
#define SENSOR_THREAD_TIMESLICE 10

// Message queue handle, defined in main.c or a global place
rt_mq_t sensor_data_mq = RT_NULL;

static void sensor_thread_entry(void *parameter)
{
    struct sensor_data data;
    rt_kprintf("Sensor thread started.\n");

    // Seed random number generator (optional, do once)
    // srand(rt_tick_get()); // Using tick might not be very random if called early

    while (1)
    {
        // Simulate sensor data
        data.temperature = (rand() % 600) - 200; // Temp range: -20.0 to +39.9 C (scaled by 10)
        data.humidity = rand() % 101;             // Humidity: 0 to 100 %

        if (sensor_data_mq != RT_NULL)
        {
            rt_err_t result = rt_mq_send(sensor_data_mq, &data, sizeof(struct sensor_data));
            if (result != RT_EOK)
            {
                rt_kprintf("Sensor: Failed to send data to MQ, error %d\n", result);
            }
            else
            {
                // rt_kprintf("Sensor: Sent Temp %d, Hum %d\n", data.temperature, data.humidity);
            }
        }
        rt_thread_mdelay(2000); // Send data every 2 seconds
    }
}

int sensor_thread_init(void)
{
    rt_thread_t tid = rt_thread_create("sensor",
                                       sensor_thread_entry,
                                       RT_NULL,
                                       SENSOR_THREAD_STACK_SIZE,
                                       SENSOR_THREAD_PRIO,
                                       SENSOR_THREAD_TIMESLICE);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    rt_kprintf("Failed to create sensor thread.\n");
    return -RT_ERROR;
}