#ifndef __ADAU17X1_H__
#define __ADAU17X1_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "rtdef.h"

#define ADDR_DSP_Parameter_RAM_START                        0x0000
#define ADDR_DSP_Parameter_RAM_END                          0x03FF
#define ADDR_DSP_Program_RAM_START                          0x0800
#define ADDR_DSP_Program_RAM_END                            0x0BFF

#define ADDR_R0_0x4000_Clock_control                        0x4000

#define ADDR_R1_0x4002_PLL_control                          0x4002

#define ADDR_R2_0x4008_Dig_mic_jack_detect                  0x4008
#define ADDR_R3_0x4009_Rec_power_mgmt                       0x4009
#define ADDR_R4_0x400A_Rec_Mixer_Left_0                     0x400A
#define ADDR_R5_0x400B_Rec_Mixer_Left_1                     0x400B
#define ADDR_R6_0x400C_Rec_Mixer_Right_0                    0x400C
#define ADDR_R7_0x400D_Rec_Mixer_Right_1                    0x400D
#define ADDR_R8_0x400E_Left_diff_input_vol                  0x400E
#define ADDR_R9_0x400F_Right_diff_input_vol                 0x400F
#define ADDR_R10_0x4010_Record_mic_bias                     0x4010
#define ADDR_R11_0x4011_ALC_0                               0x4011
#define ADDR_R12_0x4012_ALC_1                               0x4012
#define ADDR_R13_0x4013_ALC_2                               0x4013
#define ADDR_R14_0x4014_ALC_3                               0x4014
#define ADDR_R15_0x4015_Serial_Port_0                       0x4015
#define ADDR_R16_0x4016_Serial_Port_1                       0x4016
#define ADDR_R17_0x4017_Converter_0                         0x4017
#define ADDR_R18_0x4018_Converter_1                         0x4018
#define ADDR_R19_0x4019_ADC_control                         0x4019
#define ADDR_R20_0x401A_Left_digital_vol                    0x401A
#define ADDR_R21_0x401B_Right_digital_vol                   0x401B
#define ADDR_R22_0x401C_Play_Mixer_Left_0                   0x401C
#define ADDR_R23_0x401D_Play_Mixer_Left_1                   0x401D
#define ADDR_R24_0x401E_Play_Mixer_Right_0                  0x401E
#define ADDR_R25_0x401F_Play_Mixer_Right_1                  0x401F
#define ADDR_R26_0x4020_Play_L_R_mixer_left                 0x4020
#define ADDR_R27_0x4021_Play_L_R_mixer_right                0x4021
#define ADDR_R28_0x4022_Play_L_R_mixer_mono                 0x4022
#define ADDR_R29_0x4023_Play_HP_left_vol                    0x4023
#define ADDR_R30_0x4024_Play_HP_right_vol                   0x4024
#define ADDR_R31_0x4025_Line_output_left_vol                0x4025
#define ADDR_R32_0x4026_Line_output_right_vol               0x4026
#define ADDR_R33_0x4027_Play_mono_output                    0x4027
#define ADDR_R34_0x4028_Pop_click_suppress                  0x4028
#define ADDR_R35_0x4029_Play_power_mgmt                     0x4029
#define ADDR_R36_0x402A_DAC_Control_0                       0x402A
#define ADDR_R37_0x402B_DAC_Control_1                       0x402B
#define ADDR_R38_0x402C_DAC_Control_2                       0x402C
#define ADDR_R39_0x402D_Serial_port_pad                     0x402D
#define ADDR_R40_0x402F_Control_Port_Pad_0                  0x402F
#define ADDR_R41_0x4030_Control_Port_Pad_1                  0x4030
#define ADDR_R42_0x4031_Jack_detect_pin                     0x4031

#define ADDR_R67_0x4036_Dejitter_control                    0x4036

#define ADDR_R43_0x40C0_Cyclic_redundancy_check             0x40C0

#define ADDR_R47_0x40C4_CRC_enable                          0x40C4

#define ADDR_R48_0x40C6_GPIO0_pin_control                   0x40C6
#define ADDR_R49_0x40C7_GPIO1_pin_control                   0x40C7
#define ADDR_R50_0x40C8_GPIO2_pin_control                   0x40C8
#define ADDR_R51_0x40C9_GPIO3_pin_control                   0x40C9

#define ADDR_R52_0x40D0_Watchdog_enable                     0x40D0
#define ADDR_R53_0x40D1_Watchdog_value                      0x40D1

#define ADDR_R56_0x40D4_Watchdog_error                      0x40D4

#define ADDR_R57_0x40EB_DSP_sampling_rate_setting           0x40EB

#define ADDR_R58_0x40F2_Serial_input_route_control          0x40F2
#define ADDR_R59_0x40F3_Serial_output_route_control         0x40F3
#define ADDR_R60_0x40F4_Serial_data_GPIO_pin_configuration  0x40F4
#define ADDR_R61_0x40F5_DSP_enable                          0x40F5
#define ADDR_R62_0x40F6_DSP_run                             0x40F6
#define ADDR_R63_0x40F7_DSP_slew_modes                      0x40F7
#define ADDR_R64_0x40F8_Serial_port_sampling_rate           0x40F8
#define ADDR_R65_0x40F9_Clock_Enable_0                      0x40F9
#define ADDR_R66_0x40FA_Clock_Enable_1                      0x40FA

typedef struct __attribute__ ((packed))
{
    /* Core clock enable. Only the R0 and R1 registers can be accessed when this bit is set to 0 (core clock disabled).
    0 = core clock disabled (default).
    1 = core clock enabled. */
    uint8_t COREN: 1;
    /* Input clock frequency. Sets the core clock rate that generates the core clock. If the PLL is used, this value is
    automatically set to 1024 × fS.
    Setting Input Clock Frequency
    00 256 × fS (default)
    01 512 × fS
    10 768 × fS
    11 1024 × fS */
    uint8_t INFREQ: 2;
    /* Clock source select.
    0 = direct from MCLK pin (default).
    1 = PLL clock. */
    uint8_t CLKSRC: 1;
    /*  */
    uint8_t Reserved: 4;
} R0_0x4000_Clock_control;

