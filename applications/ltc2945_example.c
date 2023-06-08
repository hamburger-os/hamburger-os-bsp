#include "board.h"
#include <rtthread.h>

#define DBG_TAG "ltc2945"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/*************************LTC2945*************************************/
#ifdef BSP_USING_LTC2945
#define SAMPLE_SENSOR_LTC2945 "ltc2945" /* 设备名称 */
typedef struct
{
    rt_uint16_t vot_val;
    rt_uint16_t current_Val;
} ltc2945_5vot_current;
rt_device_t ltc2945dev;
#endif

static void ltc_2945_echo_test(int argc, char *argv[])
{
#ifdef BSP_USING_LTC2945
    /* 查找测5V电压电流设备 */
    ltc2945dev = rt_device_find(SAMPLE_SENSOR_LTC2945);
    if (ltc2945dev == RT_NULL)
    {
        LOG_E("ltc2945dev find NULL.");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(ltc2945dev, RT_DEVICE_FLAG_RDONLY);
    ltc2945_5vot_current ltc2945data = {0};
    if (rt_device_read(ltc2945dev, 0, &ltc2945data, sizeof(ltc2945data)) == 0)
    {
        LOG_D("5v = %d.%dV", ltc2945data.vot_val / 100, ltc2945data.vot_val % 100);
        LOG_D("5v_A = %d.%dA", ltc2945data.current_Val / 100, ltc2945data.current_Val % 100);
    }
#endif
}
MSH_CMD_EXPORT_ALIAS(ltc_2945_echo_test, ltc2945_test, ltc2945 read test.);
