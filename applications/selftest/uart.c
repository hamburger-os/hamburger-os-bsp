/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-17     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#define DBG_TAG "uart"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static char * error_log[] = {
    "UART2      ----> UART2     ",
    "UART3      ----> UART4     ",
    "UART4      ----> UART3     ",
};

#define BAUD_CONFIG         BAUD_RATE_115200    //波特率
#define PARITY_CONFIG       PARITY_NONE         //0无校验1奇校验2偶校验
#define DATA_BITS_CONFIG    DATA_BITS_8         //数据位
#define STOP_BITS_CONFIG    STOP_BITS_1         //停止位

#define BUFFER_LEN  256

void selftest_uart_test(SelftestlUserData *puserdata)
{
    uint8_t data_wr[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t data_rd[16];
    uint16_t data_rd_len = 0;

    for (int i = 0; i<3; i++)
    {
        /* step1：查找串口设备 */
        puserdata->uart_dev[i][0] = rt_device_find(puserdata->uart_devname[i][0]);
        puserdata->uart_dev[i][1] = rt_device_find(puserdata->uart_devname[i][1]);

        struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
        /* step2：修改串口配置参数 */
        config.baud_rate = BAUD_CONFIG;         // 修改波特率为 115200
        config.data_bits = DATA_BITS_CONFIG;    // 数据位 8
        config.stop_bits = STOP_BITS_CONFIG;    // 停止位 1
        config.parity    = PARITY_CONFIG;       // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
        config.rx_bufsz  = BUFFER_LEN;
        config.tx_bufsz  = BUFFER_LEN;
#endif
#ifdef RT_USING_SERIAL_V1
        config.bufsz     = BUFFER_LEN;
#endif

        /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
        rt_device_control(puserdata->uart_dev[i][0], RT_DEVICE_CTRL_CONFIG, &config);

        /* step4：打开串口设备。以非阻塞接收和阻塞发送模式打开串口设备 */
#ifdef RT_USING_SERIAL_V2
        if (rt_device_open(puserdata->uart_dev[i][0], RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
        if (rt_device_open(puserdata->uart_dev[i][0], RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
        {
            LOG_E("open uart '%s' failed", puserdata->uart_devname[i][0]);
        }
    }

    for (int i = 0; i<3; i++)
    {
        //尝试8次，有1次成功即为成功
        int n = 0;
        for (; n<8; n++)
        {
            rt_thread_delay(10);
            rt_device_write(puserdata->uart_dev[i][0], 0, data_wr, 8);
            rt_thread_delay(100);

            rt_memset(data_rd, 0, 16);
            data_rd_len = rt_device_read(puserdata->uart_dev[i][1], 0, data_rd, 16);
            if (data_rd_len < 1)
            {
                LOG_E("%s read failed %d", error_log[i], data_rd_len);
            }
            if (rt_memcmp(data_rd, data_wr, 8) == 0)
            {
                LOG_D("%s pass", error_log[i]);
                break;
            }
        }
        if (n == 8)
        {
            LOG_E("%s error!", error_log[i]);
            LOG_HEX("rd", 16, data_rd, 16);
        }
//        LOG_W("%s %d", error_log[i], n);
    }

    for (int i = 0; i<3; i++)
    {
        rt_device_close(puserdata->uart_dev[i][0]);
    }
}