/* Table 17. Fractional PLL Parameter Settings for fS = 48 kHz (PLL Output = 49.152 MHz = 1024 × fS)
MCLK Input (MHz) Input Divider (X) Integer (R) Denominator (M) Numerator (N) R2: PLL Control Setting (Hex)
8 1 6 125 18 0x007D 0012 3101
12 1 4 125 12 0x007D 000C 2101
13 1 3 1625 1269 0x0659 04F5 1901
14.4 2 6 75 62 0x004B 003E 3301
19.2 2 5 25 3 0x0019 0003 2B01
19.68 2 4 205 204 0x00CD 00CC 2301
19.8 2 4 825 796 0x0339 031C 2301
24 2 4 125 12 0x007D 000C 2301
26 2 3 1625 1269 0x0659 04F5 1B01
27 2 3 1125 721 0x0465 02D1 1B01 */
typedef struct __attribute__ ((packed))
{
    /* PLL denominator MSB. This value is concatenated with M[7:0] to make up a 16-bit number.
    PLL denominator LSB. This value is concatenated with M[15:8] to make up a 16-bit number.
    M[15:8] (MSB) M[7:0] (LSB) Value of M
    00000000 00000000 0
    … … …
    00000000 11111101 253 (default)
    … … …
    11111111 11111111 65,535 */
    uint16_t M;
    /* PLL numerator MSB. This value is concatenated with N[7:0] to make up a 16-bit number.
    PLL numerator LSB. This value is concatenated with N[15:8] to make up a 16-bit number.
    N[15:8] (MSB) N[7:0] (LSB) Value of N
    00000000 00000000 0
    … … …
    00000000 00001100 12 (default)
    … … …
    11111111 11111111 65,535 */
    uint16_t N;
    /* Type of PLL. When set to integer mode, the values of M and N are ignored.
    0 = integer (default).
    1 = fractional. */
    uint8_t Type: 1;
    /* PLL input clock divider.
    Setting Value of X
    00 1 (default)
    01 2
    10 3
    11 4 */
    uint8_t X: 2;
    /* PLL integer setting.
    Setting Value of R
    0010 2 (default)
    0011 3
    0100 4
    0101 5
    0110 6
    0111 7
    1000 8 */
    uint8_t R: 4;
    /*  */
    uint8_t Reserved1: 1;
    /* PLL enable.
    0 = PLL disabled (default).
    1 = PLL enabled. */
    uint8_t PLLEN: 1;
    /* PLL lock. This read-only bit is flagged when the PLL has finished locking.
    0 = PLL unlocked (default).
    1 = PLL locked. */
    uint8_t Lock: 1;
    /*  */
    uint8_t Reserved2: 6;
} R1_0x4002_PLL_control;

typedef struct __attribute__ ((packed))
{
    /* Jack detect polarity. Detects high or low signal.
    0 = detect high signal (default).
    1 = detect low signal. */
    uint8_t JDPOL: 1;
    /*  */
    uint8_t Reserved: 3;
    /* JACKDET/MICIN pin function. Enables or disables the jack detect function or configures the pin for a digital
    microphone input.
    Setting Pin Function
    00 Jack detect off (default)
    01 Jack detect on
    10 Digital microphone input
    11 Reserved */
    uint8_t JDFUNC: 2;
    /* Jack detect debounce time.
    Setting Debounce Time
    00 5 ms (default)
    01 10 ms
    10 20 ms
    11 40 ms */
    uint8_t JDDB: 2;
} R2_0x4008_Dig_mic_jack_detect;

typedef struct __attribute__ ((packed))
{
    /*  */
    uint8_t Reserved1: 1;
    /* Record path bias control. Sets the bias current for the PGAs and mixers in the record path.
    Setting Record Path Bias Control
    00 Normal operation (default)
    01 Reserved
    10 Enhanced performance
    11 Power saving */
    uint8_t RBIAS: 2;
    /* ADC bias control. Sets the bias current for the ADCs based on the mode of operation selected.
    Setting ADC Bias Control
    00 Normal operation (default)
    01 Extreme power saving
    10 Enhanced performance
    11 Power saving */
    uint8_t ADCBIAS: 2;
    /* Mixer amplifier bias boost. Sets the boost level for the bias current of the record path mixers. In some cases,
    the boost level enhances the THD + N performance.
    Setting Boost Level
    00 Normal operation (default)
    01 Boost Level 1
    10 Boost Level 2
    11 Boost Level 3 */
    uint8_t MXBIAS: 2;
    /*  */
    uint8_t Reserved2: 1;
} R3_0x4009_Rec_power_mgmt;

typedef struct __attribute__ ((packed))
{
    /* Left channel mixer enable in the record path. Referred to as Mixer 1.
    0 = mixer disabled (default).
    1 = mixer enabled. */
    uint8_t MX1EN: 1;
    /* Gain for a left channel single-ended input from the LINN pin, input to Mixer 1.
    Setting Gain
    000 Mute (default)
    001 −12 dB
    010 −9 dB
    011 −6 dB
    100 −3 dB
    101 0 dB
    110 3 dB
    111 6 dB */
    uint8_t LINNG: 3;
    /* Gain for a left channel single-ended input from the LINP pin, input to Mixer 1.
    Setting Gain
    000 Mute (default)
    001 −12 dB
    010 −9 dB
    011 −6 dB
    100 −3 dB
    101 0 dB
    110 3 dB
    111 6 dB */
    uint8_t LINPG: 3;
    /*  */
    uint8_t Reserved: 1;
} R4_0x400A_Rec_Mixer_Left_0;

typedef struct __attribute__ ((packed))
{
    /* Left single-ended auxiliary input gain from the LAUX pin in the record path, input to Mixer 1.
    Setting Auxiliary Input Gain
    000 Mute (default)
    001 −12 dB
    010 −9 dB
    011 −6 dB
    100 −3 dB
    101 0 dB
    110 3 dB
    111 6 dB */
    uint8_t MX1AUXG: 3;
    /* Left channel differential PGA input gain boost, input to Mixer 1. The left differential input uses the LINP (positive
    signal) and LINN (negative signal) pins.
    Setting Gain Boost
    00 Mute (default)
    01 0 dB
    10 20 dB
    11 Reserved */
    uint8_t LDBOOST: 2;
    /*  */
    uint8_t Reserved: 3;
} R5_0x400B_Rec_Mixer_Left_1;

typedef struct __attribute__ ((packed))
{
    /* Right channel mixer enable in the record path. Referred to as Mixer 2.
    0 = mixer disabled (default).
    1 = mixer enabled. */
    uint8_t MX2EN: 1;
    /* Gain for a right channel single-ended input from the RINN pin, input to Mixer 2.
    Setting Gain
    000 Mute (default)
    001 −12 dB
    010 −9 dB
    011 −6 dB
    100 −3 dB
    101 0 dB
    110 3 dB
    111 6 dB */
    uint8_t RINNG: 3;
    /* Gain for a right channel single-ended input from the RINP pin, input to Mixer 2.
    Setting Gain
    000 Mute (default)
    001 −12 dB
    010 −9 dB
    011 −6 dB
    100 −3 dB
    101 0 dB
    110 3 dB
    111 6 dB */
    uint8_t RINPG: 3;
    /*  */
    uint8_t Reserved: 1;
} R6_0x400C_Rec_Mixer_Right_0;

typedef struct __attribute__ ((packed))
{
    /* Right single-ended auxiliary input gain from the RAUX pin in the record path, input to Mixer 2.
    Setting Auxiliary Input Gain
    000 Mute (default)
    001 −12 dB
    010 −9 dB
    011 −6 dB
    100 −3 dB
    101 0 dB
    110 3 dB
    111 6 dB */
    uint8_t MX2AUXG: 3;
    /* Right channel differential PGA input gain boost, input to Mixer 2. The right differential input uses the RINP
    (positive signal) and RINN (negative signal) pins.
    Setting Gain Boost
    00 Mute (default)
    01 0 dB
    10 20 dB
    11 Reserved */
    uint8_t RDBOOST: 2;
    /*  */
    uint8_t Reserved: 3;
} R7_0x400D_Rec_Mixer_Right_1;

typedef struct __attribute__ ((packed))
{
    /* Left differential PGA enable. When enabled, the LINP and LINN pins are used as a full differential pair. When
    disabled, these two pins are configured as two single-ended inputs with the signals routed around the PGA.
    0 = disabled (default).
    1 = enabled. */
    uint8_t LDEN: 1;
    /* Left differential input mute control.
    0 = mute (default).
    1 = unmute. */
    uint8_t LDMUTE: 1;
    /* Left channel differential PGA input volume control. The left differential input uses the LINP (positive signal) and
    LINN (negative signal) pins. Each step corresponds to a 0.75 dB increase in gain. See Table 92 for a complete list
    of the volume settings.
    Setting Volume
    000000 −12 dB (default)
    000001 −11.25 dB
    … …
    010000 0 dB
    … …
    111110 34.5 dB
    111111 35.25 dB */
    uint8_t LDVOL: 6;
} R8_0x400E_Left_diff_input_vol;

