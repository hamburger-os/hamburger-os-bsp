#ifndef TRDP_CONFIG_H_
#define TRDP_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

#include <vos_types.h>

#ifdef __PLATFORM_SWOS2__

#define DEFAULT_ETH_NAME ("e0")

#define __bswap_8(x) ((x) & 0xff)
#define __bswap_16(x) ((__bswap_8(x) << 8) | __bswap_8((x) >> 8))
#define __bswap_32(x) ((__bswap_16(x) << 16) | __bswap_16((x) >> 16))
#define __bswap_64(x) ((__bswap_32(x) << 32) | __bswap_32((x) >> 32))

#else

#endif


#ifdef __cplusplus
}
#endif

#endif /* TRDP_CONFIG_H_ */
