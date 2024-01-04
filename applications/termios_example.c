/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-26     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <pthread.h>

#include <sys/time.h>
#include <sys/select.h>

#define DBG_TAG "termios"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DEV_NAME BSP_TERMIOS_EXAMPLE_DEVNAME

#define DEBUG_PRINTF LOG_D
#define ERROR_PRINTF LOG_E
#define WARNING_PRINTF LOG_W

#define BAUD_CONFIG_BASE    B115200 //波特率基准
#define BAUD_CONFIG         28800   //波特率
#define PARITY_CONFIG       0       //0无校验1奇校验2偶校验
#define DATA_BITS_CONFIG    8       //数据位
#define STOP_BITS_CONFIG    1       //停止位

#define BUFFER_LEN 256

#define serial_struct_enable 0

static int tty_fd = -1;
static int thread_is_run = 0;
static pthread_t termios_thread = 0;
void *termios_echo_thread(void *arg)
{
    int rv = -1;
    int drv_rv = -1;
    char r_buf[BUFFER_LEN];
    struct termios options;
    fd_set rset;
#if serial_struct_enable == 1
    struct serial_struct ss, ss_set;
#endif

    /* 打开串口设备 */
    tty_fd = open(DEV_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK, 0);
    if (tty_fd < 0)
    {
        ERROR_PRINTF("open tty '%s' failed : %s", DEV_NAME, strerror(errno));
        goto cleanup;
    }
    DEBUG_PRINTF("open tty '%s' sucessful!", DEV_NAME);

    /* 获取原有的串口属性的配置 */
    memset(&options, 0, sizeof(options));
    rv = tcgetattr(tty_fd, &options);
    if (rv != 0)
    {
        ERROR_PRINTF("tcgetattr() failed : %s", strerror(errno));
        goto cleanup;
    }

    /* 设置串口属性的配置 */
    options.c_cflag |= (CLOCAL | CREAD);//Enable in default.
    /*
    *Set baud rate. e.g B9600 is 9600 bauds.
    */
    cfsetispeed(&options, BAUD_CONFIG_BASE);//input speed
    cfsetospeed(&options, BAUD_CONFIG_BASE);//output speed
    /*
    *Set parity.
    */
#if PARITY_CONFIG == 0
    options.c_cflag &= ~PARENB;  //Disable parity.
#elif PARITY_CONFIG == 1
    options.c_cflag |= PARENB;   //Enable parity.
    options.c_cflag |= PARODD;   //Odd parity.
#else
    options.c_cflag |= PARENB;   //Enable parity.
    options.c_cflag &= ~PARODD;  //even parity.
#endif
    /*
    *Set data bits.
    */
    options.c_cflag &= ~CSIZE;
#if DATA_BITS_CONFIG == 5
    options.c_cflag |= CS5;//5位数据位
#elif DATA_BITS_CONFIG == 6
    options.c_cflag |= CS6;//6位数据位
#elif DATA_BITS_CONFIG == 7
    options.c_cflag |= CS7;//7位数据位
#elif DATA_BITS_CONFIG == 8
    options.c_cflag |= CS8;//8位数据位
#endif
    /*
    *Set stop bits.
    */
#if STOP_BITS_CONFIG == 1
    options.c_cflag &= ~ CSTOPB;//1位停止位
#elif STOP_BITS_CONFIG == 2
    options.c_cflag |= CSTOPB;//2位停止位
#endif


    tcflush(tty_fd, TCIFLUSH);
    if( tcsetattr(tty_fd, TCSANOW, &options) != 0)
    {
        ERROR_PRINTF("set tty config failed : %s", strerror(errno));
        goto cleanup;
    }
#if serial_struct_enable == 1
    if((ioctl(tty_fd, TIOCGSERIAL, &ss)) < 0)
    {
        ERROR_PRINTF("get serial_struct error : %s", strerror(errno));
        goto cleanup;
    }
    ss.flags &= ~ASYNC_SPD_MASK;
    ss.flags |= ASYNC_SPD_CUST;
    ss.custom_divisor = ss.baud_base / BAUD_CONFIG;
    if((ioctl(tty_fd, TIOCSSERIAL, &ss)) < 0)
    {
        ERROR_PRINTF("set serial_struct error: %s", strerror(errno));
        goto cleanup;
    }
    ioctl(tty_fd, TIOCGSERIAL , &ss_set);
    DEBUG_PRINTF("success set baud to %d, custom_divisor = %d, baud_base = %d",
                BAUD_CONFIG, ss_set.custom_divisor, ss_set.baud_base);
#endif

    while (thread_is_run)
    {
        FD_ZERO(&rset);
        FD_SET(tty_fd, &rset);

        struct timeval tv = {1, 0}; // 超时时间
        rv = select(tty_fd + 1, &rset, NULL, NULL, &tv);
        if (rv < 0)
        {
            ERROR_PRINTF("select() failed : %s", strerror(errno));
            goto cleanup;
        }
        else if (rv == 0)
        {
//            WARNING_PRINTF("select() time out!");
        }
        else
        {
            if(FD_ISSET(tty_fd, &rset))
            {
                ioctl(tty_fd, FIONREAD, &drv_rv);

                /* check poll and ioctl */
                assert(drv_rv != 0);

                memset(r_buf, 0, sizeof(r_buf));
                rv = read(tty_fd, r_buf, drv_rv);
                if(rv != drv_rv)
                {
                    ERROR_PRINTF("read error : %s", strerror(errno));
                    goto cleanup;
                }
                else
                {
                    DEBUG_PRINTF("read : (%d)%s", rv, r_buf);
                    /* echo test */
                    rv = write(tty_fd, r_buf, drv_rv);
                    if(rv != drv_rv)
                    {
                        ERROR_PRINTF("write error : %s", strerror(errno));
                        goto cleanup;
                    }
                    else
                    {
                        DEBUG_PRINTF("write : (%d)%s", rv, r_buf);
                    }
                }
            }
        }
    }

cleanup:
    termios_thread = 0;
    close(tty_fd);
    DEBUG_PRINTF("thread exited!");
    return 0;
}