typedef struct __attribute__ ((packed))
{
    /* Right differential PGA enable. When enabled, the RINP and RINN pins are used as a full differential pair. When
    disabled, these two pins are configured as two single-ended inputs with the signals routed around the PGA.
    0 = disabled (default).
    1 = enabled. */
    uint8_t RDEN: 1;
    /* Right differential input mute control.
    0 = mute (default).
    1 = unmute. */
    uint8_t RDMUTE: 1;
    /* RDVOL[5:0] Right channel differential PGA input volume control. The right differential input uses the RINP (positive signal)
    and RINN (negative signal) pins. Each step corresponds to a 0.75 dB increase in gain. See Table 92 for a complete
    list of the volume settings.
    Setting Volume
    000000 −12 dB (default)
    000001 −11.25 dB
    … …
    010000 0 dB
    … …
    111110 34.5 dB
    111111 35.25 dB */
    uint8_t RDVOL: 6;
} R9_0x400F_Right_diff_input_vol;

typedef struct __attribute__ ((packed))
{
    /* Enables the MICBIAS output.
    0 = disabled (default).
    1 = enabled. */
    uint8_t MBIEN: 1;
    /*  */
    uint8_t Reserved1: 1;
    /* Microphone voltage bias as a fraction of AVDD.
    0 = 0.90 × AVDD (default).
    1 = 0.65 × AVDD. */
    uint8_t MBI: 1;
    /* Microphone bias is enabled for high performance or normal operation. High performance operation sources
    more current to the microphone.
    0 = normal operation (default).
    1 = high performance. */
    uint8_t MPERF: 1;
    /*  */
    uint8_t Reserved2: 4;
} R10_0x4010_Record_mic_bias;

typedef struct __attribute__ ((packed))
{
    /* ALC select. These bits set the channels that are controlled by the ALC. When set to right only, the ALC responds
    only to the right channel input and controls the gain of the right PGA amplifier only. When set to left only, the
    ALC responds only to the left channel input and controls the gain of the left PGA amplifier only. When set to
    stereo, the ALC responds to the greater of the left or right channel and controls the gain of both the left and
    right PGA amplifiers. DSP control allows the PGA gain to be set within the DSP or from external GPIO inputs.
    These bits must be off if manual control of the volume is desired.
    Setting Channels
    000 Off (default)
    001 Right only
    010 Left only
    011 Stereo
    100 DSP control
    101 Reserved
    110 Reserved
    111 Reserved */
    uint8_t ALCSEL: 3;
    /* The maximum ALC gain sets a limit to the amount of gain that the ALC can provide to the input signal. This
    protects small signals from excessive amplification.
    Setting Maximum ALC Gain
    000 −12 dB (default)
    001 −6 dB
    010 0 dB
    011 6 dB
    100 12 dB
    101 18 dB
    110 24 dB
    111 30 dB */
    uint8_t ALCMAX: 3;
    /* PGA volume slew time when the ALC is off. The slew time is the period of time that a volume increase or decrease
    takes to ramp up or ramp down to the target volume set in Register R8 (left differential input volume control)
    and Register R9 (right differential input volume control).
    Setting Slew Time
    00 24 ms (default)
    01 48 ms
    10 96 ms
    11 Off */
    uint8_t PGASLEW: 2;
} R11_0x4011_ALC_0;

typedef struct __attribute__ ((packed))
{
    /* ALC target. The ALC target sets the desired ADC input level. The PGA gain is adjusted by the ALC to reach this
    target level. The recommended target level is between −16 dB and −10 dB to accommodate transients without
    clipping the ADC.
    Setting ALC Target
    0000 −28.5 dB (default)
    0001 −27 dB
    0010 −25.5 dB
    0011 −24 dB
    0100 −22.5 dB
    0101 −21 dB
    0110 −19.5 dB
    0111 −18 dB
    1000 −16.5 dB
    1001 −15 dB
    1010 −13.5 dB
    1011 −12 dB
    1100 −10.5 dB
    1101 −9 dB
    1110 −7.5 dB
    1111 −6 dB */
    uint8_t ALCTARG: 4;
    /* ALC hold time. The ALC hold time is the amount of time that the ALC waits after a decrease in input level before
    increasing the gain to achieve the target level. The recommended minimum setting is 21 ms (0011) to prevent
    distortion of low frequency signals. The hold time doubles with every 1-bit increase.
    Setting Hold Time
    0000 2.67 ms (default)
    0001 5.34 ms
    0010 10.68 ms
    0011 21.36 ms
    0100 42.72 ms
    0101 85.44 ms
    0110 170.88 ms
    0111 341.76 ms
    1000 683.52 ms
    1001 1.367 sec
    1010 2.7341 sec
    1011 5.4682 sec
    1100 10.936 sec
    1101 21.873 sec
    1110 43.745 sec
    1111 87.491 sec */
    uint8_t ALCHOLD: 4;
} R12_0x4012_ALC_1;

typedef struct __attribute__ ((packed))
{
    /* ALC decay time. The decay time sets how fast the ALC increases the PGA gain after a decrease in input level
    below the target. A typical setting for music recording is 24.58 seconds, and a typical setting for voice recording
    is 1.54 seconds.
    Setting Decay Time
    0000 24 ms
    0001 48 ms
    0010 96 ms
    0011 192 ms
    0100 384 ms
    0101 768 ms
    0110 1.54 sec
    0111 3.07 sec
    1000 6.14 sec
    1001 12.29 sec
    1010 24.58 sec
    1011 49.15 sec
    1100 98.30 sec
    1101 196.61 sec
    1110 393.22 sec
    1111 786.43 sec */
    uint8_t ALCDEC: 4;
    /* ALC attack time. The attack time sets how fast the ALC starts attenuating after an increase in input level above
    the target. A typical setting for music recording is 384 ms, and a typical setting for voice recording is 24 ms.
    Setting Attack Time
    0000 6 ms (default)
    0001 12 ms
    0010 24 ms
    0011 48 ms
    0100 96 ms
    0101 192 ms
    0110 384 ms
    0111 768 ms
    1000 1.54 sec
    1001 3.07 sec
    1010 6.14 sec
    1011 12.29 sec
    1100 24.58 sec
    1101 49.15 sec
    1110 98.30 sec
    1111 196.61 sec */
    uint8_t ALCATCK: 4;
} R13_0x4013_ALC_2;

typedef struct __attribute__ ((packed))
{
    /* Noise gate threshold. When the input signal falls below the threshold for 250 ms, the noise gate is activated.
    A 1 LSB increase corresponds to a −1.5 dB change. See Table 93 for a complete list of the threshold settings.
    Setting Threshold
    00000 −76.5 dB (default)
    00001 −75 dB
    … …
    11110 −31.5 dB
    11111 −30 dB */
    uint8_t NGTHR: 5;
    /* Noise gate enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t NGEN: 1;
    /* Noise gate type. When the input signal falls below the threshold for 250 ms, the noise gate can hold a constant
    PGA gain, mute the ADC output, fade the PGA gain to the minimum gain value, or fade then mute.
    Setting Noise Gate
    00 Hold PGA constant (default)
    01 Mute ADC output (digital mute)
    10 Fade to PGA minimum value (analog fade)
    11 Fade then mute (analog fade/digital mute) */
    uint8_t NGTYP: 2;
} R14_0x4014_ALC_3;

