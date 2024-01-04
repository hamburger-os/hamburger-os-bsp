# HamburgerOS bsp 说明

## 简介

![logo](figures/logo.png)

本文档为 HamburgerOS bsp (board support package 板级支持包) 说明。

主要内容如下：

- 快速上手
- 进阶使用方法

通过阅读快速上手章节开发者可以快速地上手该 bsp，将 HamburgerOS (基于RT-Thread实时内核) 运行在硬件上。在进阶使用指南章节，将会介绍更多高级功能，帮助开发者驱动更多板载资源。

## 系统支持

本 BSP 目前驱动及软件的支持情况如下：

| **板载外设**      | **支持情况** | **备注**                              |
| :----------------- | :----------: | :------------------------------------- |
| 以太网            |     支持     |  支持 KSZ8851-16MLLU 双网口                                 |
| SRAM             |     支持     |                                       |
| SDRAM             |     支持     |                                       |
| CAN               |   支持   |                                        |
| RS485               |   支持   |                                  |
| NOR               |   支持   |                                 |
| EMMC               |   支持   |                                 |
| FRAM               |   支持   |  支持 FM25xx 系列芯片  |
| SPIFLASH               |   支持   |  支持 sfud 系列芯片  |
| EEPROM               |   支持   |                                 |
| **片上外设**      | **支持情况** | **备注**                              |
| GPIO              |     支持     |                                     |
| UART              |     支持     |                               |
| SPI               |     支持     | 软件 SPI 或 硬件 SPI                     |
| I2C               |     支持     | 软件 I2C 或 硬件 I2C                            |
| CAN               |     支持     |                                    |
| RTC               |     支持     | 支持外部晶振和内部低速时钟 |
| WDT               |     支持     |                                       |
| FLASH | 支持 | 已适配 FAL (Flash Abstraction Layer) Flash 抽象层 |
| USB Device        |   不支持   |                               |
| USB Host          |   支持   | 支持挂载FAT16、FAT32、exFAT文件系统的u盘设备 |
| **第三方库**      | **支持情况** | **备注**                              |
| syswatch        |   支持   |                               |
| FS | 支持 | 已适配 DFS（Device File System） 设备虚拟文件系统 |
| CMSIS_5        |   支持   |                               |
| crclib        |   支持   |                               |
| ota_downloader        |   支持   |   支持bootloader   |
| ota_fram_file        |   支持   |  支持通过文件升级程序  |
| CherryUSB        |   支持   |                               |
| STM32USB        |   支持   |                               |
| qboot        |   支持   |  支持bootloader引导程序  |
| littlefs        |   支持   |  支持均衡写入和掉电保护的文件系统  |
| netutils        |   支持   |                               |
| FlashDB        |   支持   |   支持键值对数据库   |
| helix        |   支持   |   支持mp3文件解码   |
| vi        |   支持   |   支持文本文件编辑   |
| ulog_back        |   支持   |  支持将log输出至日志文件     ||

## 使用说明

使用说明分为如下两个章节：

- 快速上手

    本章节是为刚接触 RT-Thread 的新手准备的使用说明，遵循简单的步骤即可将 RT-Thread 操作系统运行在该开发板上，看到实验效果 。

- 进阶使用

    本章节是为需要在 RT-Thread 操作系统上使用更多开发板资源的开发者准备的。通过使用 ENV 工具对 BSP 进行配置，可以开启更多板载资源，实现更多高级功能。


### 快速上手

本 BSP 为开发者提供RT-Thread Studio、MDK4、MDK5 和 IAR 工程，并且支持 GCC 开发环境。

下面以 RT-Thread Studio 开发环境为例，介绍如何将系统运行起来。

#### 硬件连接

使用数据线连接开发板到 PC，打开电源开关。

#### 编译下载

点击 文件->导入->RT-Thread Studio项目到工作空间中，点击RT-Thread Setting对bsp进行裁剪，点击编译并下载程序到开发板。

> 工程默认配置使用 STlink 下载程序，在通过 STlink 连接开发板的基础上，点击下载按钮即可下载程序到开发板

#### 运行结果

下载程序成功之后，系统会自动运行，观察开发板上 LED 的运行效果，黄色 LED 会周期性闪烁。

连接开发板对应串口到 PC , 在终端工具里打开相应的串口（115200-8-1-N），复位设备后，可以看到 RT-Thread 的输出信息:

```bash
'   _    _                 _                                ____   _____  
'  | |  | |               | |                              # __ # # ____| 
'  | |__| | __ _ _ __ ___ | |__  _   _ _ __ __ _  ___ _ __| |  | | (___   
'  |  __  |# _` | '_ ` _ #| '_ #| | | | '__# _` |# _ # '__| |  | |#___ #  
'  | |  | | (_| | | | | | | |_) | |_| | | | (_| |  __# |  | |__| |____) | 
'  |_|  |_|#__,_|_| |_| |_|_.__# #__,_|_|  #__, |#___|_|   #____#|_____#  
'                                           __# |                         
'                                          |___#                          
'                                                  Thread Operating System
'                                         4.1.1 build Jan  4 2024 23:03:10
'                                                     Powered by RT-Thread
'                                                    2006 - 2023 Copyright
[1] I/syswatch: create syswatch thread success.
[7] I/syswatch: system watch startup successfully
total    : 13336
used     : 2276
maximum  : 2276
available: 11060
[20] I/drv.ram: init succeed 13 KB(13336).
|------------------------------
| * System Clock information * 
|    name    |  frequency
|------------------------------
|    SYSCLK  |  72000000
|    HCLK    |  72000000
|    PCLK1   |  36000000
|    PCLK2   |  72000000
|------------------------------
Type 'help' to get the list of commands.
Use UP/DOWN arrows to navigate through command history.
Press TAB when typing command name to auto-complete.
msh >
```
### 进阶使用

此 BSP 默认只开启了 LED 和 调试串口 的功能，如果需使用 SRAM、Flash 等更多高级功能，需要利用 ENV 工具对BSP 进行配置，步骤如下：

1. 在 bsp 下打开 env 工具。

2. 输入`menuconfig`命令配置工程，配置好之后保存退出。

3. 输入`pkgs --update`命令更新软件包。

4. 输入`scons --target=mdk4/mdk5/iar` 命令重新生成工程。

## 注意事项

暂无
