/* ==========================================================================
 *
 *  File      : MUE_APP.C
 *
 *  Purpose   : MVB UART Emulation - Application Example
 *
 *  Project   : Driver Software for MDFULL
 *              - MVB UART Emulation Protocol (d-000206-nnnnnn)
 *              - TCN Software Architecture   (d-000487-nnnnnn)
 *
 *  Version   : d-000543-nnnnnn
 *
 *  Licence   : Duagon Software Licence (see file 'licence.txt')
 *
 * --------------------------------------------------------------------------
 *
 *  (C) COPYRIGHT, Duagon AG, CH-8953 Dietikon, Switzerland
 *  All Rights Reserved.
 *
 * ==========================================================================*/
#include "tcn_def.h"
#include "mue_def.h"
#include "mue_osl.h"
#include "mue_acc.h"
#include "mue_sv.h"
#include "mue_pd_full.h"
#include "mue_cch.h"
#include "drv_mvb.h"
#include <string.h> /* memset, memcpy */

/*此处 增、删发送端口号 */
TCN_DECL_LOCAL TCN_DECL_CONST BITSET16 tx_port_adr[TX_PORT_ADR_NUB] = {TX_PORT_ADR1,TX_PORT_ADR2,TX_PORT_ADR3,
	                                                                    TX_PORT_ADR4,TX_PORT_ADR5,TX_PORT_ADR6};
/*此处 增、删接收端口号 */
TCN_DECL_LOCAL TCN_DECL_CONST BITSET16 rx_port_adr[TX_PORT_ADR_NUB] = {RX_PORT_ADR1,RX_PORT_ADR2,RX_PORT_ADR3,
                                                                       RX_PORT_ADR4,RX_PORT_ADR5,RX_PORT_ADR6};

 
/* --------------------------------------------------------------------------
 *  Procedure : mue_app_init_device
 * --------------------------------------------------------------------------
 */
TCN_DECL_LOCAL  MUE_RESULT  mue_app_init_device(void)
{
    MUE_RESULT  result;
    BITSET16    sv_device_config;
    /* ----------------------------------------------------------------------
     *  Initialise MVB UART Emulation
     * ----------------------------------------------------------------------
     */
    result = mue_acc_init();

    /* ----------------------------------------------------------------------
     *  Cleanup MVB UART Emulation
     * ----------------------------------------------------------------------
     */
    if (MUE_RESULT_OK == result)
    {
        result = mue_acc_clean_up();
    }

    /* ----------------------------------------------------------------------
     *  Soft reset of MVB Controller (e.g. disconnect from MVB bus)
		 *  Switch off line_A and line_B 'C' = 43
     * ----------------------------------------------------------------------
     */
    if (MUE_RESULT_OK == result)
    {
        sv_device_config = (BITSET16)0x0000;
        result = mue_sv_put_device_config(sv_device_config);
    }

    return(result);
}

/* --------------------------------------------------------------------------
 *  Procedure : mue_app_init_port
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_app_init_port(BITSET16  pd_port_config)
{
    MUE_RESULT  result;
    WORD16      pd_port_address;
    WORD8       pd_port_data[MUE_PD_FULL_PORT_SIZE_MAX];
    /* ----------------------------------------------------------
     *  NOTE:
     *  "pd_port_data" is a buffer of 32 bytes, which
     *  is the max. size of a MVB Process Data port.
     * ----------------------------------------------------------*/
    /* ----------------------------------------------------------------------
     *  Configure MVB Process Data Port 'H' 0x48 向网卡通信存储器中输入过程数据配置端口信息
     * ----------------------------------------------------------------------
     */
    result = mue_pd_full_put_port_config(pd_port_config);

    /* ----------------------------------------------------------------------
     *  Set all bytes of port data to 0x00
     * ----------------------------------------------------------------------
     */
    if (MUE_RESULT_OK == result)
    {
        pd_port_address = (WORD16)(pd_port_config & mue_pd_full_port_config_la_mask);
        memset((void*)pd_port_data, 0, mue_pd_full_port_size_max);
        result = mue_pd_full_put_port_data(pd_port_address, mue_pd_full_port_size_max, pd_port_data);
    } 
    return(result);

}
/* --------------------------------------------------------------------------
 *  Procedure : tx_port_config
 * 发送端口配置
 * --------------------------------------------------------------------------*/