typedef struct __attribute__ ((packed))
{
    /* Serial data port bus mode. Both LRCLK and BCLK are master of the serial port when set in master mode and are
    serial port slave in slave mode.
    0 = slave mode (default).
    1 = master mode. */
    uint8_t MS: 1;
    /* Channels per frame sets the number of channels per LRCLK frame.
    Setting Channels per LRCLK Frame
    00 Stereo (default)
    01 TDM 4
    10 TDM 8
    11 Reserved */
    uint8_t CHPF: 2;
    /* LRCLK polarity sets the LRCLK edge that triggers the beginning of the left channel audio frame. This can be set
    for the falling or rising edge of the LRCLK.
    0 = falling edge (default).
    1 = rising edge. */
    uint8_t LRPOL: 1;
    /* BCLK polarity sets the BCLK edge that triggers a change in audio data. This can be set for the falling or rising
    edge of the BCLK.
    0 = falling edge (default).
    1 = rising edge. */
    uint8_t BPOL: 1;
    /* LRCLK mode sets the LRCLK for either a 50% duty cycle or a pulse. The pulse mode should be at least 1 BCLK wide.
    0 = 50% duty cycle (default).
    1 = pulse mode. */
    uint8_t LRMOD: 1;
    /* Serial port sampling rate source.
    0 = converter rate set in Register R17 (default).
    1 = DSP rate set in Register R57. */
    uint8_t SPSRS: 1;
    /*  */
    uint8_t Reserved: 1;
} R15_0x4015_Serial_Port_0;

typedef struct __attribute__ ((packed))
{
    /* Data delay from LRCLK edge (in BCLK units).
    Setting Delay (Bit Clock Cycles)
    00 1 (default)
    01 0
    10 8
    11 16 */
    uint8_t LRDEL: 2;
    /* MSB position in the LRCLK frame.
    0 = MSB first (default).
    1 = LSB first. */
    uint8_t MSBP: 1;
    /* DAC serial audio data channel position in TDM mode.
    0 = left first (default).
    1 = right first. */
    uint8_t DATDM: 1;
    /* ADC serial audio data channel position in TDM mode.
    0 = left first (default).
    1 = right first. */
    uint8_t ADTDM: 1;
    /* Number of bit clock cycles per LRCLK audio frame.
    Setting Bit Clock Cycles
    000 64 (default)
    001 Reserved
    010 48
    011 128
    100 256
    101 Reserved
    110 Reserved
    111 Reserved */
    uint8_t BPF: 3;
} R16_0x4016_Serial_Port_1;

typedef struct __attribute__ ((packed))
{
    /* Converter sampling rate. The ADCs and DACs operate at the sampling rate set in this register. The converter rate
    selected is a ratio of the base sampling rate, fS. The base sampling rate is determined by the operating frequency
    of the core clock.
    Setting Sampling Rate Base Sampling Rate (fS = 48 kHz)
    000 fS 48 kHz, base (default)
    001 fS/6 8 kHz
    010 fS/4 12 kHz
    011 fS/3 16 kHz
    100 fS/2 24 kHz
    101 fS/1.5 32 kHz
    110 fS/0.5 96 kHz
    111 Reserved */
    uint8_t CONVSR: 3;
    /* ADC oversampling ratio. This bit cannot be set for 64× when CONVSR[2:0] is set to 96 kHz.
    0 = 128× (default).
    1 = 64×. */
    uint8_t ADOSR: 1;
    /* DAC oversampling ratio. This bit cannot be set for 64× when CONVSR[2:0] is set to 96 kHz.
    0 = 128× (default).
    1 = 64×. */
    uint8_t DAOSR: 1;
    /* On-chip DAC serial data selection in TDM 4 or TDM 8 mode.
    Setting Pair
    00 First pair (default)
    01 Second pair
    10 Third pair
    11 Fourth pair */
    uint8_t DAPAIR: 2;
    /*  */
    uint8_t Reserved: 1;
} R17_0x4017_Converter_0;

typedef struct __attribute__ ((packed))
{
    /* On-chip ADC serial data selection in TDM 4 or TDM 8 mode.
    Setting Pair
    00 First pair (default)
    01 Second pair
    10 Third pair
    11 Fourth pair */
    uint8_t ADPAIR: 2;
    /*  */
    uint8_t Reserved: 6;
} R18_0x4018_Converter_1;

typedef struct __attribute__ ((packed))
{
    /* ADC enable.
    Setting ADCs Enabled
    00 Both off (default)
    01 Left on
    10 Right on
    11 Both on */
    uint8_t ADCEN: 2;
    /* Digital microphone input select. When asserted, the on-chip ADCs are off, BCLK is master at 128 × fS, and
    ADC_SDATA is expected to have left and right channels interleaved.
    0 = digital microphone inputs off, ADCs enabled (default).
    1 = digital microphone inputs enabled, ADCs off. */
    uint8_t INSEL: 1;
    /* Digital microphone channel swap. Normal operation sends the left channel on the rising edge of the clock and
    the right channel on the falling edge of the clock.
    0 = normal (default).
    1 = swap left and right channels. */
    uint8_t DMSW: 1;
    /* Digital microphone data polarity swap.
    0 = invert polarity.
    1 = normal (default). */
    uint8_t DMPOL: 1;
    /* ADC high-pass filter select. At 48 kHz, f3dB = 2 Hz.
    0 = off (default).
    1 = on. */
    uint8_t HPF: 1;
    /* Invert input polarity.
    0 = normal (default).
    1 = inverted. */
    uint8_t ADCPOL: 1;
    /*  */
    uint8_t Reserved: 1;
} R19_0x4019_ADC_control;

typedef struct __attribute__ ((packed))
{
    /* Controls the digital volume attenuation for left channel inputs from either the left ADC or the left digital
    microphone input. Each bit corresponds to a 0.375 dB step with slewing between settings. See Table 94 for a
    complete list of the volume settings.
    Setting Volume Attenuation
    00000000 0 dB (default)
    00000001 −0.375 dB
    00000010 −0.75 dB
    … …
    11111110 −95.25 dB
    11111111 −95.625 dB */
    uint8_t LADVOL;
} R20_0x401A_Left_digital_vol;

typedef struct __attribute__ ((packed))
{
    /* Controls the digital volume attenuation for right channel inputs from either the right ADC or the right digital
    microphone input. Each bit corresponds to a 0.375 dB step with slewing between settings. See Table 94 for a
    complete list of the volume settings.
    Setting Volume Attenuation
    00000000 0 dB (default)
    00000001 −0.375 dB
    00000010 −0.75 dB
    … …
    11111110 −95.25 dB
    11111111 −95.625 dB */
    uint8_t RADVOL;
} R21_0x401B_Right_digital_vol;

typedef struct __attribute__ ((packed))
{
    /* Mixer 3 enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t MX3EN: 1;
    /* Mixer input gain. Controls the left channel auxiliary input gain to the left channel playback mixer (Mixer 3).
    Setting Gain
    0000 Mute (default)
    0001 −15 dB
    0010 −12 dB
    0011 −9 dB
    0100 −6 dB
    0101 −3 dB
    0110 0 dB
    0111 3 dB
    1000 6 dB */
    uint8_t MX3AUXG: 4;
    /* Mixer input mute. Mutes the left DAC input to the left channel playback mixer (Mixer 3).
    0 = muted (default).
    1 = unmuted. */
    uint8_t MX3LM: 1;
    /* Mixer input mute. Mutes the right DAC input to the left channel playback mixer (Mixer 3).
    0 = muted (default).
    1 = unmuted. */
    uint8_t MX3RM: 1;
    /*  */
    uint8_t Reserved: 1;
} R22_0x401C_Play_Mixer_Left_0;

