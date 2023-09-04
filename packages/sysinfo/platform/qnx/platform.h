/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     lvhan       the first version
 */
#ifndef PACKAGES_SYSINFO_INCLUDE_QNX_PLATFORM_H_
#define PACKAGES_SYSINFO_INCLUDE_QNX_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define __PLATFORM_QNX__

#define pPrintf printf
#define pLOG_HEX(TAG, width, buf, size) \
    pPrintf("HEX");                     \
    pPrintf("%s: ", TAG);               \
    uint8_t *buf_8 = (uint8_t *)buf; \
    for (int hex_i = 0; hex_i < size; hex_i++) { \
        if (hex_i != 0 && (hex_i) % width == 0) { \
            pPrintf("                        "); \
        } \
        if ((hex_i) % width == 0) { \
            pPrintf("%04X-%04X: ", hex_i, hex_i + width - 1); \
        } \
        pPrintf("%02X ", (uint8_t)buf_8[hex_i]); \
        if ((hex_i + 1) % width == 0) { \
            pPrintf("  "); \
            for (int hex_n = 0; hex_n < width; hex_n++) { \
                char hex_char = (char)buf_8[hex_i + 1 - width + hex_n]; \
                if (hex_char >= 0x20 && hex_char <= 0x7f) { \
                    pPrintf("%c", hex_char); \
                } \
                else { \
                    pPrintf("."); \
                } \
            } \
            pPrintf("\n"); \
        } \
    }
#define pLOG_D(...)              \
    pPrintf("\033[37mDEBUG : "); \
    pPrintf(__VA_ARGS__);        \
    pPrintf("\n\033[0m")
#define pLOG_I(...)              \
    pPrintf("\033[32mINFO  : "); \
    pPrintf(__VA_ARGS__);        \
    pPrintf("\n\033[0m")
#define pLOG_W(...)              \
    pPrintf("\033[33mWARN  : "); \
    pPrintf(__VA_ARGS__);        \
    pPrintf("\n\033[0m")
#define pLOG_E(...)              \
    pPrintf("\033[31mERROR : "); \
    pPrintf(__VA_ARGS__);        \
    pPrintf("\n\033[0m")

#ifdef __cplusplus
}
#endif

#endif /* PACKAGES_SYSINFO_INCLUDE_QNX_PLATFORM_H_ */
