/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : Common.c
**@author: Created By Chengt
**@date  : 2015.10.19
**@brief : the function for the project
********************************************************************************************/
#include "Common.h"


//uint8_t Version[4]={1,0,1,1};//版本号
//uint16_t Verdate=(0x16<<9)+(0x5<<5)+0x16;//版本日期

/* 版本号 */
/* ------------------------------------------------------------------------------------------
   |      Author      |          Date          |      Version      |          Note          |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | Unknown                | V1.0.0.5          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 24-November-2017       | V1.0.0.8          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 30-November-2017       | V1.0.1.1          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 21-December-2017       | V1.0.1.2          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 22-December-2017       | V1.0.1.3          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 23-December-2017       | V1.0.1.4          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 24-December-2017       | V1.0.1.7          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 27-December-2017       | V1.0.1.2          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 31-January-2018        | V1.0.1.3          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-March-2018          | V1.0.1.6          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 23-March-2018          | V1.0.1.7          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-April-2018          | V1.0.1.8          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 25-April-2018          | V1.0.1.9          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 27-April-2018          | V1.0.1.10         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 3-May-2018             | V1.0.1.11         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 10-May-2018            | V1.0.1.12         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 11-May-2018            | V1.0.1.12         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-May-2018            | V1.0.1.12         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-May-2018            | V1.0.1.12         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 21-May-2018            | V1.0.1.12         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 24-May-2018            | V1.0.1.13         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 24-May-2018            | V1.0.1.14         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 11-June-2018           | V1.0.1.15         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-June-2018           | V1.0.1.15         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 13-June-2018           | V1.0.1.15         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 14-June-2018           | V1.0.1.15         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 2-July-2018            | V1.0.1.16         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 6-July-2018            | V1.0.1.16         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 16-July-2018           | V1.0.1.16         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 16-August-2018         | V1.0.1.17         | Used for application.  |
	 ------------------------------------------------------------------------------------------
   | Chen Guo Tao     | 16-August-2018         | V1.0.1.18         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-August-2018         | V1.0.1.19         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 27-August-2018         | V1.0.1.20         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 5-September-2018       | V1.0.1.21         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 10-September-2018      | V1.0.1.22         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 10-September-2018      | V1.0.1.23         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 18-September-2018      | V1.0.1.24         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 19-September-2018      | V1.0.1.25         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 26-September-2018      | V1.0.1.26         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 28-September-2018      | V1.0.1.27         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 9-November-2018        | V1.0.1.28         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 12-November-2018       | V1.0.1.29         | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 28-November-2018       | V1.0.2.1          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 29-November-2018       | V1.0.2.2          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-December-2018       | V1.0.2.3          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 21-December-2018       | V1.0.2.4          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 14-January-2019        | V1.0.2.5          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 17-January-2019        | V1.0.2.6          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 22-January-2019        | V1.0.2.7          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 18-February-2019       | V1.0.2.8          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | Liang Zhen       | 19-April-2019          | V1.0.2.9          | Used for application.  |
   ------------------------------------------------------------------------------------------
   | hzj and Lunzm    |10-January-2020         |V1.0.2.10          |zengjiazhukong6Atouchuan|
   ------------------------------------------------------------------------------------------
*/

uint8_t Version[4] = { 0x01U, 0x00U, 0x03U, 0x1EU };

/* 版本日期 */
uint16_t Verdate = ( FIRMWARE_YEAR << 9u ) + ( FIRMWARE_MONTH << 5u ) + FIRMWARE_DAY;

uint8_t ID = 0x00u;

/* 判断CAN超时时间 */
/* 自检变量 */
uint32_t CAN0_time   = 0u, CAN1_time   = 0u, MCPCAN0_time = 0u, MCPCAN1_time = 0u;

uint32_t I_CAN0_time = 0u, I_CAN1_time = 0u, E_CAN0_time  = 0u, E_CAN1_time  = 0u;
uint32_t ZK1ZJ_time  = 0u, ZK2ZJ_time  = 0u, JCTXZJ_time  = 0u, JCJKZJ_time  = 0u;
uint32_t JLZJ_time   = 0u, XSQ1ZJ_time = 0u, XSQ2ZJ_time  = 0u, SK1ZJ_time   = 0u;
uint32_t SK2ZJ_time  = 0u, WJJKZJ_time = 0u, SKZJ_time    = 0u, XSQZJ_time   = 0u;
uint32_t SYSHEART_time = 0u;