TCN_DECL_LOCAL MUE_RESULT tx_port_config(void)
{
	   MUE_RESULT  result;
	   BITSET16    pd_port_config;
	   BITSET16    i = 0; 
    /* ----------------------------------------------------------------------
     *  Configure MVB Process Data Port:
     *  - port_address   = 0x550    -> 0x0550
     *  - port_size      = 32 bytes -> 0x4000
     *  - port_direction = SOURCE   -> 0x8000
     *                                 ======
     *                                 0xC550 -> pd_port_config
     * ----------------------------------------------------------------------
     *  NOTE: See also parameter constants located in file "mue_pd_full.h",
     *        e.g. "mue_pd_full_port_config_...".
     * ----------------------------------------------------------------------
     */
     for	(i = 0 ; i < TX_PORT_ADR_NUB ; i++)
	   {
        pd_port_config = (BITSET16)(0xC000 | tx_port_adr[i]);
        result = mue_app_init_port(pd_port_config);
			  if (MUE_RESULT_OK != result)
			  {
            break;
			  } 
	   } 
		 return(result);
}

/* --------------------------------------------------------------------------
 *  Procedure : tx_port_config
 * 接收端口配置
 * --------------------------------------------------------------------------*/
TCN_DECL_LOCAL MUE_RESULT rx_port_config(void)
{
	   MUE_RESULT  result;
	   BITSET16    pd_port_config;
	   BITSET16    i = 0; 
	    /* ----------------------------------------------------------------------
     *  Configure MVB Process Data Port:
     *  - port_address   = 0x020    -> 0x0020
     *  - port_size      = 32 bytes -> 0x4000
     *  - port_direction = SINK     -> 0x0000
     *                                 ======
     *                                 0x4020 -> pd_port_config
     * ----------------------------------------------------------------------
     *  NOTE: See also parameter constants located in file "mue_pd_full.h",
     *        e.g. "mue_pd_full_port_config_...".
     * ----------------------------------------------------------------------
     */
     for	( i = 0 ; i < RX_PORT_ADR_NUB ; i++)
	   {
			  pd_port_config = (BITSET16)(0x4000 | rx_port_adr[i]);
			  result = mue_app_init_port(pd_port_config);
			  if (MUE_RESULT_OK != result)
			  {
            break;
			  } 
	   }
	   return result;
}
/* --------------------------------------------------------------------------
 *  Procedure : mue_app_main

 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_app_main (void)
{
    MUE_RESULT  result;
    BITSET16    sv_device_config;
    WORD16      pd_port_address;
    BITSET8     pd_port_status;
    UNSIGNED16  pd_port_freshness;
	  BITSET16    i = 0; 
    WORD8       pd_port_data_get[MUE_PD_FULL_PORT_SIZE_MAX];
    WORD8       pd_port_data_put[MUE_PD_FULL_PORT_SIZE_MAX];
		/* ----------------------------------------------------------
		 *  NOTE:
		 *  "pd_port_data_xxx" is a buffer of 32 bytes, which
		 *  is the max. size of a MVB Process Data port.
		 *  For any port communication always use a buffer
		 *  of this max. size!
		 * ----------------------------------------------------------
		 */
#if     FMC_SRAM_MODE	
	    MVB_ADR_init();
