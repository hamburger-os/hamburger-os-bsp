/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-05     lvhan       the first version
 */

#include "board.h"

#include "tpx86_voice_ctrl.h"

#define DBG_TAG "tpx86"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

VoiceCtrlUserData voice_ctrl_userdata = {
    .adc_devname = "adc1",
    .adc_channel = 8,

    .led_devname = {"PC13", "PI8", "PC15", "PI10"},

    .play_devname = {"PH2", "PH3"},

    .voice_devname = {"PB8", "PB9", "PI6", "PI7", "PI5"},

    .uart_devname = {"uart2", "uart3", "uart4", "uart7", "uart6"},

    .can_devname = {"can1", "can2"},

    .NSBank = {FMC_NORSRAM_BANK1, FMC_NORSRAM_BANK4},
    .NSBankAddress = {(volatile uint32_t *)NOR_MEMORY_ADRESS1, (volatile uint32_t *)NOR_MEMORY_ADRESS4},

    .wav_path = "/mnt/emmc/5s_8000_2ch.wav",

    .isThreadRun = 1,
};

static int tpx86_voice_ctrl_init(void)
{
    tpx86_voice_ctrl_led_init();
    tpx86_voice_ctrl_play_init();
    tpx86_voice_ctrl_voice_init();
    tpx86_voice_ctrl_adc_init();
    tpx86_voice_ctrl_fmc_init();
    tpx86_music_ctrl_music_init();

    return RT_EOK;
}

#ifndef APP_TPX86_VOICE_CTRL //如果使用dl动态模块方式
int main(int argc, char *argv[])
{
    tpx86_voice_ctrl_init();

    while(voice_ctrl_userdata.isThreadRun)
    {
        rt_thread_delay(1000);
    }
    return 0;
}
#else
INIT_APP_EXPORT(tpx86_voice_ctrl_init);
#endif