static void termios_echo_test(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: termiosechotest [cmd]\n");
        rt_kprintf("       termiosechotest --start\n");
        rt_kprintf("       termiosechotest --write [data]\n");
        rt_kprintf("       termiosechotest --stop\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--start") == 0)
        {
            if (termios_thread == 0)
            {
                struct pthread_attr pthread_attr_t;
                pthread_attr_init(&pthread_attr_t);

                pthread_attr_t.stacksize = 2048;
                pthread_attr_t.schedparam.sched_priority = 30;
                int ret = pthread_create(&termios_thread, &pthread_attr_t, termios_echo_thread, NULL);
                if (ret != 0)
                {
                    rt_kprintf("pthread_create error %d!\n", ret);
                }
                else
                {
                    thread_is_run = 1;
                }
            }
            else
            {
                rt_kprintf("thread already exists!\n");
            }
        }
        else if (rt_strcmp(argv[1], "--write") == 0)
        {
            if (tty_fd != -1)
            {
                int data_len = strlen(argv[2]);
                int write_len = 0;
                write_len = write(tty_fd, argv[2], data_len);
                if(write_len != data_len)
                {
                    rt_kprintf("write error : %s\n", strerror(errno));
                }
                else
                {
                    rt_kprintf("write : (%d)%s\n", write_len, argv[2]);
                }
            }
            else
            {
                rt_kprintf("device does not exist!\n");
            }
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            termios_thread = 0;
            thread_is_run = 0;
        }
        else
        {
            rt_kprintf("cmd does not exist!\n");
            rt_kprintf("Usage: termiosechotest [cmd]\n");
            rt_kprintf("       termiosechotest --start\n");
            rt_kprintf("       termiosechotest --write [data]\n");
            rt_kprintf("       termiosechotest --stop\n");
        }
    }
}
#ifdef RT_USING_FINSH
#include <finsh.h>
#ifdef RT_USING_FINSH
    MSH_CMD_EXPORT_ALIAS(termios_echo_test, termiosechotest, termios echo test);
#endif /* RT_USING_FINSH */
#endif /* RT_USING_FINSH */
