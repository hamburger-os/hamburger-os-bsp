
/*******************************************************
 *
 * @FileName: ly_05c.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 音频接口测试程序.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>
#include <pthread.h>
#include <delay.h>
#include <adau17x1.h>


/*******************************************************
 *
 * @brief  设置ADAU1761为直通模式
 *
 * @param  none
 * @retval none
 *
 *******************************************************/

void set_adau1761_directout(void)
{
    R22_0x401C_Play_Mixer_Left_0 r22 = {
        .MX3EN = 1,
        .MX3AUXG = 0b0000,
        .MX3LM = 0,
        .MX3RM = 0,
    };
    adau1761_program(ADDR_R22_0x401C_Play_Mixer_Left_0, (uint8_t *)&r22, sizeof(R22_0x401C_Play_Mixer_Left_0));

    R23_0x401D_Play_Mixer_Left_1 r23 = {
        .MX3G1 = 0b0110,
        .MX3G2 = 0b0000,
    };
    adau1761_program(ADDR_R23_0x401D_Play_Mixer_Left_1, (uint8_t *)&r23, sizeof(R23_0x401D_Play_Mixer_Left_1));

    R24_0x401E_Play_Mixer_Right_0 r24 = {
        .MX4EN = 1,
        .MX4AUXG = 0b0000,
        .MX4LM = 0,
        .MX4RM = 0,
    };
    adau1761_program(ADDR_R24_0x401E_Play_Mixer_Right_0, (uint8_t *)&r24, sizeof(R24_0x401E_Play_Mixer_Right_0));

    R25_0x401F_Play_Mixer_Right_1 r25 = {
        .MX4G1 = 0b0000,
        .MX4G2 = 0b0110,
    };
    adau1761_program(ADDR_R25_0x401F_Play_Mixer_Right_1, (uint8_t *)&r25, sizeof(R25_0x401F_Play_Mixer_Right_1));

    rt_base_t amp_pin = rt_pin_get(ADAU1761_AMP_PIN);
    rt_pin_write(amp_pin, PIN_HIGH);
}
/*******************************************************
 *
 * @brief  设置ADAU1761为正常模式
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
void set_adau1761_normal(void)
{
    R22_0x401C_Play_Mixer_Left_0 r22 = {
        .MX3EN = 1,
        .MX3AUXG = 0b0000,
        .MX3LM = 1,
        .MX3RM = 0,
    };
    adau1761_program(ADDR_R22_0x401C_Play_Mixer_Left_0, (uint8_t *)&r22, sizeof(R22_0x401C_Play_Mixer_Left_0));

    R23_0x401D_Play_Mixer_Left_1 r23 = {
        .MX3G1 = 0b0000,
        .MX3G2 = 0b0000,
    };
    adau1761_program(ADDR_R23_0x401D_Play_Mixer_Left_1, (uint8_t *)&r23, sizeof(R23_0x401D_Play_Mixer_Left_1));

    R24_0x401E_Play_Mixer_Right_0 r24 = {
        .MX4EN = 1,
        .MX4AUXG = 0b0000,
        .MX4LM = 0,
        .MX4RM = 1,
    };
    adau1761_program(ADDR_R24_0x401E_Play_Mixer_Right_0, (uint8_t *)&r24, sizeof(R24_0x401E_Play_Mixer_Right_0));

    R25_0x401F_Play_Mixer_Right_1 r25 = {
        .MX4G1 = 0b0000,
        .MX4G2 = 0b0000,
    };
    adau1761_program(ADDR_R25_0x401F_Play_Mixer_Right_1, (uint8_t *)&r25, sizeof(R25_0x401F_Play_Mixer_Right_1));
    
    rt_base_t amp_pin = rt_pin_get(ADAU1761_AMP_PIN);
    rt_pin_write(amp_pin, PIN_LOW);
}

/*******************************************************
 *
 * @brief  输出用法
 *
 * @param  file: 文件名
 * @retval none
 *
 *******************************************************/

static void print_usage(void)
{
    rt_kprintf("Usage: \n");
    rt_kprintf("    VoiceTest 1000   start voice test \n");
}
/*******************************************************
 *
 * @brief  语音测试测试程序
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
int ly_05c1_voice_test(int argc, char *argv[])
{
    if (argc != 2)
    {
        print_usage();
    }
    else
    {
        if (strcmp(argv[1], "1000") == 0) /* 开始测试 */
        {
            /* 开始 */
            set_adau1761_directout();

            /* 结束 */
            while (getchar() != 'c')
            {
                rt_thread_mdelay(10);
            }
            set_adau1761_normal();
        }
        else
        {
            print_usage();
        }
    }

    return RT_EOK;
}

MSH_CMD_EXPORT_ALIAS(ly_05c1_voice_test, VoiceTest, ly - 05c voice test.);
