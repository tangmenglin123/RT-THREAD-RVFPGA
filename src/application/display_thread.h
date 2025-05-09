#ifndef DISPLAY_THREAD_H__
#define DISPLAY_THREAD_H__

#include <rtthread.h>

int display_thread_init(void);
void display_update_seg(rt_uint32_t seg_val); // Optional: function to allow other threads to update 7-seg

#endif // DISPLAY_THREAD_H__