CAN_FRAME SelfCheck_txMailbox = { 0u };

uint16_t ERROR_FLAG = 0u;
//TODO(mingzhao)
//sArrayList *p_can0_recv_list = NULL, *p_can1_recv_list = NULL;
//
//sArrayList *i_can0_recv_list = NULL, *i_can1_recv_list = NULL;

#if 0
/*******************************************************************************************
 ** @brief: 
 ** @param:
 *******************************************************************************************/
static const uint16_t crctab16[] =\
{
  0x0000U, 0x1189U, 0x2312U, 0x329bU, 0x4624U, 0x57adU, 0x6536U, 0x74bfU,
	0x8c48U, 0x9dc1U, 0xaf5aU, 0xbed3U, 0xca6cU, 0xdbe5U, 0xe97eU, 0xf8f7U,
	0x1081U, 0x0108U, 0x3393U, 0x221aU, 0x56a5U, 0x472cU, 0x75b7U, 0x643eU,
	0x9cc9U, 0x8d40U, 0xbfdbU, 0xae52U, 0xdaedU, 0xcb64U, 0xf9ffU, 0xe876U,
	0x2102U, 0x308bU, 0x0210U, 0x1399U, 0x6726U, 0x76afU, 0x4434U, 0x55bdU,
	0xad4aU, 0xbcc3U, 0x8e58U, 0x9fd1U, 0xeb6eU, 0xfae7U, 0xc87cU, 0xd9f5U,
	0x3183U, 0x200aU, 0x1291U, 0x0318U, 0x77a7U, 0x662eU, 0x54b5U, 0x453cU,
	0xbdcbU, 0xac42U, 0x9ed9U, 0x8f50U, 0xfbefU, 0xea66U, 0xd8fdU, 0xc974U,
	0x4204U, 0x538dU, 0x6116U, 0x709fU, 0x0420U, 0x15a9U, 0x2732U, 0x36bbU,
	0xce4cU, 0xdfc5U, 0xed5eU, 0xfcd7U, 0x8868U, 0x99e1U, 0xab7aU, 0xbaf3U,
	0x5285U, 0x430cU, 0x7197U, 0x601eU, 0x14a1U, 0x0528U, 0x37b3U, 0x263aU,
	0xdecdU, 0xcf44U, 0xfddfU, 0xec56U, 0x98e9U, 0x8960U, 0xbbfbU, 0xaa72U,
	0x6306U, 0x728fU, 0x4014U, 0x519dU, 0x2522U, 0x34abU, 0x0630U, 0x17b9U,
	0xef4eU, 0xfec7U, 0xcc5cU, 0xddd5U, 0xa96aU, 0xb8e3U, 0x8a78U, 0x9bf1U,
	0x7387U, 0x620eU, 0x5095U, 0x411cU, 0x35a3U, 0x242aU, 0x16b1U, 0x0738U,
	0xffcfU, 0xee46U, 0xdcddU, 0xcd54U, 0xb9ebU, 0xa862U, 0x9af9U, 0x8b70U,
	0x8408U, 0x9581U, 0xa71aU, 0xb693U, 0xc22cU, 0xd3a5U, 0xe13eU, 0xf0b7U,
	0x0840U, 0x19c9U, 0x2b52U, 0x3adbU, 0x4e64U, 0x5fedU, 0x6d76U, 0x7cffU,
	0x9489U, 0x8500U, 0xb79bU, 0xa612U, 0xd2adU, 0xc324U, 0xf1bfU, 0xe036U,
	0x18c1U, 0x0948U, 0x3bd3U, 0x2a5aU, 0x5ee5U, 0x4f6cU, 0x7df7U, 0x6c7eU,
	0xa50aU, 0xb483U, 0x8618U, 0x9791U, 0xe32eU, 0xf2a7U, 0xc03cU, 0xd1b5U,
	0x2942U, 0x38cbU, 0x0a50U, 0x1bd9U, 0x6f66U, 0x7eefU, 0x4c74U, 0x5dfdU,
	0xb58bU, 0xa402U, 0x9699U, 0x8710U, 0xf3afU, 0xe226U, 0xd0bdU, 0xc134U,
	0x39c3U, 0x284aU, 0x1ad1U, 0x0b58U, 0x7fe7U, 0x6e6eU, 0x5cf5U, 0x4d7cU,
	0xc60cU, 0xd785U, 0xe51eU, 0xf497U, 0x8028U, 0x91a1U, 0xa33aU, 0xb2b3U,
	0x4a44U, 0x5bcdU, 0x6956U, 0x78dfU, 0x0c60U, 0x1de9U, 0x2f72U, 0x3efbU,
	0xd68dU, 0xc704U, 0xf59fU, 0xe416U, 0x90a9U, 0x8120U, 0xb3bbU, 0xa232U,
	0x5ac5U, 0x4b4cU, 0x79d7U, 0x685eU, 0x1ce1U, 0x0d68U, 0x3ff3U, 0x2e7aU,
	0xe70eU, 0xf687U, 0xc41cU, 0xd595U, 0xa12aU, 0xb0a3U, 0x8238U, 0x93b1U,
	0x6b46U, 0x7acfU, 0x4854U, 0x59ddU, 0x2d62U, 0x3cebU, 0x0e70U, 0x1ff9U,
	0xf78fU, 0xe606U, 0xd49dU, 0xc514U, 0xb1abU, 0xa022U, 0x92b9U, 0x8330U,
	0x7bc7U, 0x6a4eU, 0x58d5U, 0x495cU, 0x3de3U, 0x2c6aU, 0x1ef1U, 0x0f78U,
};
#endif
/* 06-July-2020, by DuYanPo. */
static const uint32_t Crc32Table[ 256 ] =
{
0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B
,0x1A864DB2,0x1E475005,0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61
,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD,0x4C11DB70,0x48D0C6C7
,0x4593E01E,0x4152FDA9,0x5F15ADAC,0x5BD4B01B,0x569796C2,0x52568B75
,0x6A1936C8,0x6ED82B7F,0x639B0DA6,0x675A1011,0x791D4014,0x7DDC5DA3
,0x709F7B7A,0x745E66CD,0x9823B6E0,0x9CE2AB57,0x91A18D8E,0x95609039
,0x8B27C03C,0x8FE6DD8B,0x82A5FB52,0x8664E6E5,0xBE2B5B58,0xBAEA46EF
,0xB7A96036,0xB3687D81,0xAD2F2D84,0xA9EE3033,0xA4AD16EA,0xA06C0B5D
,0xD4326D90,0xD0F37027,0xDDB056FE,0xD9714B49,0xC7361B4C,0xC3F706FB
,0xCEB42022,0xCA753D95,0xF23A8028,0xF6FB9D9F,0xFBB8BB46,0xFF79A6F1
,0xE13EF6F4,0xE5FFEB43,0xE8BCCD9A,0xEC7DD02D,0x34867077,0x30476DC0
,0x3D044B19,0x39C556AE,0x278206AB,0x23431B1C,0x2E003DC5,0x2AC12072
,0x128E9DCF,0x164F8078,0x1B0CA6A1,0x1FCDBB16,0x018AEB13,0x054BF6A4
,0x0808D07D,0x0CC9CDCA,0x7897AB07,0x7C56B6B0,0x71159069,0x75D48DDE
,0x6B93DDDB,0x6F52C06C,0x6211E6B5,0x66D0FB02,0x5E9F46BF,0x5A5E5B08
,0x571D7DD1,0x53DC6066,0x4D9B3063,0x495A2DD4,0x44190B0D,0x40D816BA
,0xACA5C697,0xA864DB20,0xA527FDF9,0xA1E6E04E,0xBFA1B04B,0xBB60ADFC
,0xB6238B25,0xB2E29692,0x8AAD2B2F,0x8E6C3698,0x832F1041,0x87EE0DF6
,0x99A95DF3,0x9D684044,0x902B669D,0x94EA7B2A,0xE0B41DE7,0xE4750050
,0xE9362689,0xEDF73B3E,0xF3B06B3B,0xF771768C,0xFA325055,0xFEF34DE2
,0xC6BCF05F,0xC27DEDE8,0xCF3ECB31,0xCBFFD686,0xD5B88683,0xD1799B34
,0xDC3ABDED,0xD8FBA05A,0x690CE0EE,0x6DCDFD59,0x608EDB80,0x644FC637
,0x7A089632,0x7EC98B85,0x738AAD5C,0x774BB0EB,0x4F040D56,0x4BC510E1
,0x46863638,0x42472B8F,0x5C007B8A,0x58C1663D,0x558240E4,0x51435D53
,0x251D3B9E,0x21DC2629,0x2C9F00F0,0x285E1D47,0x36194D42,0x32D850F5
,0x3F9B762C,0x3B5A6B9B,0x0315D626,0x07D4CB91,0x0A97ED48,0x0E56F0FF
,0x1011A0FA,0x14D0BD4D,0x19939B94,0x1D528623,0xF12F560E,0xF5EE4BB9
,0xF8AD6D60,0xFC6C70D7,0xE22B20D2,0xE6EA3D65,0xEBA91BBC,0xEF68060B
,0xD727BBB6,0xD3E6A601,0xDEA580D8,0xDA649D6F,0xC423CD6A,0xC0E2D0DD
,0xCDA1F604,0xC960EBB3,0xBD3E8D7E,0xB9FF90C9,0xB4BCB610,0xB07DABA7
,0xAE3AFBA2,0xAAFBE615,0xA7B8C0CC,0xA379DD7B,0x9B3660C6,0x9FF77D71
,0x92B45BA8,0x9675461F,0x8832161A,0x8CF30BAD,0x81B02D74,0x857130C3
,0x5D8A9099,0x594B8D2E,0x5408ABF7,0x50C9B640,0x4E8EE645,0x4A4FFBF2
,0x470CDD2B,0x43CDC09C,0x7B827D21,0x7F436096,0x7200464F,0x76C15BF8
,0x68860BFD,0x6C47164A,0x61043093,0x65C52D24,0x119B4BE9,0x155A565E
,0x18197087,0x1CD86D30,0x029F3D35,0x065E2082,0x0B1D065B,0x0FDC1BEC
,0x3793A651,0x3352BBE6,0x3E119D3F,0x3AD08088,0x2497D08D,0x2056CD3A
,0x2D15EBE3,0x29D4F654,0xC5A92679,0xC1683BCE,0xCC2B1D17,0xC8EA00A0
,0xD6AD50A5,0xD26C4D12,0xDF2F6BCB,0xDBEE767C,0xE3A1CBC1,0xE760D676
,0xEA23F0AF,0xEEE2ED18,0xF0A5BD1D,0xF464A0AA,0xF9278673,0xFDE69BC4
,0x89B8FD09,0x8D79E0BE,0x803AC667,0x84FBDBD0,0x9ABC8BD5,0x9E7D9662
,0x933EB0BB,0x97FFAD0C,0xAFB010B1,0xAB710D06,0xA6322BDF,0xA2F33668
,0xBCB4666D,0xB8757BDA,0xB5365D03,0xB1F740B4 };

