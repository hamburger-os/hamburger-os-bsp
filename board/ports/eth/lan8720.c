#include "board.h"
#include "drv_eth.h"
#include "lan8720.h"

#define DRV_DEBUG
#define LOG_TAG             "lan8720"
#include <drv_log.h>

extern struct rt_stm32_eth stm32_eth_device;

// 初始化LAN8720
int32_t LAN8720_Init(void)
{
    uint32_t timeout = 0;
    uint32_t regval = 0;
    int32_t status = LAN8720_STATUS_OK;

    if (LAN8720_WritePHY(LAN8720_BCR, LAN8720_BCR_SOFT_RESET) >= 0) // LAN8720软件复位
    {
        // 等待软件复位完成
        if (LAN8720_ReadPHY(LAN8720_BCR, &regval) >= 0)
        {
            while (regval & LAN8720_BCR_SOFT_RESET)
            {
                if (LAN8720_ReadPHY(LAN8720_BCR, &regval) < 0)
                {
                    status = LAN8720_STATUS_READ_ERROR;
                    break;
                }
                rt_thread_mdelay(10);
                timeout++;
                if (timeout >= LAN8720_TIMEOUT)
                    break; // 超时跳出,5S
            }
        }
        else
        {
            status = LAN8720_STATUS_READ_ERROR;
        }
    }
    else
    {
        status = LAN8720_STATUS_WRITE_ERROR;
    }

    LAN8720_StartAutoNego(); // 开启自动协商功能

    return status;
}

// 读取PHY寄存器值
// reg要读取的寄存器地址
// 返回值:0 读取成功，-1 读取失败
int32_t LAN8720_ReadPHY(uint16_t reg, uint32_t *regval)
{
    if (HAL_ETH_ReadPHYRegister(&stm32_eth_device.heth, stm32_eth_device.phy_addr, reg, regval) != HAL_OK)
        return -1;
    return 0;
}

// 向LAN8720指定寄存器写入值
// reg:要写入的寄存器
// value:要写入的值
// 返回值:0 写入正常，-1 写入失败
int32_t LAN8720_WritePHY(uint16_t reg, uint16_t value)
{
    uint32_t temp = value;
    if (HAL_ETH_WritePHYRegister(&stm32_eth_device.heth, stm32_eth_device.phy_addr, reg, temp) != HAL_OK)
        return -1;
    return 0;
}

// 打开LAN8720 Power Down模式
void LAN8720_EnablePowerDownMode(void)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_BCR, &readval);
    readval |= LAN8720_BCR_POWER_DOWN;
    LAN8720_WritePHY(LAN8720_BCR, readval);
}

// 关闭LAN8720 Power Down模式
void LAN8720_DisablePowerDownMode(void)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_BCR, &readval);
    readval &= ~LAN8720_BCR_POWER_DOWN;
    LAN8720_WritePHY(LAN8720_BCR, readval);
}

// 开启LAN8720的自协商功能
void LAN8720_StartAutoNego(void)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_BCR, &readval);
    readval |= LAN8720_BCR_AUTONEGO_EN;
    LAN8720_WritePHY(LAN8720_BCR, readval);
}

// 使能回测模式
void LAN8720_EnableLoopbackMode(void)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_BCR, &readval);
    readval |= LAN8720_BCR_LOOPBACK;
    LAN8720_WritePHY(LAN8720_BCR, readval);
}

// 关闭LAN8720的回测模式
void LAN8720_DisableLoopbackMode(void)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_BCR, &readval);
    readval &= ~LAN8720_BCR_LOOPBACK;
    LAN8720_WritePHY(LAN8720_BCR, readval);
}

// 使能中断，中断源可选:LAN8720_ENERGYON_IT
//                      LAN8720_AUTONEGO_COMPLETE_IT
//                      LAN8720_REMOTE_FAULT_IT
//                      LAN8720_LINK_DOWN_IT
//                      LAN8720_AUTONEGO_LP_ACK_IT
//                      LAN8720_PARALLEL_DETECTION_FAULT_IT
//                      LAN8720_AUTONEGO_PAGE_RECEIVED_IT
void LAN8720_EnableIT(uint32_t interrupt)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_IMR, &readval);
    readval |= interrupt;
    LAN8720_WritePHY(LAN8720_IMR, readval);
}

// 关闭中断，中断源可选:LAN8720_ENERGYON_IT
//                      LAN8720_AUTONEGO_COMPLETE_IT
//                      LAN8720_REMOTE_FAULT_IT
//                      LAN8720_LINK_DOWN_IT
//                      LAN8720_AUTONEGO_LP_ACK_IT
//                      LAN8720_PARALLEL_DETECTION_FAULT_IT
//                      LAN8720_AUTONEGO_PAGE_RECEIVED_IT
void LAN8720_DisableIT(uint32_t interrupt)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_IMR, &readval);
    readval &= ~interrupt;
    LAN8720_WritePHY(LAN8720_IMR, readval);
}

// 清除中断标志位，读寄存器ISFR就可清除中断标志位
void LAN8720_ClearIT(uint32_t interrupt)
{
    uint32_t readval = 0;
    LAN8720_ReadPHY(LAN8720_ISFR, &readval);
}