typedef struct __attribute__ ((packed))
{
    /* Bypass gain control. The signal from the left channel record mixer (Mixer 1) bypasses the converters and gain
    can be applied before the left playback mixer (Mixer 3).
    Setting Gain
    0000 Mute (default)
    0001 −15 dB
    0010 −12 dB
    0011 −9 dB
    0100 −6 dB
    0101 −3 dB
    0110 0 dB
    0111 3 dB
    1000 6 dB */
    uint8_t MX3G1: 4;
    /* Bypass gain control. The signal from the right channel record mixer (Mixer 2) bypasses the converters and gain
    can be applied before the left playback mixer (Mixer 3).
    Setting Gain
    0000 Mute (default)
    0001 −15 dB
    0010 −12 dB
    0011 −9 dB
    0100 −6 dB
    0101 −3 dB
    0110 0 dB
    0111 3 dB
    1000 6 dB */
    uint8_t MX3G2: 4;
} R23_0x401D_Play_Mixer_Left_1;

typedef struct __attribute__ ((packed))
{
    /* Mixer 4 enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t MX4EN: 1;
    /* Mixer input gain. Controls the right channel auxiliary input gain to the right channel playback mixer (Mixer 4).
    Setting Gain
    0000 Mute (default)
    0001 −15 dB
    0010 −12 dB
    0011 −9 dB
    0100 −6 dB
    0101 −3 dB
    0110 0 dB
    0111 3 dB
    1000 6 dB */
    uint8_t MX4AUXG: 4;
    /* Mixer input mute. Mutes the left DAC input to the right channel playback mixer (Mixer 4).
    0 = muted (default).
    1 = unmuted. */
    uint8_t MX4LM: 1;
    /* Mixer input mute. Mutes the right DAC input to the right channel playback mixer (Mixer 4).
    0 = muted (default).
    1 = unmuted. */
    uint8_t MX4RM: 1;
    /*  */
    uint8_t Reserved: 1;
} R24_0x401E_Play_Mixer_Right_0;

typedef struct __attribute__ ((packed))
{
    /* Bypass gain control. The signal from the left channel record mixer (Mixer 1) bypasses the converters and gain
    can be applied before the right playback mixer (Mixer 4).
    Setting Gain
    0000 Mute (default)
    0001 −15 dB
    0010 −12 dB
    0011 −9 dB
    0100 −6 dB
    0101 −3 dB
    0110 0 dB
    0111 3 dB
    1000 6 dB */
    uint8_t MX4G1: 4;
    /* Bypass gain control. The signal from the right channel record mixer (Mixer 2) bypasses the converters and gain
    can be applied before the right playback mixer (Mixer 4).
    Setting Gain
    0000 Mute (default)
    0001 −15 dB
    0010 −12 dB
    0011 −9 dB
    0100 −6 dB
    0101 −3 dB
    0110 0 dB
    0111 3 dB
    1000 6 dB */
    uint8_t MX4G2: 4;
} R25_0x401F_Play_Mixer_Right_1;

typedef struct __attribute__ ((packed))
{
    /* Mixer 5 enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t MX5EN: 1;
    /* Mixer input gain boost. The signal from the left channel playback mixer (Mixer 3) can be enabled and boosted in
    the playback L/R mixer left (Mixer 5).
    Setting Gain Boost
    00 Mute (default)
    01 0 dB output (−6 dB gain on each of the two inputs)
    10 6 dB output (0 dB gain on each of the two inputs)
    11 Reserved */
    uint8_t MX5G3: 2;
    /* Mixer input gain boost. The signal from the right channel playback mixer (Mixer 4) can be enabled and boosted
    in the playback L/R mixer left (Mixer 5).
    Setting Gain Boost
    00 Mute (default)
    01 0 dB output (−6 dB gain on each of the two inputs)
    10 6 dB output (0 dB gain on each of the two inputs)
    11 Reserved */
    uint8_t MX5G4: 2;
    /*  */
    uint8_t Reserved: 3;
} R26_0x4020_Play_L_R_mixer_left;

typedef struct __attribute__ ((packed))
{
    /* Mixer 6 enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t MX6EN: 1;
    /* Mixer input gain boost. The signal from the left channel playback mixer (Mixer 3) can be enabled and boosted in
    the playback L/R mixer right (Mixer 6).
    Setting Gain Boost
    00 Mute (default)
    01 0 dB output (−6 dB gain on each of the two inputs)
    10 6 dB output (0 dB gain on each of the two inputs)
    11 Reserved */
    uint8_t MX6G3: 2;
    /* Mixer input gain boost. The signal from the right channel playback mixer (Mixer 4) can be enabled and boosted
    in the playback L/R mixer right (Mixer 6).
    Setting Gain Boost
    00 Mute (default)
    01 0 dB output (−6 dB gain on each of the two inputs)
    10 6 dB output (0 dB gain on each of the two inputs)
    11 Reserved */
    uint8_t MX6G4: 2;
    /*  */
    uint8_t Reserved: 3;
} R27_0x4021_Play_L_R_mixer_right;

typedef struct __attribute__ ((packed))
{
    /* Mixer 7 enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t MX7EN: 1;
    /* L/R mono playback mixer (Mixer 7). Mixes the left and right playback mixers (Mixer 3 and Mixer 4) with either a
    0 dB or 6 dB gain boost. Additionally, this mixer can operate as a common-mode output, which is used as the
    virtual ground in a capless headphone configuration.
    Setting Gain Boost
    00 Common-mode output (default)
    01 0 dB output (−6 dB gain on each of the two inputs)
    10 6 dB output (0 dB gain on each of the two inputs)
    11 Reserved */
    uint8_t MX7: 2;
    /*  */
    uint8_t Reserved: 5;
} R28_0x4022_Play_L_R_mixer_mono;

typedef struct __attribute__ ((packed))
{
    /* Headphone volume control enable. Logical OR with the HPMODE bit in Register R30. If either the HPEN bit or
    the HPMODE bit is set to 1, the headphone output is enabled.
    0 = disabled (default).
    1 = enabled. */
    uint8_t HPEN: 1;
    /* Headphone mute for left channel, LHP output (active low).
    0 = mute.
    1 = unmute (default). */
    uint8_t LHPM: 1;
    /* Headphone volume control for left channel, LHP output. Each 1-bit step corresponds to a 1 dB increase in volume.
    See Table 95 for a complete list of the volume settings.
    Setting Volume
    000000 −57 dB (default)
    … …
    111001 0 dB
    … …
    111111 6 dB */
    uint8_t LHPVOL: 6;
} R29_0x4023_Play_HP_left_vol;