#if 0
/*******************************************************************************************
 ** @brief: Common_CRC16
 ** @param: pData: the data for CRC
	        nLength: the Length of data for CRC
*******************************************************************************************/
uint16_t Common_CRC16(uint8_t* pData, uint32_t nLength)
{
	uint32_t k;
	uint16_t crc = 0U;
	
  /* 30-August-2018, by Liang Zhen. */
  #if 0
	if ( nLength > 0xffffU )
  #else
  if ( ( NULL == pData ) || ( 0U == nLength ) )
  #endif
	{
		k = 0x17U;
		return ( 0U );
	} /* end if */
	
	while ( nLength > 0u )
	{
		k   = ( crc ^ *pData ) & 0xffU;
		crc = ( crc >> 8u ) ^ crctab16[ k ];
		nLength--;
		pData++;
	} /* end while */
  
	return ( ~crc );
}
#endif

/* 06-July-2020, by DuYanPo. */
/*******************************************************************************
  * @函数名称	  uint32_t Crc32GetCrc(uint8_t *pData, uint16_t len)
  * @函数说明   计算CRC32校验值，换装用于计算整个文件CRC
  * @输入参数   pData 数据缓存 ； len 数据长度 ； u32_nReg CRC计算初始值；
*******************************************************************************/
uint32_t CRC32CCITT(uint8_t *pData, uint32_t len, uint32_t u32_nReg)
{
  uint32_t nTemp=0,nTmp;
  uint16_t i, n;

  for(n=0; n<len; n+=4)
  {
  	if(len - n >= 4)  
  	{
  	  memcpy(&nTmp,&pData[n],4);   /** ??4???*/
    }
		else
		{               
    	nTmp = 0;
    	memcpy(&nTmp,&pData[n],len - n);   /** ??????*/
    }
    u32_nReg ^= nTmp;                
    for(i=0; i<4; i++)
    {
      nTemp = Crc32Table[(uint8_t)(( u32_nReg >> 24 ) & 0xff)]; /**?????,??*/
      u32_nReg <<= 8;                         /**?????????BYTE */
      u32_nReg ^= nTemp;                      /**????BYTE??????? */
    }
  }
  return u32_nReg;
}

