#include <rtthread.h>
#include <stdlib.h>
#include <math.h>
#define THREAD_PRIORITY      6
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* 卡尔曼滤波参数 */
typedef struct {
    float x;      /* 状态估计值 */
    float P;      /* 估计协方差 */
    float Q;      /* 过程噪声协方差 */
    float R;      /* 测量噪声协方差 */
    float K;      /* 卡尔曼增益 */
} KalmanFilter;

static rt_thread_t tid = RT_NULL;

/* 初始化卡尔曼滤波器 */
static void kalman_init(KalmanFilter *kf)
{
    kf->x = 0.0f;          /* 初始状态 */
    kf->P = 1.0f;          /* 初始协方差 */
    kf->Q = 0.01f;         /* 过程噪声 */
    kf->R = 0.1f;          /* 测量噪声 */
}

/* 卡尔曼滤波更新 */
static float kalman_update(KalmanFilter *kf, float measurement)
{
    /* 预测步骤 */
    kf->P = kf->P + kf->Q;

    /* 更新步骤 */
    kf->K = kf->P / (kf->P + kf->R);           /* 计算卡尔曼增益 */
    kf->x = kf->x + kf->K * (measurement - kf->x); /* 更新状态估计 */
    kf->P = (1.0f - kf->K) * kf->P;            /* 更新协方差 */

    return kf->x;
}

/* 模拟传感器数据（带噪声） */
static float get_sensor_data(void)
{
    /* 模拟一个真实值在 10.0 附近，添加随机噪声 */
    float true_value = 10.0f;
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 2.0f; /* 噪声范围 [-1, 1] */
    return true_value + noise;
}

/* 卡尔曼滤波线程入口 */
static void kalman_thread_entry(void *parameter)
{
    KalmanFilter kf;
    kalman_init(&kf);
    rt_uint32_t count = 0;

    while (count < 100)
    {
        float measurement = get_sensor_data();
        float filtered = kalman_update(&kf, measurement);
        
        /* 将浮点数拆分为整数和小数部分 */
        int meas_int = (int)measurement;
        int meas_frac = (int)((measurement - meas_int) * 1000);
        int filt_int = (int)filtered;
        int filt_frac = (int)((filtered - filt_int) * 1000);
        
        rt_kprintf("Sample %d: Raw = %d.%03d, Filtered = %d.%03d\n", 
                   count, meas_int, meas_frac, filt_int, filt_frac);
        
        count++;
        rt_thread_mdelay(1000);
    }
    
    rt_kprintf("Kalman filter demo finished\n");
}

/* 卡尔曼滤波示例 */
int kalman_sample(void)
{
    rt_kprintf("\n卡尔曼滤波示例\n");
    
    /* 创建卡尔曼滤波线程 */
    tid = rt_thread_create("kalman",
                           kalman_thread_entry, RT_NULL,
                           THREAD_STACK_SIZE,
                           THREAD_PRIORITY, THREAD_TIMESLICE);
    
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    else
    {
        rt_kprintf("Create kalman thread failed\n");
        return -1;
    }
    
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(kalman_sample, kalman filter sample);