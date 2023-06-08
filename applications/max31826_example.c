#include "board.h"
#include <rtthread.h>

#define DBG_TAG "max31826"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/*************************MAX31826*************************************/
#ifdef BSP_USING_MAX31826
#define SAMPLE_SENSOR_MAX31826 "temp_max31826" /* 设备名称 */

rt_uint8_t max_ID[8] = {0};
rt_device_t max31826dev;
#endif

static void max31826_echo_test(int argc, char *argv[])
{
#ifdef BSP_USING_MAX31826
    /* 查找单总线MAX31826设备 */
    max31826dev = rt_device_find(SAMPLE_SENSOR_MAX31826);
    if (max31826dev == RT_NULL)
    {
        LOG_E("max31826dev find NULL.");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(max31826dev, RT_DEVICE_FLAG_RDONLY);
    rt_memset((void *)max_ID, 0x00, sizeof(max_ID));
    if (rt_device_control(max31826dev, RT_SENSOR_CTRL_GET_ID, max_ID) == 0)
    {
        LOG_D("id   : %x %x %x %x %x %x %x %x", max_ID[0], max_ID[1], max_ID[2], max_ID[3], max_ID[4], max_ID[5], max_ID[6], max_ID[7]);
    }
    struct rt_sensor_data max31826data = {0};
    if (rt_device_read(max31826dev, 0, &max31826data, sizeof(max31826data)) == 1)
    {
        LOG_D("temp : %d.%02d ℃", max31826data.data.temp / 10, max31826data.data.temp % 10);
    }
#endif
}
MSH_CMD_EXPORT_ALIAS(max31826_echo_test, max31826_test, max31826 read test.);