#endif
    /* ----------------------------------------------------------------------
     *  Initialise MVB Device
     * ----------------------------------------------------------------------
     */
    result = mue_app_init_device();
    /* ----------------------------------------------------------------------
     *  Start MVB communication:
     *  - device_address = 0x010 -> 0x0010
     *  - enable the status LED  -> 0x1000;
     *  - enable MVB line A      -> 0x8000
     *  - enable MVB line B      -> 0x4000
     *  - disable BA             -> 0x0000
     *  - clear bit DNR of DSW   -> 0x1000
     *                              ======
     *                              0xD010 -> sv_device_config
     * ----------------------------------------------------------------------
     *  NOTE: See also parameter constants located in file "mue_sv.h",
     *        e.g. "mue_sv_device_config_...".
     * ----------------------------------------------------------------------
     */
    if (MUE_RESULT_OK == result)
    {
        sv_device_config = (BITSET16)(0xD000 | MVB_DEV_ADR);
        result = mue_sv_put_device_config(sv_device_config);
    }

		sv_device_config = 0x00;
		mue_sv_get_device_config(&sv_device_config);
		if((BITSET16)(0xD000 | MVB_DEV_ADR) != sv_device_config)
		{
			 sv_device_config = 0xF101;
		}
		/********************输入总线管理器配置  41 A0 10  回 无  仅主站设备使用************************/
		 /*if (MUE_RESULT_OK == result)
     {
        sv_device_config = (BITSET16)0xA010;
			  result = mue_sv_put_ba_config(sv_device_config);//输入总线管理器配置  41 A0 10
     }
		 //获取设备状态 44 00 10
		 mue_sv_get_device_status(0x0010,  (BOOLEAN1)FALSE,&pd_port_status,&pd_port_device_status,&pd_port_freshness);*/ 
		/***********************************************************************/
    /* ----------------------------------------------------------------------
     *  NOTE: See also parameter constants located in file "mue_pd_full.h",
     *        e.g. "mue_pd_full_port_config_...".
     * ----------------------------------------------------------------------
     */ 
		if (MUE_RESULT_OK == result)
		{
				 result = rx_port_config();
		}
    /* ----------------------------------------------------------------------
     *  NOTE: See also parameter constants located in file "mue_pd_full.h",
     *        e.g. "mue_pd_full_port_config_...".
     * ----------------------------------------------------------------------
     */
			if (MUE_RESULT_OK == result)
			{
				 result = tx_port_config();
			} 
    /* ----------------------------------------------------------------------
     *  Get data from MVB Process Data Port 
     * ----------------------------------------------------------------------
     */
     for	( i = 0 ; i < RX_PORT_ADR_NUB ; i++)
	   {
        pd_port_address = (WORD16)rx_port_adr[i];
        result =  mue_pd_full_get_port_data(pd_port_address, (BOOLEAN1)FALSE, mue_pd_full_port_size_max, 
                                            &pd_port_status, &pd_port_data_get[0], &pd_port_freshness);
			  if (MUE_RESULT_OK != result)
			  {
					  result = MUE_RESULT_ERROR;
            break;
			  } 
	   }		
    /* ----------------------------------------------------------------------
     *  Data processing:
     *  - handle  data received from MVB (i.e. from port )
     *  - prepare data to be send to MVB (i.e. to   port )
     * ----------------------------------------------------------------------
     *  This application example:
     *  - copy byte 1; during copy increment byte
     *  - copy byte 2; during copy invert    byte
     *  - copy bytes 3-31
     * ----------------------------------------------------------------------
     */
    memcpy((void*)pd_port_data_put, (void*)pd_port_data_get, mue_pd_full_port_size_max);
    pd_port_data_put[0] = (WORD8)(pd_port_data_get[0] + 1);
    pd_port_data_put[1] = (WORD8)(~pd_port_data_get[1]);

    /* ----------------------------------------------------------------------
     *  Put data to MVB Process Data Port 
     * ----------------------------------------------------------------------
     */
		 for	( i = 0 ; i < TX_PORT_ADR_NUB ; i++)
	   {
			  pd_port_address = (WORD16)tx_port_adr[i];
        result =  mue_pd_full_put_port_data(pd_port_address, mue_pd_full_port_size_max, &pd_port_data_put[0]);
			  if (MUE_RESULT_OK != result)
			  {
					  result = MUE_RESULT_ERROR;
            break;
			  } 
	   }
//    /* ----------------------------------------------------------------------
//     *  Stop MVB communication
//     * ----------------------------------------------------------------------
//     */
//    if (MUE_RESULT_OK == result)
//    {
//        sv_device_config = (BITSET16)0x0000;
//        result = mue_sv_put_device_config(sv_device_config);
//    } 
    return(result);
} 

/******************************发送 接口************************************/
MUE_RESULT  MVB_tx_data(unsigned short tx_port,unsigned char *pbuf , unsigned int len)
{
	  MUE_RESULT  result;
	  unsigned int i = 0U,pakgs = 0 , last_tail = 0;
	  unsigned char  tx_buf[MUE_PD_FULL_PORT_SIZE_MAX];

//		clear_MyDCache();
	
	  pakgs = len/MUE_PD_FULL_PORT_SIZE_MAX;
	
	  for(i = 0 ; i < pakgs; i++)
	  {
			 memset(tx_buf , 0x00 , sizeof(tx_buf));
			 memcpy(tx_buf , &pbuf[i*MUE_PD_FULL_PORT_SIZE_MAX] , MUE_PD_FULL_PORT_SIZE_MAX);
			 result = mue_pd_full_put_port_data(tx_port , mue_pd_full_port_size_max , tx_buf); 
			 //如果是多包  此处需要灵活加时间间隔
       //HAL_Delay(150);
		}
		
		last_tail = len%MUE_PD_FULL_PORT_SIZE_MAX;
		if(last_tail != 0)
		{
			 memset(tx_buf , 0x00 , sizeof(tx_buf));
			 memcpy(tx_buf , &pbuf[i*MUE_PD_FULL_PORT_SIZE_MAX] , last_tail);
			 result = mue_pd_full_put_port_data(tx_port , mue_pd_full_port_size_max , tx_buf);
		}
		return result;
}

