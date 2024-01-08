/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-05     lvhan       the first version
 */
#ifndef APPLICATIONS_TPX86_VOICE_CTRL_TPX86_VOICE_CTRL_H_
#define APPLICATIONS_TPX86_VOICE_CTRL_TPX86_VOICE_CTRL_H_

enum {
    LED_CTL1 = 0,
    LED_CTL2,
    LED_CTL3,
    LED_CTL4,
};

enum {
    PLAY_CTL0 = 0,
    PLAY_CTL1,
};

enum {
    VOICE_CT1 = 0,
    VOICE_CT2,
    VOICE_CT3,
    VOICE_ADD,
    VOICE_SUB,
};

enum {
    CTL_UART2 = 0,
    CTL_UART3,
    CTL_UART4,
    CTL_UART5,
    CTL_UART6,
};

enum {
    CTL_CAN1 = 0,
    CTL_CAN2,
};

typedef union __attribute__ ((packed))
{
    uint8_t value;
    struct
    {
        uint8_t I0 : 1;
        uint8_t I1 : 1;
        uint8_t I2 : 1;
        uint8_t I3 : 1;
        uint8_t I4 : 1;
        uint8_t I5 : 1;
        uint8_t I6 : 1;
        uint8_t I7 : 1;
    };
    struct
    {
        uint8_t vcc5 : 1;
        uint8_t voice_int1 : 1;
        uint8_t voice_int0 : 1;
        uint8_t key4 : 1;
        uint8_t key3 : 1;
        uint8_t key2 : 1;
        uint8_t key1 : 1;
        uint8_t key0 : 1;
    };
} FMC_ValueDef;

typedef struct
{
    char *adc_devname;
    uint8_t adc_channel;
    rt_adc_device_t adc_dev;
    uint32_t adc_value;

    char *led_devname[4];
    rt_base_t led_pin[4];

    char *play_devname[2];
    rt_base_t play_pin[2];

    char *voice_devname[5];
    rt_base_t voice_pin[5];

    char *uart_devname[5];
    rt_device_t uart_dev[5];

    char *can_devname[2];
    rt_device_t can_dev[2];

    char *wav_path;

    uint32_t NSBank[2];
    volatile uint32_t *NSBankAddress[2];
    FMC_ValueDef NSBankValue[2];

    int isThreadRun;
} VoiceCtrlUserData;

extern VoiceCtrlUserData voice_ctrl_userdata;

int tpx86_voice_ctrl_led_init(void);
int tpx86_voice_ctrl_play_init(void);
int tpx86_voice_ctrl_voice_init(void);
int tpx86_voice_ctrl_adc_init(void);
int tpx86_voice_ctrl_fmc_init(void);
int tpx86_music_ctrl_music_init(void);

#endif /* APPLICATIONS_TPX86_VOICE_CTRL_TPX86_VOICE_CTRL_H_ */
