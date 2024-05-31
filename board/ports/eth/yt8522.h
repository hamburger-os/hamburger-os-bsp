#ifndef __JL5104_H
#define __JL5104_H

typedef enum
{
    YT8522_10M_HALF,
    YT8522_10M_FUL,
    YT8522_100M_HALF,
    YT8522_100M_FUL,
    DIS_CONNECT,
}YT8522_LINKSTA;

int32_t YT8522_Init(void);
uint32_t YT8522_GetLinkState(void);

#endif
