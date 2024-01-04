
def GetCPPPATH(BSP_ROOT, RTT_ROOT):
	CPPPATH=[
		BSP_ROOT + "/.",
		BSP_ROOT + "/applications",
		BSP_ROOT + "/applications/ly_05c1_test",
		BSP_ROOT + "/board",
		BSP_ROOT + "/board/CubeMX_Config/M4_CoreBoard_SDRAM/Inc",
		BSP_ROOT + "/board/ports",
		BSP_ROOT + "/board/ports/adau1761",
		BSP_ROOT + "/board/ports/ksz8851",
		BSP_ROOT + "/board/ports/usb_stm32",
		BSP_ROOT + "/libraries/HAL_Drivers",
		BSP_ROOT + "/libraries/HAL_Drivers/config",
		BSP_ROOT + "/libraries/HAL_Drivers/drv_flash",
		BSP_ROOT + "/libraries/STM32F4xx_HAL/CMSIS/Device/ST/STM32F4xx/Include",
		BSP_ROOT + "/libraries/STM32F4xx_HAL/STM32F4xx_HAL_Driver/Inc",
		BSP_ROOT + "/libraries/STM32_USB_Host_Library/Class/AUDIO/Inc",
		BSP_ROOT + "/libraries/STM32_USB_Host_Library/Class/CDC/Inc",
		BSP_ROOT + "/libraries/STM32_USB_Host_Library/Class/HID/Inc",
		BSP_ROOT + "/libraries/STM32_USB_Host_Library/Class/MSC/Inc",
		BSP_ROOT + "/libraries/STM32_USB_Host_Library/Class/MTP/Inc",
		BSP_ROOT + "/libraries/STM32_USB_Host_Library/Core/Inc",
		BSP_ROOT + "/packages/3gpp_amrnb",
		BSP_ROOT + "/packages/CMSIS_5/CMSIS/Core/Include",
		BSP_ROOT + "/packages/CMSIS_5/CMSIS/RTOS2/Include",
		BSP_ROOT + "/packages/CMSIS_5/CMSIS/RTOS2/Template",
		BSP_ROOT + "/packages/CMSIS_RTOS2",
		BSP_ROOT + "/packages/FlashDB/inc",
		BSP_ROOT + "/packages/FlashDB_port",
		BSP_ROOT + "/packages/crclib/inc",
		BSP_ROOT + "/packages/littlefs",
		BSP_ROOT + "/packages/netutils/tftp",
		BSP_ROOT + "/packages/optparse",
		BSP_ROOT + "/packages/ota_from_file",
		BSP_ROOT + "/packages/sysinfo",
		BSP_ROOT + "/packages/sysinfo/platform",
		BSP_ROOT + "/packages/sysinfo/platform/HamburgerOS",
		BSP_ROOT + "/packages/syswatch/inc",
		BSP_ROOT + "/packages/ulog_back",
		BSP_ROOT + "/packages/wavplayer",
		BSP_ROOT + "/packages/wavplayer/inc",
		RTT_ROOT + "/components/dfs/filesystems/devfs",
		RTT_ROOT + "/components/dfs/filesystems/elmfat",
		RTT_ROOT + "/components/dfs/filesystems/romfs",
		RTT_ROOT + "/components/dfs/filesystems/tmpfs",
		RTT_ROOT + "/components/dfs/include",
		RTT_ROOT + "/components/drivers/audio",
		RTT_ROOT + "/components/drivers/include",
		RTT_ROOT + "/components/drivers/sensors",
		RTT_ROOT + "/components/fal/inc",
		RTT_ROOT + "/components/finsh",
		RTT_ROOT + "/components/libc/compilers/common/include",
		RTT_ROOT + "/components/libc/compilers/newlib",
		RTT_ROOT + "/components/libc/posix/delay",
		RTT_ROOT + "/components/libc/posix/io/poll",
		RTT_ROOT + "/components/libc/posix/io/stdio",
		RTT_ROOT + "/components/libc/posix/ipc",
		RTT_ROOT + "/components/libc/posix/libdl",
		RTT_ROOT + "/components/libc/posix/pthreads",
		RTT_ROOT + "/components/net/lwip/lwip-2.1.2/src/include",
		RTT_ROOT + "/components/net/lwip/lwip-2.1.2/src/include/netif",
		RTT_ROOT + "/components/net/lwip/port",
		RTT_ROOT + "/components/net/netdev/include",
		RTT_ROOT + "/components/net/sal/impl",
		RTT_ROOT + "/components/net/sal/include",
		RTT_ROOT + "/components/net/sal/include/dfs_net",
		RTT_ROOT + "/components/net/sal/include/socket",
		RTT_ROOT + "/components/net/sal/include/socket/sys_socket",
		RTT_ROOT + "/components/utilities/ulog",
		RTT_ROOT + "/components/utilities/ulog/backend",
		RTT_ROOT + "/include",
		RTT_ROOT + "/libcpu/arm/common",
		RTT_ROOT + "/libcpu/arm/cortex-m4",
	]

	return CPPPATH

def GetCPPDEFINES():
	CPPDEFINES=['LFS_CONFIG=lfs_config.h', '_POSIX_C_SOURCE=1', 'ARM_MATH_CM4', 'RT_USING_LIBC', 'ARM_MATH_ROUNDING', 'ARM_MATH_MATRIX_CHECK', '__RTTHREAD__', 'RT_USING_NEWLIB', 'USE_HAL_DRIVER', 'STM32F429xx']
	return CPPDEFINES
