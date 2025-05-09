#ifndef SENSOR_THREAD_H__
#define SENSOR_THREAD_H__

#include <rtthread.h>

// Data structure for sensor readings
struct sensor_data
{
    rt_int16_t temperature; // e.g., value * 10 (255 means 25.5 C)
    rt_uint8_t humidity;    // e.g., percentage (0-100)
};

extern rt_mq_t sensor_data_mq; // Message queue handle

int sensor_thread_init(void);

#endif // SENSOR_THREAD_H__