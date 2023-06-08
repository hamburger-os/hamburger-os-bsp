#include "board.h"
#include <rtthread.h>

#define DBG_TAG "ds1682"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/*************************DS1682*************************************/
#ifdef  BSP_USING_DS1682
#define SAMPLE_SENSOR_DS1682       "ds1682"  /* 设备名称 */
struct __attribute__((packed)) DS1682DataDef
{
    rt_uint32_t times;
    rt_uint16_t count;
};
rt_device_t ds1682dev;
#endif

static void ds1682_echo_test(int argc, char *argv[])
{
#ifdef   BSP_USING_DS1682
    /* 查找历时芯片设备 */
    ds1682dev = rt_device_find(SAMPLE_SENSOR_DS1682);
    if (ds1682dev == RT_NULL)
    {
        LOG_E("ds1682dev find NULL.");
    }
    /* 以只读及轮询模式打开传感器设备 */
    rt_device_open(ds1682dev, RT_DEVICE_FLAG_RDONLY);
    struct DS1682DataDef ds1682data = {0};
    if (rt_device_read(ds1682dev, 0, &ds1682data, sizeof(ds1682data)) == 0)
    {
        LOG_D("time : %d s, count : %d", ds1682data.times, ds1682data.count);
    }
#endif	
}
MSH_CMD_EXPORT_ALIAS(ds1682_echo_test, ds1682_test, ds1682 read test.);