typedef struct __attribute__ ((packed))
{
    /* RHP and LHP output mode. These pins can be configured for either line outputs or headphone outputs. Logical
    OR with the HPEN bit in Register R29. If either the HPMODE bit or the HPEN bit is set to 1, the headphone output
    is enabled.
    0 = enable line output (default).
    1 = enable headphone output. */
    uint8_t HPMODE: 1;
    /* Headphone mute for right channel, RHP output (active low).
    0 = mute.
    1 = unmute (default). */
    uint8_t RHPM: 1;
    /* Headphone volume control for right channel, RHP output. Each 1-bit step corresponds to a 1 dB increase in
    volume. See Table 95 for a complete list of the volume settings.
    Setting Volume
    000000 −57 dB (default)
    … …
    111001 0 dB
    … …
    111111 6 dB */
    uint8_t RHPVOL: 6;
} R30_0x4024_Play_HP_right_vol;

typedef struct __attribute__ ((packed))
{
    /* Line output mode for left channel, LOUTN and LOUTP outputs. These pins can be configured for either line
    outputs or headphone outputs. To drive earpiece speakers, set this bit to 1 (headphone output).
    0 = line output (default).
    1 = headphone output. */
    uint8_t LOMODE: 1;
    /* Line output mute for left channel, LOUTN and LOUTP outputs (active low).
    0 = mute.
    1 = unmute (default). */
    uint8_t LOUTM: 1;
    /* Line output volume control for left channel, LOUTN and LOUTP outputs. Each 1-bit step corresponds to a 1 dB
    increase in volume. See Table 95 for a complete list of the volume settings.
    Setting Volume
    000000 −57 dB (default)
    … …
    111001 0 dB
    … …
    111111 6 dB */
    uint8_t LOUTVOL: 6;
} R31_0x4025_Line_output_left_vol;

typedef struct __attribute__ ((packed))
{
    /* Line output mode for right channel, ROUTN and ROUTP outputs. These pins can be configured for either line
    outputs or headphone outputs. To drive earpiece speakers, set this bit to 1 (headphone output).
    0 = line output (default).
    1 = headphone output. */
    uint8_t ROMODE: 1;
    /* Line output mute for right channel, ROUTN and ROUTP outputs (active low).
    0 = mute.
    1 = unmute (default). */
    uint8_t ROUTM: 1;
    /* Line output volume control for right channel, ROUTN and ROUTP outputs. Each 1-bit step corresponds to a 1 dB
    increase in volume. See Table 95 for a complete list of the volume settings.
    Setting Volume
    000000 −57 dB (default)
    … …
    111001 0 dB
    … …
    111111 6 dB */
    uint8_t ROUTVOL: 6;
} R32_0x4026_Line_output_right_vol;

typedef struct __attribute__ ((packed))
{
    /* Headphone mode enable. If MX7[1:0] in Register R28 is set for common-mode output for a capless headphone
    configuration, this bit should be set to 1 (headphone output).
    0 = line output (default).
    1 = headphone output. */
    uint8_t MOMODE: 1;
    /* Mono output mute (active low).
    0 = mute.
    1 = unmute (default). */
    uint8_t MONOM: 1;
    /* MONOVOL[5:0] Mono output volume control. Each 1-bit step corresponds to a 1 dB increase in volume. If MX7[1:0] in Register R28
    is set for common-mode output, volume control is disabled. See Table 95 for a complete list of the volume
    settings.
    Setting Volume
    000000 −57 dB (default)
    … …
    111001 0 dB
    … …
    111111 6 dB */
    uint8_t MONOVOL: 6;
} R33_0x4027_Play_mono_output;

typedef struct __attribute__ ((packed))
{
    /*  */
    uint8_t Reserved1: 1;
    /* Analog volume slew rate for playback volume controls.
    Setting Slew Rate
    00 21.25 ms (default)
    01 42.5 ms
    10 85 ms
    11 Off */
    uint8_t ASLEW: 2;
    /* Pop suppression disable. The pop suppression circuits are enabled by default. They can be disabled to save
    power; however, disabling the circuits increases the risk of pops and clicks.
    0 = enabled (default).
    1 = disabled. */
    uint8_t POPLESS: 1;
    /* Pop suppression circuit power saving mode. The pop suppression circuits charge faster in normal operation;
    however, after they are charged, they can be put into low power operation.
    0 = normal (default).
    1 = low power. */
    uint8_t POPMODE: 1;
    /*  */
    uint8_t Reserved2: 3;
} R34_0x4028_Pop_click_suppress;

typedef struct __attribute__ ((packed))
{
    /* Playback left channel enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t PLEN: 1;
    /* Playback right channel enable.
    0 = disabled (default).
    1 = enabled. */
    uint8_t PREN: 1;
    /* Playback path channel bias control.
    Setting Playback Path Bias Control
    00 Normal operation (default)
    01 Reserved
    10 Enhanced performance
    11 Power saving */
    uint8_t PBIAS: 2;
    /* DAC bias control.
    Setting DAC Bias Control
    00 Normal operation (default)
    01 Extreme power saving
    10 Enhanced performance
    11 Power saving */
    uint8_t DACBIAS: 2;
    /* Headphone bias control.
    Setting Headphone Bias Control
    00 Normal operation (default)
    01 Extreme power saving
    10 Enhanced performance
    11 Power saving */
    uint8_t HPBIAS: 2;
} R35_0x4029_Play_power_mgmt;

typedef struct __attribute__ ((packed))
{
    /* DAC enable.
    Setting DACs Enabled
    00 Both off (default)
    01 Left on
    10 Right on
    11 Both on */
    uint8_t DACEN: 2;
    /* DAC de-emphasis filter enable. The de-emphasis filter is designed for use with a sampling rate of 44.1 kHz only.
    0 = disabled (default).
    1 = enabled. */
    uint8_t DEMPH: 1;
    /*  */
    uint8_t Reserved: 2;
    /* Invert input polarity of the DACs.
    0 = normal (default).
    1 = inverted. */
    uint8_t DACPOL: 1;
    /* DAC mono mode. The DAC channels can be set to mono mode within the DAC and output on the left
    channel, the right channel, or both channels.
    Setting Mono Mode
    00 Stereo (default)
    01 Left channel in mono mode
    10 Right channel in mono mode
    11 Both channels in mono mode */
    uint8_t DACMONO: 2;
} R36_0x402A_DAC_Control_0;

typedef struct __attribute__ ((packed))
{
    /* Controls the digital volume attenuation for left channel inputs from the left DAC. Each bit corresponds to a
    0.375 dB step with slewing between settings. See Table 94 for a complete list of the volume settings.
    Setting Volume Attenuation
    00000000 0 dB (default)
    00000001 −0.375 dB
    00000010 −0.75 dB
    … …
    11111110 −95.25 dB
    11111111 −95.625 dB */
    uint8_t LDAVOL;
} R37_0x402B_DAC_Control_1;

typedef struct __attribute__ ((packed))
{
    /* Controls the digital volume attenuation for right channel inputs from the right DAC. Each bit corresponds to a
    0.375 dB step with slewing between settings. See Table 94 for a complete list of the volume settings.
    Setting Volume Attenuation
    00000000 0 dB (default)
    00000001 −0.375 dB
    00000010 −0.75 dB
    … …
    11111110 −95.25 dB
    11111111 −95.625 dB */
    uint8_t RDAVOL;
} R38_0x402C_DAC_Control_2;

