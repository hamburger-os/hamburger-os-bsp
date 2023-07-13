/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     lvhan       the first version
 */
#ifndef APPLICATIONS_COUPLER_CONTROLLER_COUPLER_CONTROLLER_H_
#define APPLICATIONS_COUPLER_CONTROLLER_COUPLER_CONTROLLER_H_

enum {
    LED_RUN = 0,
    LED_CAN2,
    LED_CAN1,
    LED_485,
    LED_DIO,
};

enum {
    MCU_DO1 = 0,
    MCU_DO2,
    MCU_DI1,
    MCU_DI2,
};

typedef struct
{
    char *station_devname;
    char *module_devname;
    char *adc_devname;
    char *led_devname[5];
    char *ctrl_devname[4];
    char *bat_devname[2];

    rt_device_t station_dev;
    rt_device_t module_dev;
    rt_adc_device_t adc_dev;
    rt_base_t led_pin[5];
    rt_base_t ctrl_pin[4];
    rt_base_t bat_pin[2];

    struct rt_messagequeue *rx_station_mq;
    struct rt_messagequeue *process_station_mq;

    struct rt_messagequeue *rx_module_mq;
    struct rt_messagequeue *process_module_mq;

    //车钩adc
    uint16_t adc[2];
    //图像测距模块
    uint8_t isopen;
    uint32_t distance_h;        //测距-远
    uint32_t distance_l;        //测距-近
    uint8_t logo;               //标识牌
    uint8_t out_hook;           //摘钩状态
    uint32_t timeout;           //回复超时ms

    int isThreadRun;
} CouplerCtrlUserData;

extern CouplerCtrlUserData coupler_controller_userdata;

void coupler_controller_dbinit(void);
void coupler_controller_pressureinit(void);
void coupler_controller_ledinit(void);
void coupler_controller_ctrlinit(void);
void coupler_controller_moduleinit(void);
void coupler_controller_batinit(void);

void set_device_addr(uint8_t addr);
uint8_t get_device_addr(void);
void module_ctrl_open(uint8_t isopen);
void ctrl_air_pressure(uint8_t onoff);

void coupler_controller_led_toggle(int pin);

#endif /* APPLICATIONS_COUPLER_CONTROLLER_COUPLER_CONTROLLER_H_ */
