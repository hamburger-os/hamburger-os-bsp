/*
 * Driver for ADAU1761/ADAU1461/ADAU1761/ADAU1961 codec
 *
 */
#include "board.h"
#include "adau17x1.h"

#define DBG_TAG "adau1761"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static struct rt_i2c_bus_device *i2c_bus = NULL;

static void adau1761_delay_us(uint32_t us)
{
    rt_hw_us_delay(us);
}

static void adau1761_delay_ms(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

/*
 * @brief 读寄存器内容.
 * @param reg - 寄存器地址.
 * @param len_u16 - 写入字节数
 * @param *data - 写入数据缓冲区指针.
 */
static void adau1761_write_reg(uint16_t reg, const uint8_t *data, uint16_t len_u16)
{
    reg = __SWP16(reg);
    uint8_t cmd[2 + len_u16];
    rt_memcpy(&cmd[0], &reg, 2);
    rt_memcpy(&cmd[2], data, len_u16);

    rt_i2c_master_send(i2c_bus, ADAU1761_I2C_ADD, RT_I2C_WR, cmd, 2 + len_u16);
}

/*
 * @brief 读ADAU寄存器.
 * @param reg - 寄存器地址.
 * @param len_u16 - 读数据字节数.
 * @param *data - 读数据缓冲区.
 */
static void adau1761_read_reg(uint16_t reg, uint8_t *data, uint16_t len_u16)
{
    reg = __SWP16(reg);
    rt_i2c_master_send(i2c_bus, ADAU1761_I2C_ADD, RT_I2C_WR, (uint8_t *)&reg, 2);
    rt_i2c_master_recv(i2c_bus, ADAU1761_I2C_ADD, RT_I2C_RD, data, len_u16);
}

static void adau1761_show()
{
    uint8_t data[0xFA + 1];
    uint8_t pll_len = sizeof(R0_0x4000_Clock_control) + sizeof(R1_0x4002_PLL_control) + 1;

    adau1761_read_reg(0x4000, data, pll_len);
    adau1761_read_reg(0x4008, data + pll_len, sizeof(data) - pll_len);
    LOG_HEX("reg hex  ", 16, data, sizeof(data));
}

static void adau1761_program(uint16_t addr, uint8_t *data, uint16_t len_u16)
{
    char addr_str[16];
    uint8_t cmp[len_u16];

    adau1761_write_reg(addr, data, len_u16);
//    rt_snprintf(addr_str, sizeof(addr_str), "0x%x ->", addr);
//    LOG_HEX(addr_str, 16, data, len_u16);

    uint8_t i = 0, max = 4;
    for (i = 0; i < max; i++)
    {
        adau1761_delay_us(10);
        adau1761_read_reg(addr, cmp, len_u16);
        if (rt_memcmp(data, cmp, len_u16) != 0)
        {
            //尝试再次写入寄存器
            adau1761_write_reg(addr, data, len_u16);
        }
        else
        {
            break;
        }
    }
    if (i > 0)
    {
        rt_snprintf(addr_str, sizeof(addr_str), "0x%x <-", addr);
        LOG_HEX(addr_str, 16, cmp, len_u16);
        LOG_E("program 0x%x error %d %d", addr, len_u16, i);
    }
}

/*
 * @brief 设置输出音量.
 * @param volume - 音量值
 * 经过音频设备驱动转换,音量值为0~100
 */
void adau1761_set_volume(uint8_t volume)
{
    uint8_t vol = 0x3f * volume / 100;
    uint8_t mute = (vol > 0)?(1):(0);
    LOG_D("volume: %d %d -> %d", mute, volume, vol);

    R31_0x4025_Line_output_left_vol r31 = {
        .LOMODE = 0,
        .LOUTM = mute,
        .LOUTVOL = vol,
    };
    adau1761_program(ADDR_R31_0x4025_Line_output_left_vol, (uint8_t *)&r31, sizeof(R31_0x4025_Line_output_left_vol));

    R32_0x4026_Line_output_right_vol r32 = {
        .ROMODE = 0,
        .ROUTM = mute,
        .ROUTVOL = vol,
    };
    adau1761_program(ADDR_R32_0x4026_Line_output_right_vol, (uint8_t *)&r32, sizeof(R32_0x4026_Line_output_right_vol));
}

/*
 * @brief 设置端口采样率
 * @param rate - 采样率
 * 48000 8000 12000 16000 24000 32000 96000
 */
void adau1761_set_sampling_rate(uint32_t rate)
{
    R17_0x4017_Converter_0 r17 = {
        .CONVSR = 0b001,
        .ADOSR = 0,
        .DAOSR = 0,
        .DAPAIR = 0,
    };
    R57_0x40EB_DSP_sampling_rate_setting r57 = {
        .DSPSR = 0b0110,
    };
    R64_0x40F8_Serial_port_sampling_rate r64 = {
        .SPSR = 0b001,
    };
    if (rate == 8000)
    {
        r17.CONVSR = 0b001;
        r57.DSPSR = 0b0110;
        r64.SPSR = 0b001;
    }
    else if (rate == 12000)
    {
        r17.CONVSR = 0b010;
        r57.DSPSR = 0b0101;
        r64.SPSR = 0b010;
    }
    else if (rate == 16000)
    {
        r17.CONVSR = 0b011;
        r57.DSPSR = 0b0100;
        r64.SPSR = 0b011;
    }
    else if (rate == 24000)
    {
        r17.CONVSR = 0b100;
        r57.DSPSR = 0b0011;
        r64.SPSR = 0b100;
    }
    else if (rate == 32000)
    {
        r17.CONVSR = 0b101;
        r57.DSPSR = 0b0010;
        r64.SPSR = 0b101;
    }
    else if (rate == 48000)
    {
        r17.CONVSR = 0b000;
        r57.DSPSR = 0b0001;
        r64.SPSR = 0b000;
    }
    else if (rate == 96000)
    {
        r17.CONVSR = 0b110;
        r57.DSPSR = 0b0000;
        r64.SPSR = 0b110;
    }
    adau1761_program(ADDR_R17_0x4017_Converter_0, (uint8_t *)&r17, sizeof(R17_0x4017_Converter_0));
    adau1761_program(ADDR_R57_0x40EB_DSP_sampling_rate_setting, (uint8_t *)&r57, sizeof(R57_0x40EB_DSP_sampling_rate_setting));
    adau1761_program(ADDR_R64_0x40F8_Serial_port_sampling_rate, (uint8_t *)&r64, sizeof(R64_0x40F8_Serial_port_sampling_rate));
}

rt_err_t adau1761_player_start()
{
#ifdef ADAU1761_USING_AMP_PIN
    rt_base_t amp_pin = rt_pin_get(ADAU1761_AMP_PIN);
    rt_pin_write(amp_pin, PIN_HIGH);
#endif

    return 0;
}

rt_err_t adau1761_player_stop()
{
#ifdef ADAU1761_USING_AMP_PIN
    rt_base_t amp_pin = rt_pin_get(ADAU1761_AMP_PIN);
    rt_pin_write(amp_pin, PIN_LOW);
#endif

    return 0;
}

/*
 * @brief adau1761初始化.
 */
rt_err_t adau1761_init(struct rt_i2c_bus_device *dev)
{
    i2c_bus = dev;

#ifdef ADAU1761_USING_PWR_PIN
    rt_base_t pwr_pin = rt_pin_get(ADAU1761_PWR_PIN);
    rt_pin_mode(pwr_pin, PIN_MODE_OUTPUT);
    rt_pin_write(pwr_pin, PIN_HIGH);

    adau1761_delay_ms(10);
#endif

#ifdef ADAU1761_USING_AMP_PIN
    rt_base_t amp_pin = rt_pin_get(ADAU1761_AMP_PIN);
    rt_pin_mode(amp_pin, PIN_MODE_OUTPUT);
    rt_pin_write(amp_pin, PIN_LOW);
#endif

    R0_0x4000_Clock_control r0 = {
        .COREN = 1,
#ifdef ADAU1761_CLKSRC_MCLK256
        .INFREQ = 0b00,
#endif
#ifdef ADAU1761_CLKSRC_MCLK512
        .INFREQ = 0b01,
#endif
#ifdef ADAU1761_CLKSRC_MCLK768
        .INFREQ = 0b10,
#endif
#ifdef ADAU1761_CLKSRC_MCLK1024
        .INFREQ = 0b11,
#endif
#ifdef ADAU1761_CLKSRC_PLL
        .INFREQ = 0b11,
#endif

#ifdef ADAU1761_CLKSRC_MCLK
        .CLKSRC = 0,
#endif
#ifdef ADAU1761_CLKSRC_PLL
        .CLKSRC = 1,
#endif
    };
    adau1761_program(ADDR_R0_0x4000_Clock_control, (uint8_t *)&r0, sizeof(R0_0x4000_Clock_control));

    R1_0x4002_PLL_control r1 = {
#ifdef ADAU1761_MCLK_8
        .M = __SWP16(125),
        .N = __SWP16(18),
        .X = 0b00,//1
        .R = 0b0110,//6
        .Type = 1,
#endif
#ifdef ADAU1761_MCLK_12
        .M = __SWP16(125),
        .N = __SWP16(12),
        .X = 0b00,//1
        .R = 0b0100,//4
        .Type = 1,
#endif
#ifdef ADAU1761_MCLK_12288
        .M = __SWP16(0),
        .N = __SWP16(0),
        .X = 0b00,//1
        .R = 0b0100,//4
        .Type = 0,
#endif
#ifdef ADAU1761_MCLK_24576
        .M = __SWP16(0),
        .N = __SWP16(0),
        .X = 0b00,//1
        .R = 0b0010,//2
        .Type = 0,
#endif
#ifdef ADAU1761_CLKSRC_MCLK
        .M = __SWP16(253),
        .N = __SWP16(12),
        .X = 0b00,//1
        .R = 0b0010,//2
        .Type = 0,
#endif

#ifdef ADAU1761_CLKSRC_MCLK
        .PLLEN = 0,
        .Lock = 0,
#endif
#ifdef ADAU1761_CLKSRC_PLL
        .PLLEN = 1,
        .Lock = 1,
#endif
    };
    adau1761_program(ADDR_R1_0x4002_PLL_control, (uint8_t *)&r1, sizeof(R1_0x4002_PLL_control));

    adau1761_delay_ms(5);

    R2_0x4008_Dig_mic_jack_detect r2 = {
        .JDPOL = 0,
        .JDFUNC = 0,
        .JDDB = 0,
    };
    adau1761_program(ADDR_R2_0x4008_Dig_mic_jack_detect, (uint8_t *)&r2, sizeof(R2_0x4008_Dig_mic_jack_detect));

    R3_0x4009_Rec_power_mgmt r3 = {
        .RBIAS = 0,
        .ADCBIAS = 0,
        .MXBIAS = 0,
    };
    adau1761_program(ADDR_R3_0x4009_Rec_power_mgmt, (uint8_t *)&r3, sizeof(R3_0x4009_Rec_power_mgmt));

    R4_0x400A_Rec_Mixer_Left_0 r4 = {
        .MX1EN = 1,
        .LINNG = 0b000,
        .LINPG = 0b000,
    };
    adau1761_program(ADDR_R4_0x400A_Rec_Mixer_Left_0, (uint8_t *)&r4, sizeof(R4_0x400A_Rec_Mixer_Left_0));

    R5_0x400B_Rec_Mixer_Left_1 r5 = {
        .MX1AUXG = 0b000,
        .LDBOOST = 0b01,
    };
    adau1761_program(ADDR_R5_0x400B_Rec_Mixer_Left_1, (uint8_t *)&r5, sizeof(R5_0x400B_Rec_Mixer_Left_1));

    R6_0x400C_Rec_Mixer_Right_0 r6 = {
        .MX2EN = 1,
        .RINNG = 0b000,
        .RINPG = 0b000,
    };
    adau1761_program(ADDR_R6_0x400C_Rec_Mixer_Right_0, (uint8_t *)&r6, sizeof(R6_0x400C_Rec_Mixer_Right_0));

    R7_0x400D_Rec_Mixer_Right_1 r7 = {
        .MX2AUXG = 0b000,
        .RDBOOST = 0b01,
    };
    adau1761_program(ADDR_R7_0x400D_Rec_Mixer_Right_1, (uint8_t *)&r7, sizeof(R7_0x400D_Rec_Mixer_Right_1));

    R8_0x400E_Left_diff_input_vol r8 = {
        .LDEN = 1,
        .LDMUTE = 1,
        .LDVOL = 0b110100,
    };
    adau1761_program(ADDR_R8_0x400E_Left_diff_input_vol, (uint8_t *)&r8, sizeof(R8_0x400E_Left_diff_input_vol));

    R9_0x400F_Right_diff_input_vol r9 = {
        .RDEN = 1,
        .RDMUTE = 1,
        .RDVOL = 0b110100,
    };
    adau1761_program(ADDR_R9_0x400F_Right_diff_input_vol, (uint8_t *)&r9, sizeof(R9_0x400F_Right_diff_input_vol));

    R10_0x4010_Record_mic_bias r10 = {
        .MBIEN = 0,
        .MBI = 0,
        .MPERF = 0,
    };
    adau1761_program(ADDR_R10_0x4010_Record_mic_bias, (uint8_t *)&r10, sizeof(R10_0x4010_Record_mic_bias));

    R11_0x4011_ALC_0 r11 = {
        .ALCSEL = 0b000,//ALC开关：000 off ;011 Stereo
        .ALCMAX = 0b010,
        .PGASLEW = 0b00,
    };
    adau1761_program(ADDR_R11_0x4011_ALC_0, (uint8_t *)&r11, sizeof(R11_0x4011_ALC_0));

    R12_0x4012_ALC_1 r12 = {
        .ALCTARG = 0b1011,
        .ALCHOLD = 0b0100,
    };
    adau1761_program(ADDR_R12_0x4012_ALC_1, (uint8_t *)&r12, sizeof(R12_0x4012_ALC_1));

    R13_0x4013_ALC_2 r13 = {
        .ALCDEC = 0b0110,
        .ALCATCK = 0b0010,
    };
    adau1761_program(ADDR_R13_0x4013_ALC_2, (uint8_t *)&r13, sizeof(R13_0x4013_ALC_2));

    R14_0x4014_ALC_3 r14 = {
        .NGTHR = 0b10000,
        .NGEN = 1,//噪声门开关
        .NGTYP = 0b11,
    };
    adau1761_program(ADDR_R14_0x4014_ALC_3, (uint8_t *)&r14, sizeof(R14_0x4014_ALC_3));

    R15_0x4015_Serial_Port_0 r15 = {
        .MS = 0,
        .CHPF = 0b00,
        .LRPOL = 0,
        .BPOL = 0,
        .LRMOD = 0,
        .SPSRS = 0,
    };
    adau1761_program(ADDR_R15_0x4015_Serial_Port_0, (uint8_t *)&r15, sizeof(R15_0x4015_Serial_Port_0));

    R16_0x4016_Serial_Port_1 r16 = {
        .LRDEL = 0b00,
        .MSBP = 0,
        .DATDM = 0,
        .ADTDM = 0,
        .BPF = 0b000,
    };
    adau1761_program(ADDR_R16_0x4016_Serial_Port_1, (uint8_t *)&r16, sizeof(R16_0x4016_Serial_Port_1));

    R17_0x4017_Converter_0 r17 = {
        .CONVSR = 0b001,
        .ADOSR = 0,
        .DAOSR = 0,
        .DAPAIR = 0,
    };
    adau1761_program(ADDR_R17_0x4017_Converter_0, (uint8_t *)&r17, sizeof(R17_0x4017_Converter_0));

    R18_0x4018_Converter_1 r18 = {
        .ADPAIR = 0,
    };
    adau1761_program(ADDR_R18_0x4018_Converter_1, (uint8_t *)&r18, sizeof(R18_0x4018_Converter_1));

    R19_0x4019_ADC_control r19 = {
        .ADCEN = 0b11,
        .INSEL = 0,
        .DMSW = 0,
        .DMPOL = 1,
        .HPF = 1,
        .ADCPOL = 0,
    };
    adau1761_program(ADDR_R19_0x4019_ADC_control, (uint8_t *)&r19, sizeof(R19_0x4019_ADC_control));

    R20_0x401A_Left_digital_vol r20 = {
        .LADVOL = 0b00000000,
    };
    adau1761_program(ADDR_R20_0x401A_Left_digital_vol, (uint8_t *)&r20, sizeof(R20_0x401A_Left_digital_vol));

    R21_0x401B_Right_digital_vol r21 = {
        .RADVOL = 0b00000000,
    };
    adau1761_program(ADDR_R21_0x401B_Right_digital_vol, (uint8_t *)&r21, sizeof(R21_0x401B_Right_digital_vol));

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

    R26_0x4020_Play_L_R_mixer_left r26 = {
        .MX5EN = 1,
        .MX5G3 = 0b10,
        .MX5G4 = 0b10,
    };
    adau1761_program(ADDR_R26_0x4020_Play_L_R_mixer_left, (uint8_t *)&r26, sizeof(R26_0x4020_Play_L_R_mixer_left));

    R27_0x4021_Play_L_R_mixer_right r27 = {
        .MX6EN = 1,
        .MX6G3 = 0b10,
        .MX6G4 = 0b10,
    };
    adau1761_program(ADDR_R27_0x4021_Play_L_R_mixer_right, (uint8_t *)&r27, sizeof(R27_0x4021_Play_L_R_mixer_right));

    R28_0x4022_Play_L_R_mixer_mono r28 = {
        .MX7EN = 0,
        .MX7 = 0b01,
    };
    adau1761_program(ADDR_R28_0x4022_Play_L_R_mixer_mono, (uint8_t *)&r28, sizeof(R28_0x4022_Play_L_R_mixer_mono));

    R29_0x4023_Play_HP_left_vol r29 = {
        .HPEN = 1,
        .LHPM = 1,
        .LHPVOL = 0b111111,
    };
    adau1761_program(ADDR_R29_0x4023_Play_HP_left_vol, (uint8_t *)&r29, sizeof(R29_0x4023_Play_HP_left_vol));

    R30_0x4024_Play_HP_right_vol r30 = {
        .HPMODE = 1,
        .RHPM = 1,
        .RHPVOL = 0b111111,
    };
    adau1761_program(ADDR_R30_0x4024_Play_HP_right_vol, (uint8_t *)&r30, sizeof(R30_0x4024_Play_HP_right_vol));

    R31_0x4025_Line_output_left_vol r31 = {
        .LOMODE = 0,
        .LOUTM = 1,
        .LOUTVOL = 0b111111,
    };
    adau1761_program(ADDR_R31_0x4025_Line_output_left_vol, (uint8_t *)&r31, sizeof(R31_0x4025_Line_output_left_vol));

    R32_0x4026_Line_output_right_vol r32 = {
        .ROMODE = 0,
        .ROUTM = 1,
        .ROUTVOL = 0b111111,
    };
    adau1761_program(ADDR_R32_0x4026_Line_output_right_vol, (uint8_t *)&r32, sizeof(R32_0x4026_Line_output_right_vol));

    R33_0x4027_Play_mono_output r33 = {
        .MOMODE = 0,
        .MONOM = 0,
        .MONOVOL = 0b111111,
    };
    adau1761_program(ADDR_R33_0x4027_Play_mono_output, (uint8_t *)&r33, sizeof(R33_0x4027_Play_mono_output));

    R34_0x4028_Pop_click_suppress r34 = {
        .ASLEW = 0b00,
        .POPLESS = 0,
        .POPMODE = 0,
    };
    adau1761_program(ADDR_R34_0x4028_Pop_click_suppress, (uint8_t *)&r34, sizeof(R34_0x4028_Pop_click_suppress));

    R35_0x4029_Play_power_mgmt r35 = {
        .PLEN = 1,
        .PREN = 1,
        .PBIAS = 0,
        .DACBIAS = 0,
        .HPBIAS = 0,
    };
    adau1761_program(ADDR_R35_0x4029_Play_power_mgmt, (uint8_t *)&r35, sizeof(R35_0x4029_Play_power_mgmt));

    R36_0x402A_DAC_Control_0 r36 = {
        .DACEN = 0b11,
        .DEMPH = 1,
        .DACPOL = 0,
        .DACMONO = 0b00,
    };
    adau1761_program(ADDR_R36_0x402A_DAC_Control_0, (uint8_t *)&r36, sizeof(R36_0x402A_DAC_Control_0));

    R37_0x402B_DAC_Control_1 r37 = {
        .LDAVOL = 0b00000000,
    };
    adau1761_program(ADDR_R37_0x402B_DAC_Control_1, (uint8_t *)&r37, sizeof(R37_0x402B_DAC_Control_1));

    R38_0x402C_DAC_Control_2 r38 = {
        .RDAVOL = 0b00000000,
    };
    adau1761_program(ADDR_R38_0x402C_DAC_Control_2, (uint8_t *)&r38, sizeof(R38_0x402C_DAC_Control_2));

    R39_0x402D_Serial_port_pad r39 = {
        .BCLKP = 0b10,
        .LRCLKP = 0b10,
        .DACSDP = 0b10,
        .ADCSDP = 0b10,
    };
    adau1761_program(ADDR_R39_0x402D_Serial_port_pad, (uint8_t *)&r39, sizeof(R39_0x402D_Serial_port_pad));

    R40_0x402F_Control_Port_Pad_0 r40 = {
        .SDAP = 0b10,
        .SCLP = 0b10,
        .CLCHP = 0b10,
        .CDATP = 0b10,
    };
    adau1761_program(ADDR_R40_0x402F_Control_Port_Pad_0, (uint8_t *)&r40, sizeof(R40_0x402F_Control_Port_Pad_0));

    R41_0x4030_Control_Port_Pad_1 r41 = {
        .SDASTR = 0,
    };
    adau1761_program(ADDR_R41_0x4030_Control_Port_Pad_1, (uint8_t *)&r41, sizeof(R41_0x4030_Control_Port_Pad_1));

    R42_0x4031_Jack_detect_pin r42 = {
        .JDP = 0b10,
        .JDSTR = 0,
    };
    adau1761_program(ADDR_R42_0x4031_Jack_detect_pin, (uint8_t *)&r42, sizeof(R42_0x4031_Jack_detect_pin));

    R67_0x4036_Dejitter_control r67 = {
        .DEJIT = 0b00000011,
    };
    adau1761_program(ADDR_R67_0x4036_Dejitter_control, (uint8_t *)&r67, sizeof(R67_0x4036_Dejitter_control));

    R47_0x40C4_CRC_enable r47 = {
        .CRCEN = 0,
    };
    adau1761_program(ADDR_R47_0x40C4_CRC_enable, (uint8_t *)&r47, sizeof(R47_0x40C4_CRC_enable));

    R48_0x40C6_GPIO0_pin_control r48 = {
        .GPIO0 = 0b0000,
    };
    adau1761_program(ADDR_R48_0x40C6_GPIO0_pin_control, (uint8_t *)&r48, sizeof(R48_0x40C6_GPIO0_pin_control));

    R49_0x40C7_GPIO1_pin_control r49 = {
        .GPIO1 = 0b0000,
    };
    adau1761_program(ADDR_R49_0x40C7_GPIO1_pin_control, (uint8_t *)&r49, sizeof(R49_0x40C7_GPIO1_pin_control));

    R50_0x40C8_GPIO2_pin_control r50 = {
        .GPIO2 = 0b0000,
    };
    adau1761_program(ADDR_R50_0x40C8_GPIO2_pin_control, (uint8_t *)&r50, sizeof(R50_0x40C8_GPIO2_pin_control));

    R51_0x40C9_GPIO3_pin_control r51 = {
        .GPIO3 = 0b0000,
    };
    adau1761_program(ADDR_R51_0x40C9_GPIO3_pin_control, (uint8_t *)&r51, sizeof(R51_0x40C9_GPIO3_pin_control));

    R52_0x40D0_Watchdog_enable r52 = {
        .DOGEN = 1,
    };
    adau1761_program(ADDR_R52_0x40D0_Watchdog_enable, (uint8_t *)&r52, sizeof(R52_0x40D0_Watchdog_enable));

    R57_0x40EB_DSP_sampling_rate_setting r57 = {
        .DSPSR = 0b0110,
    };
    adau1761_program(ADDR_R57_0x40EB_DSP_sampling_rate_setting, (uint8_t *)&r57, sizeof(R57_0x40EB_DSP_sampling_rate_setting));

    R58_0x40F2_Serial_input_route_control r58 = {
        .SINRT = 0b0001,
    };
    adau1761_program(ADDR_R58_0x40F2_Serial_input_route_control, (uint8_t *)&r58, sizeof(R58_0x40F2_Serial_input_route_control));

    R59_0x40F3_Serial_output_route_control r59 = {
        .SOUTRT = 0b0001,
    };
    adau1761_program(ADDR_R59_0x40F3_Serial_output_route_control, (uint8_t *)&r59, sizeof(R59_0x40F3_Serial_output_route_control));

    R60_0x40F4_Serial_data_GPIO_pin_configuration r60 = {
        .SDIGP0 = 0,
        .SDOGP1 = 0,
        .BGP2 = 0,
        .LRGP3 = 0,
    };
    adau1761_program(ADDR_R60_0x40F4_Serial_data_GPIO_pin_configuration, (uint8_t *)&r60, sizeof(R60_0x40F4_Serial_data_GPIO_pin_configuration));

    R61_0x40F5_DSP_enable r61 = {
        .DSPEN = 0,
    };
    adau1761_program(ADDR_R61_0x40F5_DSP_enable, (uint8_t *)&r61, sizeof(R61_0x40F5_DSP_enable));

    R62_0x40F6_DSP_run r62 = {
        .DSPRUN = 0,
    };
    adau1761_program(ADDR_R62_0x40F6_DSP_run, (uint8_t *)&r62, sizeof(R62_0x40F6_DSP_run));

    R63_0x40F7_DSP_slew_modes r63 = {
        .LHPSLW = 0,
        .RHPSLW = 0,
        .LOSLW = 0,
        .ROSLW = 0,
        .MOSLW = 0,
    };
    adau1761_program(ADDR_R63_0x40F7_DSP_slew_modes, (uint8_t *)&r63, sizeof(R63_0x40F7_DSP_slew_modes));

    R64_0x40F8_Serial_port_sampling_rate r64 = {
        .SPSR = 0b001,
    };
    adau1761_program(ADDR_R64_0x40F8_Serial_port_sampling_rate, (uint8_t *)&r64, sizeof(R64_0x40F8_Serial_port_sampling_rate));

    R65_0x40F9_Clock_Enable_0 r65 = {
        .SPPD = 1,
        .SINPD = 1,
        .INTPD = 1,
        .SOUTPD = 1,
        .DECPD = 1,
        .ALCPD = 1,
        .SLEWPD = 1,
    };
    adau1761_program(ADDR_R65_0x40F9_Clock_Enable_0, (uint8_t *)&r65, sizeof(R65_0x40F9_Clock_Enable_0));

    R66_0x40FA_Clock_Enable_1 r66 = {
        .CLK0 = 1,
        .CLK1 = 1,
    };
    adau1761_program(ADDR_R66_0x40FA_Clock_Enable_1, (uint8_t *)&r66, sizeof(R66_0x40FA_Clock_Enable_1));

    // adau1761_show();

    return 0;
}