// 获取中断标志位
// 返回值，1 中断标志位置位，
//         0 中断标志位清零
uint8_t LAN8720_GetITStatus(uint32_t interrupt)
{
    uint32_t readval = 0;
    uint32_t status = 0;
    LAN8720_ReadPHY(LAN8720_ISFR, &readval);
    if (readval & interrupt)
        status = 1;
    else
        status = 0;
    return status;
}

// 获取LAN8720的连接状态
// 返回值：LAN8720_STATUS_LINK_DOWN              连接断开
//         LAN8720_STATUS_AUTONEGO_NOTDONE       自动协商完成
//         LAN8720_STATUS_100MBITS_FULLDUPLEX    100M全双工
//         LAN8720_STATUS_100MBITS_HALFDUPLEX    100M半双工
//         LAN8720_STATUS_10MBITS_FULLDUPLEX     10M全双工
//         LAN8720_STATUS_10MBITS_HALFDUPLEX     10M半双工
uint32_t LAN8720_GetLinkState(void)
{
    uint32_t readval = 0;

    // 读取两遍，确保读取正确！！！
    LAN8720_ReadPHY(LAN8720_BSR, &readval);
    LAN8720_ReadPHY(LAN8720_BSR, &readval);

    // 获取连接状态(硬件，网线的连接，不是TCP、UDP等软件连接！)
    if ((readval & LAN8720_BSR_LINK_STATUS) == 0)
        return LAN8720_STATUS_LINK_DOWN;

    // 获取自动协商状态
    LAN8720_ReadPHY(LAN8720_BCR, &readval);
    if ((readval & LAN8720_BCR_AUTONEGO_EN) != LAN8720_BCR_AUTONEGO_EN) // 未使能自动协商
    {
        if (((readval & LAN8720_BCR_SPEED_SELECT) == LAN8720_BCR_SPEED_SELECT) &&
            ((readval & LAN8720_BCR_DUPLEX_MODE) == LAN8720_BCR_DUPLEX_MODE))
            return LAN8720_STATUS_100MBITS_FULLDUPLEX;
        else if ((readval & LAN8720_BCR_SPEED_SELECT) == LAN8720_BCR_SPEED_SELECT)
            return LAN8720_STATUS_100MBITS_HALFDUPLEX;
        else if ((readval & LAN8720_BCR_DUPLEX_MODE) == LAN8720_BCR_DUPLEX_MODE)
            return LAN8720_STATUS_10MBITS_FULLDUPLEX;
        else
            return LAN8720_STATUS_10MBITS_HALFDUPLEX;
    }
    else // 使能了自动协商
    {
        LAN8720_ReadPHY(LAN8720_PHYSCSR, &readval);
        if ((readval & LAN8720_PHYSCSR_AUTONEGO_DONE) == 0)
            return LAN8720_STATUS_AUTONEGO_NOTDONE;
        if ((readval & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_100BTX_FD)
            return LAN8720_STATUS_100MBITS_FULLDUPLEX;
        else if ((readval & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_100BTX_HD)
            return LAN8720_STATUS_100MBITS_HALFDUPLEX;
        else if ((readval & LAN8720_PHYSCSR_HCDSPEEDMASK) == LAN8720_PHYSCSR_10BT_FD)
            return LAN8720_STATUS_10MBITS_FULLDUPLEX;
        else
            return LAN8720_STATUS_10MBITS_HALFDUPLEX;
    }
}

// 设置LAN8720的连接状态
// 参数linkstate：LAN8720_STATUS_100MBITS_FULLDUPLEX 100M全双工
//                LAN8720_STATUS_100MBITS_HALFDUPLEX 100M半双工
//                LAN8720_STATUS_10MBITS_FULLDUPLEX  10M全双工
//                LAN8720_STATUS_10MBITS_HALFDUPLEX  10M半双工
void LAN8720_SetLinkState(uint32_t linkstate)
{

    uint32_t bcrvalue = 0;
    LAN8720_ReadPHY(LAN8720_BCR, &bcrvalue);
    // 关闭连接配置，比如自动协商，速度和双工
    bcrvalue &= ~(LAN8720_BCR_AUTONEGO_EN | LAN8720_BCR_SPEED_SELECT | LAN8720_BCR_DUPLEX_MODE);
    if (linkstate == LAN8720_STATUS_100MBITS_FULLDUPLEX) // 100M全双工
        bcrvalue |= (LAN8720_BCR_SPEED_SELECT | LAN8720_BCR_DUPLEX_MODE);
    else if (linkstate == LAN8720_STATUS_100MBITS_HALFDUPLEX) // 100M半双工
        bcrvalue |= LAN8720_BCR_SPEED_SELECT;
    else if (linkstate == LAN8720_STATUS_10MBITS_FULLDUPLEX) // 10M全双工
        bcrvalue |= LAN8720_BCR_DUPLEX_MODE;

    LAN8720_WritePHY(LAN8720_BCR, bcrvalue);
}
