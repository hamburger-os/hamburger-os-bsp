#include "board.h"
#include "drv_eth.h"
#include "jl5104.h"

#define DRV_DEBUG
#define LOG_TAG             "jl5104"
#include <drv_log.h>

extern struct rt_stm32_eth stm32_eth_device;

static uint8_t PHY_ADDRESS_TAB[3] = {0,1,2};
// 初始化JL5104
int32_t JL5104_Init(void)
{
    uint8_t i;
    uint32_t timeout = 0;
    uint32_t regval = 0;
    int32_t status = JL5104_STATUS_OK;

    for(i = 0; i < 3; i++)
    {
        timeout = 0;
        if (JL5104_WritePHY(PHY_ADDRESS_TAB[i], JL5104_BCR, JL5104_BCR_SOFT_RESET) >= 0)
        {
            // 等待软件复位完成
            if (JL5104_ReadPHY(PHY_ADDRESS_TAB[i], JL5104_BCR, &regval) >= 0)
            {
                while (regval & JL5104_BCR_SOFT_RESET)
                {
                    if (JL5104_ReadPHY(PHY_ADDRESS_TAB[i], JL5104_BCR, &regval) < 0)
                    {
                        status = JL5104_STATUS_READ_ERROR;
                        return status;
                    }
                    rt_thread_mdelay(10);
                    timeout++;

                    if (timeout >= JL5104_TIMEOUT)
                    {
                        status = JL5104_STATUS_READ_ERROR;
                        return status;
                    }
                }
            }
            else
            {
                status = JL5104_STATUS_READ_ERROR;
                break;
            }
        }
        else
        {
            status = JL5104_STATUS_WRITE_ERROR;
            break;
        }

        JL5104_StartAutoNego(PHY_ADDRESS_TAB[i]); // 开启自动协商功能
    }
    return status;
}

// 读取PHY寄存器值
// reg要读取的寄存器地址
// 返回值:0 读取成功，-1 读取失败
int32_t JL5104_ReadPHY(uint16_t phyaddr, uint16_t reg, uint32_t *regval)
{
    if (HAL_ETH_ReadPHYRegister(&stm32_eth_device.heth, phyaddr, reg, regval) != HAL_OK)
        return -1;
    return 0;
}

// 向JL5104指定寄存器写入值
// reg:要写入的寄存器
// value:要写入的值
// 返回值:0 写入正常，-1 写入失败
int32_t JL5104_WritePHY(uint16_t phyaddr, uint16_t reg, uint16_t value)
{
    uint32_t temp = value;
    if (HAL_ETH_WritePHYRegister(&stm32_eth_device.heth, phyaddr, reg, temp) != HAL_OK)
        return -1;
    return 0;
}

// 开启JL5104的自协商功能
void JL5104_StartAutoNego(uint16_t phyaddr)
{
    uint32_t readval = 0;
    JL5104_ReadPHY(phyaddr, JL5104_BCR, &readval);
    readval |= JL5104_BCR_AUTONEGO_EN;
    JL5104_WritePHY(phyaddr, JL5104_BCR, readval);
}