#if 0   //TODO(mingzhao)
/*******************************************************************************************
 ** @brief: CAN_OutTime
 ** @param: time: variable for count
               ms: the length for timeout 
*******************************************************************************************/
uint8_t CAN_OutTime( uint32_t time, uint32_t ms )
{	
	if( ( GetTicks() - time ) > ms )
	{
		return 1u;
	}
	else
	{
		return 0u;
	} /* end if...else */
}


/*******************************************************************************************
 ** @brief: Common_BeTimeOutMN
 ** @param: *time: variable for count
               ms: the length for timeout 
*******************************************************************************************/

uint8_t Common_BeTimeOutMN( uint32_t *time, uint32_t ms )
{
	uint8_t  tmp = 0u;
	uint32_t curtv;
	
	curtv = GetTicks();
	if ( ( curtv - *time ) >=ms )
	{
		*time = curtv;
		tmp   = 1u;
	}
  else
	{
		tmp = 0u;
	} /* end if...else */
	
	return tmp;
}
#endif
#if 0   //TODO(mingzhao)
/*----------------------------------------------------------------------------
* ??:arraylist_init()
* ??:???????
* ????:item_count: ????
*				  item_size:  ????
* ????:sArrayList *: ????????
----------------------------------------------------------------------------*/
sArrayList *Common_arraylist_init(uint8_t *pname, uint32_t item_count, uint32_t item_size)
{
	sArrayList *pal;
	uint8_t *p   = NULL;
	uint32_t len = 0u;

	pal = malloc( sizeof( sArrayList ) );
  
	if ( NULL != pal )
	{
		p = malloc( item_count * item_size );
		if ( NULL == p )
		{
			free( pal );
			return NULL;
		} /* end if */
    
		pal->write_idx  = 0u;
		pal->read_idx   = 0u;
		pal->item_count = item_count;
		pal->item_size  = item_size;
		memset( &pal->name[ 0u ], 0u, 24u );
		if ( NULL != pname )
		{
			len = strlen( ( const char * )pname );
			if ( len > 23u )
			{
				len = 23u;
			} /* end if */
			memcpy( &pal->name[ 0u ], pname, len );
		} /* end if */
		pal->parray = p;
		
		return pal;
	}
	else
	{
		return NULL;
	} /* end if...else */
}


