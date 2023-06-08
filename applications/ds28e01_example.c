#include "board.h"
#include <rtthread.h>

#define DBG_TAG "ds28e01"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/******************************DS28E01******************************/
#ifdef BSP_USING_DS28E01
#define SAMPLE_SENSOR_DS28E01 "mag_ds28e01" /* 设备名称 */
rt_device_t ds28e01dev;
#endif
/*******************************************************************/

static void ds28e01_echo_test(int argc, char *argv[])
{
#ifdef BSP_USING_DS28E01
    /* 查找单总线28e01设备 */
    ds28e01dev = rt_device_find(SAMPLE_SENSOR_DS28E01);
    if (ds28e01dev == RT_NULL)
    {
        LOG_E("ds28e01dev find NULL.");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(ds28e01dev, RT_DEVICE_FLAG_RDONLY);
    rt_uint8_t SHA_ID[8] = {0};
    if (rt_device_control(ds28e01dev, RT_SENSOR_CTRL_GET_ID, SHA_ID) == 0)
    {
        LOG_D("id ： %x %x %x %x %x %x %x %x ", SHA_ID[0], SHA_ID[1], SHA_ID[2], SHA_ID[3], SHA_ID[4], SHA_ID[5], SHA_ID[6], SHA_ID[7]);
    }
#endif
}
MSH_CMD_EXPORT_ALIAS(ds28e01_echo_test, ds28e01_test, ds28e01 read test.);
