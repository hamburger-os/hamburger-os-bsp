#include <stdio.h>
#include <string.h>
#include "../../TinyFrame.h"
#include "../utils.h"

/**
 * 发送接收示例
 */
TinyFrame *demo_tf;

bool do_corrupt = false;   

/**
 * 此功能应在应用程序代码中定义。
 * 它实现了最低层 - 发送字节到UART（或其他）
 */
void TF_WriteImpl(TinyFrame *tf, const uint8_t *buff, uint32_t len)
{
    printf("--------------------\n");
    printf("\033[32mTF_WriteImpl - sending frame:\033[0m\n");
    
    uint8_t *xbuff = (uint8_t *)buff;    
    if (do_corrupt) {
      printf("(corrupting to test checksum checking...)\n");
      xbuff[8]++;
    }
    
    dumpFrame(xbuff, len);

    // 将它发回，就好像我们收到了它
    TF_Accept(tf, xbuff, len);
}

/** 一个监听器函数的示例 */
TF_Result myListener(TinyFrame *tf, TF_Msg *msg)
{
    dumpFrameInfo(msg);
    return TF_STAY;
}

TF_Result testIdListener(TinyFrame *tf, TF_Msg *msg)
{
    printf("OK - ID Listener triggered for msg!\n");
    dumpFrameInfo(msg);
    return TF_CLOSE;
}

void main(void)
{
    TF_Msg msg;
    const char *longstr = "Lorem ipsum dolor sit amet.";

    //设置TinyFrame库
    demo_tf = TF_Init(TF_MASTER); // 1 = master, 0 = slave
    // 添加lintener
    TF_AddGenericListener(demo_tf, myListener);

    printf("------ Simulate sending a message --------\n");

    TF_ClearMsg(&msg);
    msg.type = 0x22;
    msg.data = (pu8) "Hello TinyFrame";
    msg.len = 16;
    TF_Send(demo_tf, &msg);

    msg.type = 0x33;
    msg.data = (pu8) longstr;
    msg.len = (TF_LEN) (strlen(longstr) + 1); // 添加null类型
    TF_Send(demo_tf, &msg);

    msg.type = 0x44;
    msg.data = (pu8) "Hello2";
    msg.len = 7;
    TF_Send(demo_tf, &msg);

    msg.len = 0;
    msg.type = 0x77;
    TF_Query(demo_tf, &msg, testIdListener, 0);
    
    printf("This should fail:\n");
    
    // 测试校验和
    do_corrupt = true;    
    msg.type = 0x44;
    msg.data = (pu8) "Hello2";
    msg.len = 7;
    TF_Send(demo_tf, &msg);
}
