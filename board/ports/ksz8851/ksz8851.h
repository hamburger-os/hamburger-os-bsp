/*
 * @file ksz8851.c
 * ksz8851 ethernet control chip
 */

#ifndef KSZ8851_H_
#define KSZ8851_H_

#include "drv_fmc_eth.h"

#ifdef __cplusplus
}
#endif

/**
 * 寄存器地址定义
 */
#define KSZ8851_CCR (0x08)     /** 配置寄存器 */
#define KSZ8851_MARL (0x10)    /** MAC地址低字 */
#define KSZ8851_MARM (0x12)    /** MAC地址中字 */
#define KSZ8851_MARH (0x14)    /** MAC地址高字-6字节表示法中的前两字节 */
#define KSZ8851_OBCR (0x20)    /** 芯片总线控制 */
#define KSZ8851_EEPCR (0x22)   /** EEPROM控制 */
#define KSZ8851_MBIR (0x24)    /** 芯片内存自测试状态 */
#define KSZ8851_GRR (0x26)     /** 全局复位寄存器 */
#define KSZ8851_WFCR (0x2A)    /** 唤醒帧控制 */
#define KSZ8851_WF0CRC0 (0x30) /** 唤醒帧0CRC0寄存器 */
#define KSZ8851_WF0CRC1 (0x32) /** 唤醒帧0CRC1寄存器 */
#define KSZ8851_WF0BM0 (0x34)  /** 唤醒帧0字节掩码0寄存器 */
#define KSZ8851_WF0BM1 (0x36)  /** 唤醒帧0字节掩码1寄存器 */
#define KSZ8851_WF0BM2 (0x38)  /** 唤醒帧0字节掩码2寄存器 */
#define KSZ8851_WF0BM3 (0x3A)  /** 唤醒帧0字节掩码3寄存器 */
#define KSZ8851_WF1CRC0 (0x40) /** 唤醒帧1CRC0寄存器 */
#define KSZ8851_WF1CRC1 (0x42) /** 唤醒帧1CRC1寄存器 */
#define KSZ8851_WF1BM0 (0x44)  /** 唤醒帧1字节掩码0寄存器 */
#define KSZ8851_WF1BM1 (0x46)  /** 唤醒帧1字节掩码1寄存器 */
#define KSZ8851_WF1BM2 (0x48)  /** 唤醒帧1字节掩码2寄存器 */
#define KSZ8851_WF1BM3 (0x4A)  /** 唤醒帧1字节掩码3寄存器 */
#define KSZ8851_WF2CRC0 (0x50) /** 唤醒帧2CRC0寄存器 */
#define KSZ8851_WF2CRC1 (0x52) /** 唤醒帧2CRC1寄存器 */
#define KSZ8851_WF2BM0 (0x54)  /** 唤醒帧2字节掩码0寄存器 */
#define KSZ8851_WF2BM1 (0x56)  /** 唤醒帧2字节掩码1寄存器 */
#define KSZ8851_WF2BM2 (0x58)  /** 唤醒帧2字节掩码2寄存器 */
#define KSZ8851_WF2BM3 (0x5A)  /** 唤醒帧2字节掩码3寄存器 */
#define KSZ8851_WF3CRC0 (0x60) /** 唤醒帧3CRC0寄存器 */
#define KSZ8851_WF3CRC1 (0x62) /** 唤醒帧3CRC1寄存器 */
#define KSZ8851_WF3BM0 (0x64)  /** 唤醒帧3字节掩码0寄存器 */
#define KSZ8851_WF3BM1 (0x66)  /** 唤醒帧3字节掩码1寄存器 */
#define KSZ8851_WF3BM2 (0x68)  /** 唤醒帧3字节掩码2寄存器 */
#define KSZ8851_WF3BM3 (0x6A)  /** 唤醒帧3字节掩码3寄存器 */
#define KSZ8851_TXCR (0x70)    /** 发送控制寄存器 */
#define KSZ8851_TXSR (0x72)    /** 发送状态寄存器 */
#define KSZ8851_RXCR1 (0x74)   /** 接收状态寄存器1 */
#define KSZ8851_RXCR2 (0x76)   /** 接收状态寄存器2 */
#define KSZ8851_TXMIR (0x78)   /** TXQ存储区信息寄存器 */
#define KSZ8851_RXFHSR (0x7C)  /** 接收帧头状态寄存器 */
#define KSZ8851_RXFHBCR (0x7E) /** 接收帧头字节数寄存器 */
#define KSZ8851_TXQCR (0x80)   /** TXQ命令寄存器 */
#define KSZ8851_RXQCR (0x82)   /** RXQ命令寄存器 */
#define KSZ8851_TXFDPR (0x84)  /** 发送帧数据地址指针寄存器 */
#define KSZ8851_RXFDPR (0x86)  /** 接收帧数据地址指针寄存器 */
#define KSZ8851_RXDTTR (0x8C)  /** 接收数据持续时间阈值寄存器 */
#define KSZ8851_RXDBCTR (0x8E) /** 接收数据字节长度阈值寄存器 */
#define KSZ8851_IER (0x90)     /** 中断使能寄存器 */
#define KSZ8851_ISR (0x92)     /** 中断状态寄存器 */
#define KSZ8851_RXFCTR (0x9C)  /** 接收帧计数及阈值寄存器 */
#define KSZ8851_TXNTFSR (0x9E) /** 待发送帧长度请求寄存器 */
#define KSZ8851_MAHTR0 (0xA0)  /** 哈希过滤表寄存器0 */
#define KSZ8851_MAHTR1 (0xA2)  /** 哈希过滤表寄存器1 */
#define KSZ8851_MAHTR2 (0xA4)  /** 哈希过滤表寄存器2 */
#define KSZ8851_MAHTR3 (0xA6)  /** 哈希过滤表寄存器3 */
#define KSZ8851_FCLWR (0xB0)   /** 流控下限寄存器 */
#define KSZ8851_FCHWR (0xB2)   /** 流控上限寄存器 */
#define KSZ8851_FCOWR (0xB4)   /** 流控超限寄存器 */
#define KSZ8851_CIDER (0xC0)   /**  芯片ID寄存器 */
#define KSZ8851_CGCR (0xC6)    /**  芯片全局控制寄存器 */
#define KSZ8851_IACR (0xC8)
#define KSZ8851_IADLR (0xD0)
#define KSZ8851_IADHR (0xD2)
#define KSZ8851_PMECR (0xD4) /** 电源管理事件控制寄存器 */
#define KSZ8851_PHYRR (0xD8)
#define KSZ8851_P1MBCR (0xE4) /** MII寄存器基本控制寄存器 */
#define KSZ8851_P1MBSR (0xE6) /** MII寄存器基本状态寄存器 */
#define KSZ8851_P1ANAR (0xEC)
#define KSZ8851_P1SCLMD (0xF4) /** 端口1LinkMD控制/状态寄存器 */
#define KSZ8851_P1CR (0xF6)    /** 端口1控制寄存器 */
#define KSZ8851_P1SR (0xF8)    /** 端口1状态寄存器 */
/** 配置寄存器 */
#define CCR_EEPROM (1 << 9) /** EEPROM配置位 */
#define CCR_SPI (1 << 8)    /** SPI模式，手册中为保留 */
#define CCR_8BIT (1 << 7)   /** 8位总线模式 */
#define CCR_16BIT (1 << 6)  /** 16位总线模式 */
#define CCR_32BIT (1 << 5)  /** 32位总线模式，手册中为保留 */
#define CCR_SHARED (1 << 4) /** 地址数据总线共享模式 */
#define CCR_48PIN (1 << 1)  /** 48脚封装 */
#define CCR_32PIN (1 << 0)  /** 32脚封装，手册中为保留 */
/** 芯片总线控制 */
#define OBCR_ODS_16MA (1 << 6) /** 引脚驱动电流16mA */
#define OBCR_BCDS_1 (0 << 0)   /** 1分频 */
#define OBCR_BCDS_2 (1 << 0)   /** 2分频 */
#define OBCR_BCDS_3 (2 << 0)   /** 3分频 */
#define OBCR_BCDS_MSK (3 << 0) /** 分频设置掩码 */
/** EEPROM控制 */
#define EEPCR_EESA (1 << 4)  /** 允许软件存取EEPROM */
#define EEPCR_EESB (1 << 3)  /** EEPROM数据读出位，对应EED_IO  */
#define EEPCR_EEDO (1 << 2)  /** EEPROM数据写入位，对应EED_IO */
#define EEPCR_EESCK (1 << 1) /** EEPROM时钟，对应EESK */
#define EEPCR_EECS (1 << 0)  /** EEPROM操作选择位，对应EECS */
/** 芯片内存自测试状态 */
#define MBIR_TXMBF (1 << 12)  /** 置位，发送区测试完成 */
#define MBIR_TXMBFA (1 << 11) /** 置位，发送区测试失败 */
#define MBIR_RXMBF (1 << 4)   /** 置位，接收区测试完成 */
#define MBIR_RXMBFA (1 << 3)  /** 置位，接收区测试失败 */
/** 全局复位寄存器 */
#define GRR_QMU (1 << 1) /** 软件复位QMU */
#define GRR_GSR (1 << 0) /** 全局软件复位 */
/** 唤醒帧控制寄存器 */
#define WFCR_MPRXE (1 << 7) /** 魔幻包唤醒使能 */
#define WFCR_WF3E (1 << 3)  /** 唤醒帧3使能 */
#define WFCR_WF2E (1 << 2)  /** 唤醒帧2使能 */
#define WFCR_WF1E (1 << 1)  /** 唤醒帧1使能 */
#define WFCR_WF0E (1 << 0)  /** 唤醒帧0使能 */
/** 发送控制寄存器 */
#define TXCR_TCGICMP (1 << 8) /** 生成ICMP校验码 */
#define TXCR_TCGUDP (1 << 7)  /** 生成UDP校验码，手册中为保留 */
#define TXCR_TCGTCP (1 << 6)  /** 生成TCP校验码 */
#define TXCR_TCGIP (1 << 5)   /** 生成IP校验码 */
#define TXCR_FTXQ (1 << 4)    /** 清除发送队列缓冲区并复位发送指针 */
#define TXCR_TXFCE (1 << 3)   /** 使能发送流控 */
#define TXCR_TXPE (1 << 2)    /** 使能自动追加数据 */
#define TXCR_TXCRC (1 << 1)   /** 使能发送帧校验和计算 */
#define TXCR_TXE (1 << 0)     /** 使能发送 */
/** 发送状态寄存器 */
#define TXSR_TXLC (1 << 13)         /** 发送晚到冲突 */
#define TXSR_TXMC (1 << 12)         /** 发送冲突达到最大值 */
#define TXSR_TXFID_MASK (0x3f << 0) /** 帧ID掩码 */
#define TXSR_TXFID_SHIFT (0)        /** 帧ID移位数 */
#define TXSR_TXFID_GET(_v) (((_v) >> 0) & 0x3f)
/** 接收控制寄存器1 */
#define RXCR1_FRXQ (1 << 15)     /** 清除接收缓冲区并复位接收指针，须先禁能接收 */
#define RXCR1_RXUDPFCC (1 << 14) /** 使能UDP校验检查 */
#define RXCR1_RXTCPFCC (1 << 13) /** 使能TCP校验检查 */
#define RXCR1_RXIPFCC (1 << 12)  /** 使能IP校验检查 */
#define RXCR1_RXPAFMA (1 << 11)  /** 使能MAC地址过滤 */
#define RXCR1_RXFCE (1 << 10)    /** 使能接收流控 */
#define RXCR1_RXEFE (1 << 9)     /** 使能接收CRC错误帧 */
#define RXCR1_RXMAFMA (1 << 8)   /** 使能组播MAC地址过滤 */
#define RXCR1_RXBE (1 << 7)      /** 使能接收广播帧 */
#define RXCR1_RXME (1 << 6)      /** 使能接收组播帧 */
#define RXCR1_RXUE (1 << 5)      /** 使能接收单播帧 */
#define RXCR1_RXAE (1 << 4)      /** 使能接收所有帧 */
#define RXCR1_RXINVF (1 << 1)    /** 使能反转过滤 */
#define RXCR1_RXE (1 << 0)       /** 使能接收 */
/** 过滤掩码 */
#define RXCR1_FILTER_MASK (RXCR1_RXINVF | RXCR1_RXAE | RXCR1_RXMAFMA | RXCR1_RXPAFMA)
/** 接收控制寄存器2 */
#if 0                               /** 手册中无此定义 */
#define RXCR2_SRDBL_MASK (0x7 << 5) /**  */
#define RXCR2_SRDBL_SHIFT (5)       /**  */
#define RXCR2_SRDBL_4B (0x0 << 5)
#define RXCR2_SRDBL_8B (0x1 << 5)
#define RXCR2_SRDBL_16B (0x2 << 5)
#define RXCR2_SRDBL_32B (0x3 << 5)
#define RXCR2_SRDBL_FRAME (0x4 << 5)
#endif
#define RXCR2_IUFFP (1 << 4)     /** 使能IP分割帧校验接收 */
#define RXCR2_RXIUFCEZ (1 << 3)  /** 使能IP校验检查 */
#define RXCR2_UDPLFE (1 << 2)    /** 使能UDP校验检查 */
#define RXCR2_RXICMPFCC (1 << 1) /** 使能ICMP校验检查 */
#define RXCR2_RXSAF (1 << 0)     /** 使能源地址过滤 */
#define TXMIR_SIZE_MASK (0x1FFF) /** 可用发送缓冲区大小，最大6kB */
/** 接收帧头状态寄存器 */
#define RXFSHR_RXFV (1 << 15)      /** 接收帧有效标志 */
#define RXFSHR_RXICMPFCS (1 << 13) /** 接收的ICMP帧校验错 */
#define RXFSHR_RXIPFCS (1 << 12)   /** 接收的IP帧校验错 */
#define RXFSHR_RXTCPFCS (1 << 11)  /** 接收的TCP帧校验错 */
#define RXFSHR_RXUDPFCS (1 << 10)  /** 接收的UDP帧校验错 */
#define RXFSHR_RXBF (1 << 7)       /** 接收到广播帧 */
#define RXFSHR_RXMF (1 << 6)       /** 接收到组播帧 */
#define RXFSHR_RXUF (1 << 5)       /** 接收到单播帧 */
#define RXFSHR_RXMR (1 << 4)       /** 接收的帧存在MII标记错 */
#define RXFSHR_RXFT (1 << 3)       /** 置位表示接收的是以太帧，清位表示是802.3帧 */
#define RXFSHR_RXFTL (1 << 2)      /** 接收到超长帧 */
#define RXFSHR_RXRF (1 << 1)       /** 接收到过小帧 */
#define RXFSHR_RXCE (1 << 0)       /** 接收帧CRC错 */
/** 接收错误标志码 */
#define RXFSHR_ERR (RXFSHR_RXCE | RXFSHR_RXRF | RXFSHR_RXFTL | RXFSHR_RXMR | \
                    RXFSHR_RXICMPFCS | RXFSHR_RXIPFCS | RXFSHR_RXTCPFCS)