/*----------------------------------------------------------------------------
* ??:arraylist_add()
* ??:????????
* ????:*pal: ????????
*				  *pdata:  ????
*				  len: ????
* ????:bool: true/false
----------------------------------------------------------------------------*/
bool Common_arraylist_add( sArrayList *pal, uint8_t *pdata, uint32_t len )
{
	uint32_t idx=0;
  /* 30-August-2018, by Liang Zhen. */
  #if 0
	uint32_t id=0;
  uint32_t i;
  #endif
	
	if( ( NULL != pal ) && ( NULL != pal->parray ) )
	{
		if( len == pal->item_size )
		{
			pal->write_idx = pal->write_idx % pal->item_count;
			
			idx = pal->write_idx * pal->item_size;
      
			memcpy( &pal->parray[ idx ], pdata, pal->item_size );
			
			pal->write_idx++;
			pal->write_idx = pal->write_idx % pal->item_count;
			
			return true;
		}
		else
		{
			printf( "name:%s arraylist_add err !\r\n", &pal->name[0] );
			return false;
		} /* end if...else */
	}
	else
	{
		printf( "NULL arraylist_add err !\r\n" );
		return false;
	} /* end if...else */
}


/*----------------------------------------------------------------------------
* ??:arraylist_get()
* ??:?????????
* ????:*pal: ????????
* ????:pdata: ????
*				  uint32_t: ????
----------------------------------------------------------------------------*/
uint32_t Common_arraylist_get( sArrayList *pal, uint8_t *pdata )
{
	uint32_t idx = 0;
  /* 30-August-2018, by Liang Zhen. */
  #if 0
  uint32_t i;
  #endif
  
	if( ( NULL != pal ) && ( NULL != pal->parray ) && ( NULL != pdata ) )
	{
		if( ( pal->read_idx < pal->item_count ) && ( pal->read_idx != pal->write_idx ) )
		{
      
			idx = pal->read_idx * pal->item_size;
      
			memcpy( pdata, &pal->parray[ idx ], pal->item_size );
      
			pal->read_idx++;
			pal->read_idx = pal->read_idx % pal->item_count;
			
			return pal->item_size;
		}
    else
		{
			return 0u;
		} /* end if...else */
	}
	else
	{
		printf("NULL arraylist_get err !\r\n");
		return 0u;
	} /* end if...else */
}


