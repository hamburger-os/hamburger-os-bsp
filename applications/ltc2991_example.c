#include "board.h"
#include <rtthread.h>

#define DBG_TAG "ltc2991"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/*************************LTC2991*************************************/
#ifdef BSP_USING_LTC2991
#define SAMPLE_SENSOR_LTC2991 "ltc2991" /* 设备名称 */
rt_device_t ltc2991dev;
#endif

static void ltc_2991_echo_test(int argc, char *argv[])
{
#ifdef BSP_USING_LTC2991
    /* 查找测5V电压电流设备 */
    ltc2991dev = rt_device_find(SAMPLE_SENSOR_LTC2991);
    if (ltc2991dev == RT_NULL)
    {
        LOG_E("ltc2991dev find NULL.");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(ltc2991dev, RT_DEVICE_FLAG_RDONLY);
    rt_uint16_t ltc2991data[2] = {0};
    if (rt_device_read(ltc2991dev, 0, &ltc2991data, sizeof(ltc2991data)) == 0)
    {
        LOG_D("5v = %d.%dV", (ltc2991data[0] * 2) / 1000, (ltc2991data[0] * 2) % 1000);
        LOG_D("3.3v = %d.%dV", (ltc2991data[1] * 2) / 1000, (ltc2991data[1] * 2) % 1000);
    }
#endif
}
MSH_CMD_EXPORT_ALIAS(ltc_2991_echo_test, ltc2991_test, ltc2991 read test.);