typedef struct __attribute__ ((packed))
{
    /* BCLK pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t BCLKP: 2;
    /* LRCLK pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t LRCLKP: 2;
    /* DAC_SDATA pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t DACSDP: 2;
    /* ADC_SDATA pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t ADCSDP: 2;
} R39_0x402D_Serial_port_pad;

typedef struct __attribute__ ((packed))
{
    /* SDA/COUT pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t SDAP: 2;
    /* SCL/CCLK pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t SCLP: 2;
    /* CLATCH pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t CLCHP: 2;
    /* CDATA pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t CDATP: 2;
} R40_0x402F_Control_Port_Pad_0;

typedef struct __attribute__ ((packed))
{
    /* SDA/COUT pin drive strength.
    0 = low (default).
    1 = high. */
    uint8_t SDASTR: 1;
    /*  */
    uint8_t Reserved: 7;
} R41_0x4030_Control_Port_Pad_1;

typedef struct __attribute__ ((packed))
{
    /*  */
    uint8_t Reserved1: 2;
    /* JACKDET/MICIN pad pull-up/pull-down configuration.
    Setting Configuration
    00 Pull-up
    01 Reserved
    10 None (default)
    11 Pull-down */
    uint8_t JDP: 2;
    /*  */
    uint8_t Reserved2: 1;
    /* JACKDET/MICIN pin drive strength.
    0 = low (default).
    1 = high. */
    uint8_t JDSTR: 1;
    /*  */
    uint8_t Reserved3: 2;
} R42_0x4031_Jack_detect_pin;

typedef struct __attribute__ ((packed))
{
    /*  */
    uint8_t DEJIT;
} R67_0x4036_Dejitter_control;

typedef struct __attribute__ ((packed))
{
    /* CRC hash sum, Bits 32 (read-only register) */
    uint32_t CRC32;
} R43_0x40C0_Cyclic_redundancy_check;

typedef struct __attribute__ ((packed))
{
    /* CRC enable
    0 = disabled (default)
    1 = enabled */
    uint8_t CRCEN: 1;
    /*  */
    uint8_t Reserved: 7;
} R47_0x40C4_CRC_enable;

typedef struct __attribute__ ((packed))
{
    /* GPIOx[3:0] Bits GPIO Pin Function
    0000 Input without debounce (default)
    0001 Input with debounce (0.3 ms)
    0010 Input with debounce (0.6 ms)
    0011 Input with debounce (0.9 ms)
    0100 Input with debounce (5 ms)
    0101 Input with debounce (10 ms)
    0110 Input with debounce (20 ms)
    0111 Input with debounce (40 ms)
    1000 Input controlled by I2C/SPI port
    1001 Output set by I2C/SPI port, with pull-up
    1010 Output set by I2C/SPI port, no pull-up
    1011 Output set by DSP core, with pull-up
    1100 Output set by DSP core, no pull-up
    1101 Reserved
    1110 Output CRC error (sticky)
    1111 Output watchdog error (sticky) */
    uint8_t GPIO0: 4;
    /*  */
    uint8_t Reserved: 4;
} R48_0x40C6_GPIO0_pin_control;

typedef struct __attribute__ ((packed))
{
    /* GPIOx[3:0] Bits GPIO Pin Function
    0000 Input without debounce (default)
    0001 Input with debounce (0.3 ms)
    0010 Input with debounce (0.6 ms)
    0011 Input with debounce (0.9 ms)
    0100 Input with debounce (5 ms)
    0101 Input with debounce (10 ms)
    0110 Input with debounce (20 ms)
    0111 Input with debounce (40 ms)
    1000 Input controlled by I2C/SPI port
    1001 Output set by I2C/SPI port, with pull-up
    1010 Output set by I2C/SPI port, no pull-up
    1011 Output set by DSP core, with pull-up
    1100 Output set by DSP core, no pull-up
    1101 Reserved
    1110 Output CRC error (sticky)
    1111 Output watchdog error (sticky) */
    uint8_t GPIO1: 4;
    /*  */
    uint8_t Reserved: 4;
} R49_0x40C7_GPIO1_pin_control;

typedef struct __attribute__ ((packed))
{
    /* GPIOx[3:0] Bits GPIO Pin Function
    0000 Input without debounce (default)
    0001 Input with debounce (0.3 ms)
    0010 Input with debounce (0.6 ms)
    0011 Input with debounce (0.9 ms)
    0100 Input with debounce (5 ms)
    0101 Input with debounce (10 ms)
    0110 Input with debounce (20 ms)
    0111 Input with debounce (40 ms)
    1000 Input controlled by I2C/SPI port
    1001 Output set by I2C/SPI port, with pull-up
    1010 Output set by I2C/SPI port, no pull-up
    1011 Output set by DSP core, with pull-up
    1100 Output set by DSP core, no pull-up
    1101 Reserved
    1110 Output CRC error (sticky)
    1111 Output watchdog error (sticky) */
    uint8_t GPIO2: 4;
    /*  */
    uint8_t Reserved: 4;
} R50_0x40C8_GPIO2_pin_control;

typedef struct __attribute__ ((packed))
{
    /* GPIOx[3:0] Bits GPIO Pin Function
    0000 Input without debounce (default)
    0001 Input with debounce (0.3 ms)
    0010 Input with debounce (0.6 ms)
    0011 Input with debounce (0.9 ms)
    0100 Input with debounce (5 ms)
    0101 Input with debounce (10 ms)
    0110 Input with debounce (20 ms)
    0111 Input with debounce (40 ms)
    1000 Input controlled by I2C/SPI port
    1001 Output set by I2C/SPI port, with pull-up
    1010 Output set by I2C/SPI port, no pull-up
    1011 Output set by DSP core, with pull-up
    1100 Output set by DSP core, no pull-up
    1101 Reserved
    1110 Output CRC error (sticky)
    1111 Output watchdog error (sticky) */
    uint8_t GPIO3: 4;
    /*  */
    uint8_t Reserved: 4;
} R51_0x40C9_GPIO3_pin_control;

typedef struct __attribute__ ((packed))
{
    /* Watchdog enable bit.
    0 = disabled (default).
    1 = enabled. */
    uint8_t DOGEN: 1;
    /*  */
    uint8_t Reserved: 7;
} R52_0x40D0_Watchdog_enable;

typedef struct __attribute__ ((packed))
{
    /* Watchdog value, Bits 24.
    DOG[23:16] DOG[15:8] DOG[7:0] Hex Value
    00000000 00000000 00000000 0x000000 (default)
    … … … …
    11111111 11111111 11111111 0xFFFFFF */
    uint32_t DOG: 24;
} R53_0x40D1_Watchdog_value;

typedef struct __attribute__ ((packed))
{
    /* Watchdog error (read-only bit).
    0 = no error (default).
    1 = error. */
    uint8_t DOGER: 1;
    /*  */
    uint8_t Reserved: 7;
} R56_0x40D4_Watchdog_error;

typedef struct __attribute__ ((packed))
{
    /* SigmaDSP core sampling rate. The DSP sampling rate is a ratio of the base sampling rate, fS. The base sampling rate
    is determined by the operating frequency of the core clock. For most applications, the SigmaDSP core sampling
    rate should equal the converter sampling rate (set using the CONVSR[2:0] bits in Register R17) and the serial
    port sampling rate (set using the SPSR[2:0] bits in Register R64).
    Setting Sampling Rate Base Sampling Rate (fS = 48 kHz)
    0000 fS/0.5 96 kHz, base
    0001 fS 48 kHz (default)
    0010 fS/1.5 32 kHz
    0011 fS/2 24 kHz
    0100 fS/3 16 kHz
    0101 fS/4 12 kHz
    0110 fS/6 8 kHz
    0111 Serial input data rate
    1000 Serial output data rate
    1111 None */
    uint8_t DSPSR: 4;
    /*  */
    uint8_t Reserved: 4;
} R57_0x40EB_DSP_sampling_rate_setting;

