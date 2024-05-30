#include "board.h"
#include "drv_eth.h"
#include "yt8522.h"

#define DRV_DEBUG
#define LOG_TAG             "yt8522"
#include <drv_log.h>


#define YT8522_ADD        0

#define PHY_ID_REG_ADD    0x02

#define PHY_ID1_VAL       0x4F51

extern struct rt_stm32_eth stm32_eth_device;




// 读取PHY寄存器值
// reg要读取的寄存器地址
// 返回值:0 读取成功，-1 读取失败
static int32_t YT8522_ReadPHY(uint32_t regadd, uint32_t *regval)
{
    if (HAL_ETH_ReadPHYRegister(&stm32_eth_device.heth, YT8522_ADD, regadd, regval) != HAL_OK)
        return RT_ERROR;
    return RT_EOK;
}

// 向JL5104指定寄存器写入值
// reg:要写入的寄存器
// value:要写入的值
// 返回值:0 写入正常，-1 写入失败
static int32_t YT8522_WritePHY(uint32_t regadd, uint32_t value)
{
    uint32_t temp = value;
    if (HAL_ETH_WritePHYRegister(&stm32_eth_device.heth, YT8522_ADD, regadd, temp) != HAL_OK)
        return RT_ERROR;
    return RT_EOK;
}


static int32_t YT8522_Read_ExtReg(uint32_t regadd, uint32_t *regval)
{
    if(RT_EOK != YT8522_WritePHY(0x1e, regadd))
    {
        return RT_ERROR;
    }
    else if(RT_EOK != YT8522_ReadPHY(0x1f, regval))
    {
        return RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

static int32_t YT8522_Write_ExtReg(uint32_t regadd, uint32_t regval)
{
    if(RT_EOK != YT8522_WritePHY(0x1e, regadd))
    {
        return RT_ERROR;
    }
    else if(RT_EOK != YT8522_WritePHY(0x1f, regval))
    {
        return RT_ERROR;
    }
    else
    {
        return RT_ERROR;
    }
}

// 初始化JL5104
int32_t YT8522_Init(void)
{
    uint32_t regval;

    YT8522_ReadPHY(0x02, &regval);
    if(PHY_ID1_VAL != regval)
    {
        LOG_D("yt8851 id1 err, id1 is %x", regval);
        return RT_ERROR;
    }

    YT8522_Read_ExtReg(0x4000, &regval);
    regval |= 0x10;
    YT8522_Write_ExtReg(0x4000, regval);
    YT8522_Write_ExtReg(0x19, 0x9F);
    YT8522_Write_ExtReg(0x4001, 0x81D4);

    YT8522_Write_ExtReg(0x4210, 0);
    YT8522_Write_ExtReg(0x4008, 0xBF2A);
    YT8522_Write_ExtReg(0x2057, 0x297F);
    YT8522_Write_ExtReg(0x14, 0x1FE);
    YT8522_Write_ExtReg(0x15, 0x1FE);

    return RT_EOK;
}


// 开启JL5104的自协商功能
uint32_t YT8522_GetLinkState(void)
{
    uint32_t regval;
    uint32_t lint_sta;

    if(RT_EOK == YT8522_ReadPHY(0x11, &regval))
    {
        LOG_D("REG VAL is %x", regval);
        /* 已连接 */
        if(regval & (0x01<<10))
        {
            if(((regval & 0xE000) == 0))
            {
                lint_sta = YT8522_10M_HALF;
            }
            else if(((regval & 0xE000) == 0x2000))
            {
                lint_sta = YT8522_10M_FUL;
            }
            else if(((regval & 0xE000) == 0x4000))
            {
                lint_sta = YT8522_100M_HALF;
            }
            else if(((regval & 0xE000) == 0x6000))
            {
                lint_sta = YT8522_100M_FUL;
            }
            else
            {
                lint_sta = DIS_CONNECT;
            }
        }
        else
        {
            lint_sta = DIS_CONNECT;
        }
    }
    else
    {
        lint_sta = DIS_CONNECT;
    }

    return lint_sta;
}