/** 发送命令寄存器 */
#define TXQCR_AETFE (1 << 2)  /** 置位启用发送自动队列 */
#define TXQCR_TXQMAM (1 << 1) /** 使能发送缓冲区可用监视 */
#define TXQCR_METFE (1 << 0)  /** 使能手动队列 */
/** 接收命令寄存器 */
#define RXQCR_RXDTTS (1 << 12)  /** 接收持续定时器阈值状态 */
#define RXQCR_RXDBCTS (1 << 11) /** 接收字节数阈值状态 */
#define RXQCR_RXFCTS (1 << 10)  /** 接收帧数阈值状态 */
#define RXQCR_RXIPHTOE (1 << 9) /** 使能IP报头两字节偏移功能 */
#define RXQCR_RXDTTE (1 << 7)   /** 使能持续接收定时功能 */
#define RXQCR_RXDBCTE (1 << 6)  /** 使能接收字节数阈值控制功能 */
#define RXQCR_RXFCTE (1 << 5)   /** 使能接收帧数阈值控制功能 */
#define RXQCR_ADRFE (1 << 4)    /** 使能接收报文自动出队列 */
#define RXQCR_SDA (1 << 3)      /** 使能DMA操作 */
#define RXQCR_RRXEF (1 << 0)    /** 释放错误帧 */
/** 使用帧数控制和报文自动队列 */
#define RXQCR_CMD_CNTL (RXQCR_RXFCTE | RXQCR_ADRFE)
/** 发送帧数据指针寄存器 */
#define TXFDPR_TXFPAI (1 << 14)       /** 发送帧数据指针自动增加 */
#define TXFDPR_TXFP_MASK (0x7ff << 0) /** 发送帧数据指针值掩码 */
#define TXFDPR_TXFP_SHIFT (0)         /** 移位数 */
/** 接收帧数据指针寄存器 */
#define RXFDPR_RXFPAI (1 << 14)       /** 接收帧数据指针自动增加 */
#define RXFDPR_WST (1 << 12)          /** 写采样时间，清位8～16ns，置位4ns */
#define RXFDPR_EMS (1 << 11)          /** 端模式设置，清位;小端，置位:大端 */
#define RXFDPR_RXFP_MASK (0x7ff << 0) /** 接收帧数据指针值掩码 */
#define RXFDPR_RXFP_SHIFT (0)         /** 移位数 */
/** 中断使能、状态寄存器 */
#define IRQ_LCI (1 << 15)   /** 链接状态改变中断标志，使能或中断有效 */
#define IRQ_TXI (1 << 14)   /** 发送中断 */
#define IRQ_RXI (1 << 13)   /** 接收中断 */
#define IRQ_RXOI (1 << 11)  /** 接收超载中断 */
#define IRQ_TXPSI (1 << 9)  /** 发送停止中断 */
#define IRQ_RXPSI (1 << 8)  /** 接收停止中断 */
#define IRQ_TXSAI (1 << 6)  /** 发送空间有效中断 */
#define IRQ_RXWFDI (1 << 5) /** 接收检测到唤醒帧 */
#define IRQ_RXMPDI (1 << 4) /** 接收检测到麻木魔术包 */
#define IRQ_LDI (1 << 3)    /** 建立链接中断 */
#define IRQ_EDI (1 << 2)    /** 当前信道检测到能量中断 */
#define IRQ_SPIBEI (1 << 1) /** 手册中未定义 */
#define IRQ_DEDI (1 << 0)   /** 延迟能量检测中断 */
/** 接收帧计数及阈值寄存器 */
#define RXFCTR_THRESHOLD_MASK (0x00FF) /** 帧计数阈值掩码 */
/** 芯片ID寄存器 */
#define CIDER_ID (0x8870) /** 芯片ID */
#define CIDER_REV_MASK (0x7 << 1)
#define CIDER_REV_SHIFT (1)
#define CIDER_REV_GET(_v) (((_v) >> 1) & 0x7)
/** 间接存取控制寄存器 */
#define IACR_RDEN (1 << 12)        /** 读使能 */
#define IACR_TSEL_MASK (0x3 << 10) /** 数据表选择掩码 */
#define IACR_TSEL_SHIFT (10)       /**  */
#define IACR_TSEL_MIB (0x3 << 10)
#define IACR_ADDR_MASK (0x1f << 0) /** MIB地址 */
#define IACR_ADDR_SHIFT (0)
/** 电源管理控制寄存器 */
#define PMECR_PME_DELAY (1 << 14)               /** PME引脚输出延时使能 */
#define PMECR_PME_POL (1 << 12)                 /** PME输出极性，置位:高有效 清位:低有效 */
#define PMECR_WOL_WAKEUP (1 << 11)              /** 使能唤醒帧影响PME引脚 */
#define PMECR_WOL_MAGICPKT (1 << 10)            /** 使能魔术帧影响PME引脚 */
#define PMECR_WOL_LINKUP (1 << 9)               /** 使能链接建立状态影响PME引脚 */
#define PMECR_WOL_ENERGY (1 << 8)               /** 使能信道能量检测影响PME引脚 */
#define PMECR_AUTO_WAKE_EN (1 << 7)             /** 使能自动唤醒 */
#define PMECR_WAKEUP_NORMAL (1 << 6)            /** 能量检测唤醒到正常模式 */
#define PMECR_WKEVT_MASK (0xf << 2)             /** 唤醒事件掩码 */
#define PMECR_WKEVT_SHIFT (2)                   /** 移位数 */
#define PMECR_WKEVT_GET(_v) (((_v) >> 2) & 0xf) /** 取唤醒事件 */
#define PMECR_WKEVT_ENERGY (0x1 << 2)           /** 信道能量检测唤醒事件 */
#define PMECR_WKEVT_LINK (0x2 << 2)             /** 链接建立唤醒事件 */
#define PMECR_WKEVT_MAGICPKT (0x4 << 2)         /** 魔术包唤醒事件 */
#define PMECR_WKEVT_FRAME (0x8 << 2)            /** 唤醒帧唤醒事件 */
#define PMECR_PM_MASK (0x3 << 0)                /** 电源管理模式掩码 */
#define PMECR_PM_SHIFT (0)                      /** 移位数 */
#define PMECR_PM_NORMAL (0x0 << 0)              /** 正常模式 */
#define PMECR_PM_ENERGY (0x1 << 0)              /** 能量检测模式 */
#define PMECR_PM_SOFTDOWN (0x2 << 0)            /** 软件掉电模式 */
#define PMECR_PM_POWERSAVE (0x3 << 0)           /** 节能模式 */
/** PHY MII寄存器基础控制寄存器 */
#define P1MBCR_LLB (1 << 14)        /** 本地loopback模式 */
#define P1MBCR_FORCE_100 (1 << 13)  /** 禁止自动协商时，强制100/10Mbps，缺省100Mbps */
#define P1MBCR_AN (1 << 12)         /** 使能自动协商 */
#define P1MBCR_RAN (1 << 9)         /** 重启自动协商 */
#define P1MBCR_FORCE_FDX (1 << 8)   /** 强制全双工 */
#define P1MBCR_HP_MDIX (1 << 5)     /** 自动MDIX */
#define P1MBCR_FORCE_MDIX (1 << 4)  /** 强制MDIX */
#define P1MBCR_DIS_HP_MDIX (1 << 3) /** 禁止自动MDIX */
#define P1MBCR_DIS_TX (1 << 1)      /** 禁止发送 */
#define P1MBCR_DIS_LED (1 << 0)     /** 禁止LED指示 */
/** PHY MII基础状态寄存器 */
#define P1MBSR_AN_COMPLETE (1 << 5) /** */
#define P1MBSR_AN_CAPABLE (1 << 3)
#define P1MBSR_LINK_UP (1 << 2)
#define PHY_ID_LR (0x1430)
#define PHY_ID_HI (0X0022)
/** 端口1控制寄存器 */
#define P1CR_DIS_LED (1 << 15)      /** 禁止LED指示 */
#define P1CR_TXIDS (1 << 14)        /** 禁止发送 */
#define P1CR_RAN (1 << 13)          /** 重启自动协商 */
#define P1CR_DIS_MDIX (1 << 10)     /** 禁止自动MDI/MDIX识别 */
#define P1CR_FORCE_MDIX (1 << 9)    /** 强制MDIX */
#define P1CR_ANE (1 << 7)           /** 使能自动协商 */
#define P1CR_FORCE_100BT (1 << 6)   /** 强制100Mbps */
#define P1CR_FORCE_FDX (1 << 5)     /** 强制全双工 */
#define P1CR_ADT_FLOW (1 << 4)      /** 宣告具有流控能力 */
#define P1CR_ADT_100BT_FDX (1 << 3) /** 宣告具有100M全双工能力 */
#define P1CR_ADT_100BT_HDX (1 << 2) /** 宣告具有100M半双工能力 */
#define P1CR_ADT_10BT_FDX (1 << 1)  /** 宣告具有10M全双工能力 */
#define P1CR_ADT_10BT_HDX (1 << 0)  /** 宣告具有10M半双工能力 */
/** 端口1状态寄存器 */
#define P1SR_HP_MDIX (1 << 15)       /** HP自动MDIX模式 */
#define P1SR_REV_POL (1 << 13)       /** 极性翻转 */
#define P1SR_OP_100M (1 << 10)       /** 链接速度100Mbps */
#define P1SR_OP_FDX (1 << 9)         /** 全双工模式 */
#define P1SR_OP_MDI (1 << 7)         /** 工作在MDI模式 */
#define P1SR_AN_DONE (1 << 6)        /** 自动协商完成 */
#define P1SR_LINK_GOOD (1 << 5)      /** 链接状态正常 */
#define P1SR_PNTR_FLOW (1 << 4)      /** 链接远端流控能力 */
#define P1SR_PNTR_100BT_FDX (1 << 3) /** 链接远端具备100M全双工能力 */
#define P1SR_PNTR_100BT_HDX (1 << 2) /** 链接远端具备100M半双工能力 */
#define P1SR_PNTR_10BT_FDX (1 << 1)  /** 链接远端具备10M全双工能力 */
#define P1SR_PNTR_10BT_HDX (1 << 0)  /** 链接远端具备10M半双工能力 */

#define BE3 0x8000 /* Byte Enable 3 */
#define BE2 0x4000 /* Byte Enable 2 */
#define BE1 0x2000 /* Byte Enable 1 */
#define BE0 0x1000 /* Byte Enable 0 */

/* 结构/枚举定义 ------------------------------- */
typedef enum
{
    ENUM_BUS_8BIT = 1,
    ENUM_BUS_16BIT = 2,
    ENUM_BUS_32BIT = 3
} KSZ_BUSWD_ENUM;

int ks_init(struct rt_fmc_eth_port *ps_ks);
int ks_readid(struct rt_fmc_eth_port *ps_ks);

int ks_start_xmit(struct rt_fmc_eth_port *ps_ks, struct pbuf *p);
int32_t ks_start_xmit_link_layer(struct rt_fmc_eth_port *ps_ks, KSZ_S_LEP_BUF *ps_lep_buf);
struct pbuf *ks_irq(struct rt_fmc_eth_port *ps_ks);

#ifdef __cplusplus
}
#endif

#endif /** KSZ8851_H_ */