/*一包32个字节  包与包之间需要时间间隔  ，根据实际需要调整 如果单包不需要*/
//测试 接收
void test_Tx(unsigned short port_adr)
{
	 unsigned char   pd_port_data_get[32] ,i = 0;
	 static unsigned char t ;
	 t++;
	 for(i = 0 ; i < sizeof(pd_port_data_get) ;i++)
	    pd_port_data_get[i] = t;
	
	 MVB_tx_data(port_adr,pd_port_data_get ,  sizeof(pd_port_data_get));
}
/******************************发送 接口  END************************************/
/******************************接收 接口 ************************************/
MUE_RESULT MVB_Rx_data(unsigned short port_adr,unsigned char *pbuf)
{
  unsigned char   result;
  unsigned char   pd_port_status = 0;
  unsigned short  pd_port_freshness = 0xFFFF;
  unsigned char   pd_port_data_get[MUE_PD_FULL_PORT_SIZE_MAX];
	
	memset(pd_port_data_get,0xFF,sizeof(pd_port_data_get));
	result =  mue_pd_full_get_port_data(port_adr, (BOOLEAN1)FALSE, mue_pd_full_port_size_max, 
																									&pd_port_status, &pd_port_data_get[0], &pd_port_freshness);  
		
  if(result == MUE_RESULT_OK)
	{
		memcpy(pbuf ,pd_port_data_get , MUE_PD_FULL_PORT_SIZE_MAX);
		return MUE_RESULT_OK;
		
	} 
	else
	{
		return MUE_RESULT_RECEIVE_TIMEOUT;
	}

}
//测试 接收
extern void z85230_send(uint8_t *buf,uint32_t len);
void test_Rx(unsigned short port_adr)
{
	 unsigned char  pd_port_data_get[MUE_PD_FULL_PORT_SIZE_MAX],i;
	 static unsigned char t = 0;
	 if(MVB_Rx_data(port_adr,pd_port_data_get) == MUE_RESULT_OK)
	 {
     z85230_send(pd_port_data_get,MUE_PD_FULL_PORT_SIZE_MAX);
	 }
}

/******************************接收 接口 END************************************/



/************************调试 测试接口 发送********************************************/
/*
unsigned  int circle = 0;
void MVBManager_TransmitDataTest(void)
{
	  unsigned char ucIndex;
    unsigned char   pd_port_data_get[MUE_PD_FULL_PORT_SIZE_MAX];
	  memset(pd_port_data_get,0xFF,sizeof(pd_port_data_get));
	
    circle++;
	
		for(ucIndex=0x00; ucIndex < mue_pd_full_port_size_max; ucIndex++)
		{
		 if(circle%2 == 0)
				 pd_port_data_get[ucIndex] = ucIndex;
		 else
				 pd_port_data_get[ucIndex] = ucIndex+10;
		 }	
		  
     mue_pd_full_put_port_data(TX_PORT_ADR1, mue_pd_full_port_size_max, &pd_port_data_get[0]);   		 
}

void MVBManager_RXDataTest(void)
{
  unsigned char   result;
  unsigned char   pd_port_status = 0;
  unsigned short  pd_port_freshness = 0xFFFF;
  unsigned char   pd_port_data_get[MUE_PD_FULL_PORT_SIZE_MAX];
	
	memset(pd_port_data_get,0xFF,sizeof(pd_port_data_get));
	result =  mue_pd_full_get_port_data(RX_PORT_ADR1, (BOOLEAN1)FALSE, mue_pd_full_port_size_max, 
																									&pd_port_status, &pd_port_data_get[0], &pd_port_freshness);  
		
	if(result == MUE_RESULT_OK )
  {
			pd_port_status = 0;
	}
}
*/
/************************测试接口 发送 end*******************************************/