void UartBuf_add( UART_BUF *rec_buf, uint8_t data )
{
	
	rec_buf->buf[ rec_buf->write_pos ] = data;
	
	rec_buf->write_pos++;
	
	if( rec_buf->write_pos >= 100u )
	{
		rec_buf->write_pos = 0u;
	} /* end if */
	
	rec_buf->size++;
	
	if( rec_buf->size > 100u )
	{
		rec_buf->size = 100u;
	} /* end if */
}


uint8_t UartBuf_get( UART_BUF *rec_buf, uint8_t *data )
{	
	if( rec_buf->size == 0u )
	{
		return 0u;
	} /* end if */
	
	*data = rec_buf->buf[ rec_buf->read_pos ];
	
	rec_buf->size--;
	
	rec_buf->read_pos++;
	
	if( rec_buf->read_pos >= 100u )
	{
		rec_buf->read_pos = 0u;
	} /* end if */
	
	return 1u;
}

#endif
uint8_t u16_abscmp( uint16_t a, uint16_t b, uint16_t val )
{
	if ( a > b )
	{
		if ( a - b >= val )
		{
			return 1u;
		} /* end if */
	}
	else if ( a < b )
	{
		if ( b - a >= val )
		{
			return 1u;
		} /* end if */
	}
	else
	{
	} /* end if...else if...else */
	
	return 0u;
}


/**************************************************************************************************
(^_^) Function Name : GetBoardType.
(^_^) Brief         : Getting the type of board.
(^_^) Parameter     : none.
(^_^) Return        : Return the type of board.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Tip           : Before using the program, you should read comments carefully.
**************************************************************************************************/
BOARD_TYPE GetBoardType( void )
{
	BOARD_TYPE board;
	uint8_t ucID = ID >> 1u;
	
	switch( ucID )
	{
		case 0x04u : board = BOARD_TRAIN_INTERFACE; break;
		case 0x05u : board = BOARD_RECORD;          break;
		case 0x06u : board = BOARD_COMMUNICATION;   break;
		default    : board = BOARD_UNKNOWN;         break;
	} /* end switch */
	
	return board;
} /* end function GetBoardType */


/**************************************************************************************************
(^_^) Function Name : Get_CPU_Type.
(^_^) Brief         : Getting the type of CPU.
(^_^) Parameter     : none.
(^_^) Return        : Return the type of CPU.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Tip           : Before using the program, you should read comments carefully.
**************************************************************************************************/
CPU_TYPE Get_CPU_Type( void )
{
	CPU_TYPE cpu;
	uint8_t ucID = ID & 0x01u;
	
	if( ucID == 0x00u )
	{
		cpu = CPU_A;
	}
	else
	{
		cpu = CPU_B;
	} /* end if...else */
	
	return cpu;
} /* end function Get_CPU_Type */


/**************************************end file*********************************************/