typedef struct __attribute__ ((packed))
{
    /* Serial data input routing. This register sets the input where the DACs receive serial data. This location can be
    from the DSP or from any TDM slot on the serial port.
    Setting Routing
    0000 DSP to DACs [L, R] (default)
    0001 Serial input [L0, R0] to DACs [L, R]
    0010 Reserved
    0011 Serial input [L1, R1] to DACs [L, R]
    0100 Reserved
    0101 Serial input [L2, R2] to DACs [L, R]
    0110 Reserved
    0111 Serial input [L3, R3] to DACs [L, R]
    1000 Reserved
    1001 Serial input [R0, L0] to DACs [L, R]
    1010 Reserved
    1011 Serial input [R1, L1] to DACs [L, R]
    1100 Reserved
    1101 Serial input [R2, L2] to DACs [L, R]
    1110 Reserved
    1111 Serial input [R3, L3] to DACs [L, R] */
    uint8_t SINRT: 4;
    /*  */
    uint8_t Reserved: 4;
} R58_0x40F2_Serial_input_route_control;

typedef struct __attribute__ ((packed))
{
    /* Serial data output routing. This register sets the output where the ADCs send serial data. This location can be to
    the DSP or to any TDM slot on the serial port.
    Setting Routing
    0000 ADCs [L, R] to DSP (default)
    0001 ADCs [L, R] to serial output [L0, R0]
    0010 Reserved
    0011 ADCs [L, R] to serial output [L1, R1]
    0100 Reserved
    0101 ADCs [L, R] to serial output [L2, R2]
    0110 Reserved
    0111 ADCs [L, R] to serial output [L3, R3]
    1000 Reserved
    1001 ADCs [L, R] to serial output [R0, L0]
    1010 Reserved
    1011 ADCs [L, R] to serial output [R1, L1]
    1100 Reserved
    1101 ADCs [L, R] to serial output [R2, L2]
    1110 Reserved
    1111 ADCs [L, R] to serial output [R3, L3] */
    uint8_t SOUTRT: 4;
    /*  */
    uint8_t Reserved: 4;
} R59_0x40F3_Serial_output_route_control;

typedef struct __attribute__ ((packed))
{
    /* DAC_SDATA or GPIO0 pin configuration select.
    0 = DAC_SDATA enabled (default).
    1 = GPIO0 enabled. */
    uint8_t SDIGP0: 1;
    /* ADC_SDATA or GPIO1 pin configuration select.
    0 = ADC_SDATA enabled (default).
    1 = GPIO1 enabled. */
    uint8_t SDOGP1: 1;
    /* BCLK or GPIO2 pin configuration select.
    0 = BCLK enabled (default).
    1 = GPIO2 enabled. */
    uint8_t BGP2: 1;
    /* LRCLK or GPIO3 pin configuration select.
    0 = LRCLK enabled (default).
    1 = GPIO3 enabled. */
    uint8_t LRGP3: 1;
    /*  */
    uint8_t Reserved: 4;
} R60_0x40F4_Serial_data_GPIO_pin_configuration;

typedef struct __attribute__ ((packed))
{
    /* Enables the DSP. Set this bit before writing to the parameter RAM and before setting the DSPRUN bit in
    Register R62 (Address 0x40F6).
    0 = DSP disabled (default).
    1 = DSP enabled. */
    uint8_t DSPEN: 1;
    /*  */
    uint8_t Reserved: 7;
} R61_0x40F5_DSP_enable;

typedef struct __attribute__ ((packed))
{
    /* Run the DSP. Set the DSPEN bit in Register R61 (Address 0x40F5) before setting this bit.
    0 = DSP off (default).
    1 = run the DSP. */
    uint8_t DSPRUN: 1;
    /*  */
    uint8_t Reserved: 7;
} R62_0x40F6_DSP_run;

typedef struct __attribute__ ((packed))
{
    /* Headphone left slew generation.
    0 = codec (default).
    1 = DSP. */
    uint8_t LHPSLW: 1;
    /* Headphone right slew generation.
    0 = codec (default).
    1 = DSP. */
    uint8_t RHPSLW: 1;
    /* Line output left slew generation.
    0 = codec (default).
    1 = DSP. */
    uint8_t LOSLW: 1;
    /* Line output right slew generation.
    0 = codec (default).
    1 = DSP. */
    uint8_t ROSLW: 1;
    /* Mono output slew generation.
    0 = codec (default).
    1 = DSP. */
    uint8_t MOSLW: 1;
    /*  */
    uint8_t Reserved: 3;
} R63_0x40F7_DSP_slew_modes;

typedef struct __attribute__ ((packed))
{
    /* Serial port sampling rate. The serial port sampling rate is a ratio of the base sampling rate, fS. The base sampling
    rate is determined by the operating frequency of the core clock. For most applications, the serial port sampling
    rate should equal the converter sampling rate (set using the CONVSR[2:0] bits in Register R17) and the DSP sampling
    rate (set using the DSPSR[3:0] bits in Register R57).
    Setting Sampling Rate Base Sampling Rate (fS = 48 kHz)
    000 fS 48 kHz, base (default)
    001 fS/6 8 kHz
    010 fS/4 12 kHz
    011 fS/3 16 kHz
    100 fS/2 24 kHz
    101 fS/1.5 32 kHz
    110 fS/0.5 96 kHz
    111 Reserved */
    uint8_t SPSR: 3;
    /*  */
    uint8_t Reserved: 5;
} R64_0x40F8_Serial_port_sampling_rate;

typedef struct __attribute__ ((packed))
{
    /* Serial port digital clock engine enable.
    0 = powered down (default).
    1 = enabled. */
    uint8_t SPPD: 1;
    /* Serial routing inputs digital clock engine enable.
    0 = powered down (default).
    1 = enabled. */
    uint8_t SINPD: 1;
    /* Interpolator resync (dejitter) digital clock engine enable.
    0 = powered down (default).
    1 = enabled. */
    uint8_t INTPD: 1;
    /* Serial routing outputs digital clock engine enable.
    0 = powered down (default).
    1 = enabled. */
    uint8_t SOUTPD: 1;
    /* Decimator resync (dejitter) digital clock engine enable.
    0 = powered down (default).
    1 = enabled. */
    uint8_t DECPD: 1;
    /* ALC digital clock engine enable.
    0 = powered down (default).
    1 = enabled. */
    uint8_t ALCPD: 1;
    /* Codec slew digital clock engine enable. When powered down, the analog playback path volume controls are
    disabled and stay set to their current state.
    0 = powered down (default).
    1 = enabled. */
    uint8_t SLEWPD: 1;
    /*  */
    uint8_t Reserved: 1;
} R65_0x40F9_Clock_Enable_0;

typedef struct __attribute__ ((packed))
{
    /* Digital Clock Generator 0.
    0 = off (default).
    1 = on. */
    uint8_t CLK0: 1;
    /* Digital Clock Generator 1.
    0 = off (default).
    1 = on. */
    uint8_t CLK1: 1;
    /*  */
    uint8_t Reserved: 6;
} R66_0x40FA_Clock_Enable_1;

/* DSP芯片ADAU1761初始化 */
rt_err_t adau1761_init(struct rt_i2c_bus_device *dev);

void adau1761_set_volume(uint8_t volume);
void adau1761_set_sampling_rate(uint32_t volume);

rt_err_t adau1761_player_start(void);
rt_err_t adau1761_player_stop(void);

#endif
