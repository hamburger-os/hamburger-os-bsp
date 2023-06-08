/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-17     lvhan       the first version
 */
#ifndef BOARD_PORTS_DRV_I2S_H_
#define BOARD_PORTS_DRV_I2S_H_

void MX_I2S_Init(void);

void I2S_Samplerate_Set(uint32_t freq);
void I2S_Channels_Set(uint16_t channels);
void I2S_Samplebits_Set(uint16_t samplebits);

void I2S_TxHalfCpltCallback_Set(void (*callback)(void));
void I2S_TxCpltCallback_Set(void (*callback)(void));
void I2S_RxHalfCpltCallback_Set(void (*callback)(void));
void I2S_RxCpltCallback_Set(void (*callback)(void));
void I2S_ErrorCallback_Set(void (*callback)(void));

void I2S_Play_Start(const uint8_t *pData, uint16_t Size);
void I2S_Play_Stop(void);

void I2S_Rec_Start(uint8_t *pData, uint16_t Size);
void I2S_Rec_Stop(void);

#endif /* BOARD_PORTS_DRV_I2S_H_ */
