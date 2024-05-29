/***************************************************************************
文件名：comm_proc_ctl.h
模  块：应用层通道协议解析头文件
详  述：
by lzy
***************************************************************************/
#ifndef COMM_PROC_CTL_H
#define COMM_PROC_CTL_H


/***********************************************************************/
/* 定义通道编号 */
/* 对系内通道 */
#define IN_ETH_DEV          (0x01)				/* 内网 */
#define IN_FDCAN1_DEV      	(0x02)				/* 内CAN1 */
#define IN_FDCAN2_DEV      	(0x03)				/* 内CAN2 */

#define COMM_MAX_PAYLOAD_LEN (1500U) /*安全层数据最大为1500*/
#define SAFE_LAYER_PLOADLEN  (1480U) /*安全层负载数据区最大为1480= 1500 - 14 - 4- 4 *///存疑问？为什么
#define APP_LAYER_PLOADLEN   (1476U) /*应用层负载区数据最大为1480-app_head(4)*/

/*应用层报文类型*/
#define ROUND_CIRCLE_TYPE      3 /*轮循模式  请求方*/
#define ROUND_CIRCLE_ACK_TYPE  5 /*轮循模式  应答方*/
#define ROUND_PULSE_MODE       6 /*周期模式  有应答模式*/

/*应用层帧子类型*/
#define TIME_SET_LOCAL     3   /*轮循模式时 时钟同步，主控下发时钟通信插件同步 业务数据为4个字节 有应答*/
#define IAP_PAKAGE         5   /*周期模式时 IAP数据包  业务数据为IAP数据包  无应答*/
#define APPLY_TIME_SET     6   /*轮循模式时 向主控申请时钟同步  有应答*/
#define SET_CHANNL_INFO    23  /*轮循模式时 设置通道配置信息，即为配置信息 通道数目+通道1参数+通道2参数+N参数 有应答 2字节OK或ERR*/
#define GET_CHANNL_INFO    24  /*轮循模式时 查询通道配置信息，通道数目+通道号+通道号+N 有应答 通道数目+通道1参数+通道2参数+N参数*/
#define SET_CONTIUE_INFO   27  /*轮循模式时 设置数据转发路由表，路由表数目1字节+路由表1(4字节)+N 有应答 2字节OK或ERR*/
#define GET_CONTIUE_INFO   29  /*轮循模式时 查询数据转发路由表，2字节保留 有应答  路由表数目1字节+路由表1(4字节)+N*/
#define COMM_PLUG_INFO     30  /*轮循模式时 主控获取插件信息，周期+平态工作状态 有应答*/
#define RX_MAINCTLDATA_EXP 33  /*周期模式时 收到主控数据发送到外部通道  通道数目+通道数据(通道编号1+时间戳4+长度2+数据N)+N  无应答*/
#define RX_EXPDATA_MAINCTL 34  /*周期模式时 收到外部数据发送给主控     通道数目+通道数据(通道编号1+时间戳4+长度2+数据N)+N 无应答*/



#if 0
//comm

/* 数据流通道 */

/* TX1插件CAN通道信息 */
/* TX1插件CAN通道号  */
#define DATA_CHANNEL_TX1CAN1 ( 0x10U )      /* 通信1的CAN1通道 */
#define DATA_CHANNEL_TX1CAN2 ( 0x11U )      /* 通信1的CAN2通道 */
#define DATA_CHANNEL_TX1CAN3 ( 0x12U )      /* 通信1的CAN3通道 */
#define DATA_CHANNEL_TX1CAN4 ( 0x13U )      /* 通信1的CAN4通道 */
#define DATA_CHANNEL_TX1CAN5 ( 0x14U )      /* 通信1的CAN5通道 */
#define DATA_CHANNEL_TX1VMCAN1 ( 0x15U )     /* 通信1的VMCAN1通道 */
#define DATA_CHANNEL_TX1VMCAN2 ( 0x16U )     /* 通信1的VMCAN2通道 */

/* TX2插件CAN通道号 */
#define DATA_CHANNEL_TX2CAN1 ( 0x17U )      /* 通信2的CAN1通道 */
#define DATA_CHANNEL_TX2CAN2 ( 0x18U )      /* 通信2的CAN2通道 */
#define DATA_CHANNEL_TX2CAN3 ( 0x19U )      /* 通信2的CAN3通道 */
#define DATA_CHANNEL_TX2CAN4 ( 0x1AU )      /* 通信2的CAN4通道 */
#define DATA_CHANNEL_TX2CAN5 ( 0x1BU )      /* 通信2的CAN5通道 */
#define DATA_CHANNEL_TX2VMCAN1 ( 0x1CU )     /* 通信2的VMCAN1通道 */
#define DATA_CHANNEL_TX2VMCAN2 ( 0x1DU )     /* 通信2的VMCAN2通道 */

#define DATA_CHANNEL_TX1ETH1 ( 0x30U )      /* 通信1的ETH1通道 */
#define DATA_CHANNEL_TX1ETH2 ( 0x31U )      /* 通信1的ETH2通道 */
#define DATA_CHANNEL_TX1ETH3 ( 0x32U )      /* 通信1的ETH3通道 */
#define DATA_CHANNEL_TX1ETH4 ( 0x33U )      /* 通信1的ETH4通道 */

#define DATA_CHANNEL_TX2ETH1 ( 0x34U )      /* 通信2的ETH1通道 */
#define DATA_CHANNEL_TX2ETH2 ( 0x35U )      /* 通信2的ETH2通道 */

/* TX插件HDLC通道信息  */
#define DATA_CHANNEL_TX2HDLC ( 0x50U )      /* 通信2的HDLC通道 */
#define DATA_CHANNEL_TX2MVB  ( 0x60U )      /* 通信2MVB通道 */

/* TX插件RS485通道号 */
#define DATA_CHANNEL_TX1RS485a ( 0x80U )    /* 通信1RS485a通道 */
#define DATA_CHANNEL_TX1RS485b ( 0x81U )    /* 通信1RS485b通道 */
#define DATA_CHANNEL_TX2RS485a ( 0x82U )    /* 通信2RS485a通道 */





/***********************************************************************/
#define COMM_RBUF_SIZE       (1536)
#define COMM_RBUF_NUM        (8)


#define EXPORT_DATA_PLOADLEN (1468) /*应用层负载区数据最大为1476-exp_head(8)*/
#define	MAX_ETH_CAN_LEN			 (1465) /*应用层负载区数据最大为1476-exp_head(8)-pack_head(3)*/
#define	LIMIT_EXPORT_DATA_PLOADLEN	(1400) /*应用层负载区数据最大为1476-exp_head(8)-pack_head(3)*/
#define SYS_CAN_MAX_PAYLOAD_LEN       (COMM_MAX_PAYLOAD_LEN - 1 - 1 - 4)
#define COMM_EXP_TX_IDLE_TIMEOUT (5)




#define MAC_ADDR_LEN  6 /*MAC地址长度*/
/*路由表数量*/
#define CHL_NUM   28   
/** 1字节对齐*/
#pragma pack(1)
typedef struct tag_comm_node {
  uint8 dev;
	uint32 tick;
	uint32 len;
  uint8 buf[COMM_RBUF_SIZE];
} s_comm_node;

typedef struct tag_comm_msg
{
  uint8 dest[6];
  uint8 src[6];
  uint16 len;
	uint8 payload[COMM_MAX_PAYLOAD_LEN];
}s_comm_msg;

typedef struct tag_sys_can_msg
{
	uint32 packet_no;
	uint8 channel_id;
	uint8 reserved;
	uint8 payload[SYS_CAN_MAX_PAYLOAD_LEN];
}s_sys_can_msg;

typedef struct tag_comm_rbuf  {
  uint8 in;
  uint8 out;
  s_comm_node node[COMM_RBUF_NUM];
} s_comm_rbuf;

/*安全层帧头解析结构*/
typedef struct eth_rx_safe_layer {
    uint8 des_adr;                  /* 目的地址 */
    uint8 src_adr;                  /* 源地址 */
    uint8 sig_pos;                  /* 标识位 */
    uint8 res;                      /* 预留 */
    uint32 serial_num;              /* 序列号 */
    uint32 time_print;              /* 时间戳 */
    uint16 lenth;                   /* 安全层整体长度 */
}rx_safe_layer;

/*********************应用层**************************/
/*应用层报文做为安全层的负载区传输*/
typedef struct eth_rx_app_layer {
	 uint8 msg_type; /*报文类型*/
	 uint8 msg_sub_type; /*报文子类型*/
	 uint16 serial_num; /*报文序列号*/
} r_app_layer;
/***********************应用层end********************/

/********************插件信息***************************/
/*插件通道信息*/
typedef struct comm_chanl_info {
	uint8   chanl_index;
	uint8   chanl_sta;
	uint32  Tx_datalen;
  uint32  Rx_datalen;
 } s_chanl_info;
/*插件通道信系电压、电流*/
typedef struct comm_chanl_power {
	uint16   plug_power;
	uint16   plug_current;
 } s_chanl_power;
/*插件信息固定参数*/
typedef struct comm_plug_info {
    uint32 ctl_cpu_soft_ver;
	uint32 comm_cpu_soft_ver;
	uint32 ctl_self_A;
	uint32 ctl_self_B;
	uint8  chl_self_all; /*0x55 正常 0xAA异常*/
	uint8  plug_id[8];
	uint32 open_Times;
    uint32 open_long_Time;
	uint8  tempr_num;
	uint16 plug_tempr;
	uint8  power_channls;
	uint16 power_vot;
	uint16 power_curret;
	uint8  chl_num;
 } s_plug_info;
/***********************插件信息end********************/
/*************************通道**配置信息****************************/
 /*以太网通道*/
 typedef struct eth_chanl_refer {
  uint8  channl_index; /*通道索引号*/
	uint8  relay_sig;    /*转发标识 0x55转发 其它不转发*/
	uint8  iap_deal;     /*IAP处理  0x55处理IAP*/
  uint8  tcp_raw;      /* 0x55TCP协议栈 0xAARWA格式 其它无效*/
	uint32 IP_adr;       /* IP地址*/
	uint32 IP_mask;      /*子网掩码*/
	uint32 face_IP;      /*对端IP*/
	uint16 port;         /*本端口号*/
	uint16 face_port;    /*对端端口号*/
	uint8  conect_type;  /*连接类型 1TCP 2UDP*/
	uint8  acter;        /*服务器或管户端 0xAA服务器端  其它客户端*/
 } s_eth_chanl_refer;
 /*CAN通道*/
 typedef struct can_chanl_refer {
  uint8  channl_index; /*通道索引号*/
	uint8  relay_sig;    /*转发标识 0x55转发 其它不转发*/
	uint8  iap_deal;     /*IAP处理  0x55处理IAP*/
  uint8  can_mode;     /*can模式 1标准CAN 2 FDCAN 3 29位标识符can 4 29位标识符FDCAN*/
	uint32 just_beaut;    /* 仲裁区波特率*/
	uint32 data_beaut;     /*数据区波特率*/
	uint8  data_maxlen;     /*数据区最大长度*/
 } s_can_chanl_refer;
 /*RS422通道*/
 typedef struct rs422_chanl_refer {
  uint8  channl_index; /*通道索引号*/
	uint8  relay_sig;    /*转发标识 0x55转发 其它不转发*/
	uint8  iap_deal;     /*IAP处理  0x55处理IAP*/
  uint32 uart_beaut;   /*波特率*/
	uint8  start_sig;    /*起始位*/
	uint8  data_sig;     /*数据位*/
	uint8  stop_sig;     /*停止位*/
 } s_rs422_chanl_refer;
 /**********************通道**配置信息**end*********************************/
 /**********************路由表****************************************/
  /*通道转发路由表*/
  typedef struct chanl_router {
  uint8  channl_index; /*通道索引号*/
	uint8  src_adr;      /*源地址*/
	uint8  des_adr;      /*目的地址*/
	uint8  target_chl;   /*目的通道号*/
 } s_chanl_router;
 /************************路由表*end********************************/
/**************************内转发外部接口数据*********************************/
/*外部通道进来的数据格式*/
typedef struct exp_chanl {
		uint8  channl_index; /*通道索引号*/
		uint32 time_print;   /*时间戳*/
		uint16 data_len;     /*数据长度*/
    uint8 buf[EXPORT_DATA_PLOADLEN];	
  } r_exp_chanl;
/*外部通道头数据格式*/
typedef struct exp_chanl_head {
		uint8  channl_index; /*通道索引号*/
		uint32 time_print;   /*时间戳*/
		uint16 data_len;     /*数据长度*/
  } h_exp_chanl;
/*外部通道进来的协议中的CAN帧格式*/
typedef struct exp_CAN {
		uint32  fram_sig;      /*帧标识*/
		uint8   fram_type;     /*帧类型*/
		uint8   can_fram_size;  /*can 帧大小*/
	  uint8 buf[256];
 } r_exp_CAN;
/*接收到外部接口数据组成一大包送到系内以太网*/
typedef struct exp_I_II_eth {
	 uint16 data_len;     /*数据长度*/
   uint8 buf[APP_LAYER_PLOADLEN];
} t_exp_I_II_eth;
/* 外部通道数据打包格式 */
typedef struct 
{  
	uint32	frameAllNum;
  uint8 	frameNum;
	uint16	datalen;
	uint8		data[MAX_ETH_CAN_LEN];
}S_APP_INETH_PACK;

/* 单应用数据格式 */
typedef struct 
{  
	uint16	datalen;
	uint8		data[MAX_ETH_CAN_LEN - 7];
}S_APP_DATA_PACK;

 /**************************内转发外部接口数据end******************************/
 #pragma pack()
/***************************配置信息 end*************************/



int32_t comm_rbuf_get(s_comm_node *node);
/****************************************************************************
* 函数名: start_comm_proctl
* 说明:创建协议解析线程
* 参数: void
* 返回值: 线程ID号
 ****************************************************************************/
int start_comm_proctl(void);
/****************************************************************************
* 函数名: rx_safe_layer_check
* 说明:校核安全层数据
* 参数: uint8 *pBuf 安全层数据
        uint8 from_chl 来源信道
* 返回值: 安全层校验合法标识
 ****************************************************************************/
sint8 rx_safe_layer_check(uint8 *pBuf , uint8 from_chl);
 /****************************************************************************
 * 函数名: app_add_safelayer_pakage_tx
* 说明:把APP的数据封装成安全层并发到相应的通道
* 参数: uint8 *pSafe 收到的安全层信息
*      uint8 *pApp  应用层据
*      uint16 app_len 应用层数据长度
* 返回值: 无
 ****************************************************************************/
void app_add_safelayer_pakage_tx(uint8 *pSafe , uint8 *pApp , uint16 app_len);
/****************************************************************************
* 函数名: app_layer_check
* 说明:分析处理应用层数据
* 参数: uint8 *pBuf 应用层数据
*       uint8 *p_safe_layer 收到的安全层数据
* 返回值: 应用层处理结果标识
 ****************************************************************************/
sint8 app_layer_check(uint8 *pBuf , uint8 *p_safe_layer);
/****************************************************************************
 * 函数名: comm_pubg_info
* 说明:平台获取通信插件信息 电压 电流 软件版本等
* 参数:   uint8 *pSafe 安全层数据
*         r_app_layer *pApp_layer  应用层数据
* 返回值: 无
 ****************************************************************************/
void get_comm_plug_info(uint8 *pSafe , r_app_layer *pApp_layer);
/****************************************************************************
* 函数名: set_exp_chanl_refer
* 说  明: 收到帧 存储到本地通道参数
* 参数:   uint8 *p_safe_layer 安全层数据
*         r_app_layer *pApp_layer  应用层数据
*         uint8 *pbuf 应用层业数据
*         uint8 chnal_num  应用层业数据块数
* 返回值: 设置结果标识
 ****************************************************************************/
sint16 set_exp_chanl_refer(uint8 *p_safe_layer , r_app_layer *pApp_layer, uint8 *pbuf , uint8 chnal_num );
/****************************************************************************
* 函数名: get_exp_chanl_refer
* 说  明:	获取本地通道参数
* 参数:   uint8 *p_safe_layer 安全层数据
*         r_app_layer *pApp_layer  应用层数据
*         uint8 *pbuf 应用层业数据
*         uint8 chnal_num  应用层业数据块数
* 返回值: 无
 ****************************************************************************/
void get_exp_chanl_refer(uint8 *p_safe_layer , r_app_layer *pApp_layer, uint8 *pbuf , uint8 chnal_num);
/****************************************************************************
* 函数名: save_exp_chanl_refer
* 说  明:收到帧 设置到本地通道参数
* 参  数:  uint8 *pbuf 通道号及参数
         uint8 chnal_num 通道数量
* 返回值: 存储结果标识
 ****************************************************************************/
void save_exp_chanl_refer(uint8 *pbuf , uint8 chnal_num);
/****************************************************************************
* 函数名: exp_chanl_refer_init
* 说  明:初始化对外通道端口参数
* 参  数: 无
* 返回值: 无
 ****************************************************************************/
void exp_chanl_refer_init(void);
/****************************************************************************
* 函数名: app_chl_router_find
* 说  明: 查找路由表
* 参  数: uint8 chl_code  通道号
* 返回值: 返回查找到的需查找的通道路由表
 ****************************************************************************/
s_chanl_router *app_chl_router_find(uint8 chl_code);
/****************************************************************************
* 函数名: src_chl_find_des_chl
* 说  明:根据源通道号查找目的通道号
* 参数: uint8 src_chl_code  源通道号
* 返回值: 目的通道号
 ****************************************************************************/
uint8 src_chl_find_des_chl(uint8 src_chl_code);
/****************************************************************************
* 函数名: get_can_chl_refer
* 说  明:根据外部CAN 通道号，遍历出对应的外部CAN端口参数
* 参数:  uint8 can_chnal_code can外部通道号
* 返回值: 对应的CAN端口参数指针地址
 ****************************************************************************/
s_can_chanl_refer *get_can_chl_refer(uint8 can_chnal_code);
/****************************************************************************
* 函数名: app_chl_router_get
* 说  明:获取路由表
* 参数: uint8 *p_safe_layer 安全层数据
*       r_app_layer *pApp_layer  应用层数据
* 返回值: 无
 ****************************************************************************/
void app_chl_router_get(uint8 *p_safe_layer , r_app_layer *pApp_layer);
/****************************************************************************
* 函数名: app_chl_router_save
* 说  明:存储路由表
* 参数:   uint8 *p_safe_layer 安全层数据
*         r_app_layer *pApp_layer  应用层数据
*         uint8 *pbuf 应用层业数据
*         uint8 chnal_num  应用层业数据块数
* 返回值: 无
 ****************************************************************************/
void app_chl_router_save(uint8 *p_safe_layer , r_app_layer *pApp_layer,uint8 *pbuf , uint8 chnal_num);
/****************************************************************************
* 函数名: mainctl_2_export
* 说  明: 主控发来的数据发到外部通道
* 参  数: uint8 *pbuf主控发来的应用层业务数据中的通道数据
          uint8 chnal_num 通道数据 对应的通道数量
* 返回值: 无
 ****************************************************************************/
void mainctl_2_export( uint8 *pBuf , uint8  chl_num);
/****************************************************************************
* 函数名:  export_2_mainctl_wait_coming
* 说  明:  外部来的数据转发到内部通道 和超时发送
* 参数:
*         s_comm_node *p_s_node 通道数据块
*         uint8 time_MS   超时值
* 返回值: 无
 ****************************************************************************/
void export_2_mainctl_wait_coming(s_comm_node *p_s_node , uint8 time_MS);
/****************************************************************************
 * 函数名: add_applayer_pakage_tx
* 说明:封装APP 层
* 参数:   uint8 *pSafe 安全层数据
*         r_app_layer *pApp  应用层数据
*         uint8 *pbuf 应用层业数据
*         uint8 app_len  应用层业数据长度
* 返回值: 无
 ****************************************************************************/
void add_applayer_pakage_tx(uint8 *pSafe , r_app_layer *pApp , uint8 *pbuf , uint16 app_len);
/****************************************************************************
 * 函数名: comm_rbuf_put
* 说明:
* 参数:
* 返回值:
 ****************************************************************************/
int32_t comm_rbuf_put(s_comm_node *node);
/****************************************************************************
* 函数名: start_in_Can_Comm_Task
* 说明：创建系内冗余CAN接口 接收线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_in_Can_Comm_Task(void);
/****************************************************************************
* 函数名: start_exp_Can_Comm_Task
* 说明：创建外部CAN接口 接收线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_exp_Can_Comm_Task(void);
/****************************************************************************
* 函数名: start_exp_net1_udp_Task
* 说明：外部带协议栈以太网口启动线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_exp_net1_udp_Task(void);
/****************************************************************************
* 函数名: start_exp_net1_tcp_Task
* 说明：外部带协议栈以太网口启动线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_exp_net1_tcp_Task(void);
/********************************************
* 函数名: start_exp_net2_udp_Task
* 说明：外部带协议栈以太网口启动线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
*********************************************/
int start_exp_net2_udp_Task(void);
/****************************************************************************
* 函数名: start_exp_net2_tcp_Task
* 说明：外部带协议栈以太网口启动线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_exp_net2_tcp_Task(void);
/****************************************************************************
* 函数名: init_rs422_refer
* 说明�r初始化接收参数
* 参数: 无
* 返回值:  无
 ****************************************************************************/
void init_rs422_refer(void);
/****************************************************************************
* 函数名: start_RS422_Task
* 说明：启动外部接口RS422接收线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_RS422_Task(void);
/**************************************************************************
函数：Tx_Data_2_exp_1net
功能：把数据发送到外部以太网接口
参数：Data，数据指针；len，数据长度
返回：OK，成功分发；ER，错误
***************************************************************************/
int Tx_Data_to_exp_1net(uint8 *Data,uint16 len);
/**************************************************************************
函数：Tx_Data_to_exp_2net
功能：把数据发送到外部以太网接口
参数：Data，数据指针；len，数据长度
返回：OK，成功分发；ER，错误
***************************************************************************/
int Tx_Data_to_exp_2net(uint8 *Data,uint16 len);
/****************************************************************************
* 函数名: tx_data_to_rs422_I
* 说明�r发送数据到外部的RS422I 接口
* 参数: const char  *buffer, 发送的数据
        size_t buflen  数据长度
* 返回值:  无
 ****************************************************************************/
void tx_data_to_rs422_I(uint8 *Data, size_t buflen);
/****************************************************************************
* 函数名: tx_data_to_rs422_II
* 说明�r发送数据到外部的RS422II接口
* 参数: const char  *buffer, 发送的数据
        size_t buflen  数据长度
* 返回值:  无
 ****************************************************************************/
void tx_data_to_rs422_II(uint8 *Data, size_t buflen);

/****************************************************************************
* 函数名: tx_data_to_exp_can
* 说明�r发送数据到外部CAN接口
* 参数: uint8 *Data, 发送的数据
        size_t buflen  数据长度
* 返回值:  无
 ****************************************************************************/
void tx_data_to_exp_can(uint8 des_chl , uint8 *Data, size_t buflen);

/****************************************************************************
* 函数名: start_in_net_Comm_Task
* 说明：系内网
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_in_net_Comm_Task(void);
/****************************************************************************
 * 函数名: comm_rbuf_init
* 说明:
* 参数: 无
* 返回值: 无
 ****************************************************************************/
void comm_rbuf_init(void);
/****************************************************************************
* 函数名: in_net_I_II_comm_open_dev
* 说明：打开设备资源节点
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
sint32 in_net_I_II_comm_open_dev(void);
/****************************************************************************
* 函数名: exp_can_comm_open_dev
* 说明：打开设备资源节点
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
sint32 exp_can_comm_open_dev(void);
/****************************************************************************
* 函数名: in_can_comm_open_dev
* 说明：打开设备资源节点
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
sint32 in_can_comm_open_dev(void);
/****************************************************************************
* 函数名: exp_rs422_comm_open_dev
* 说明：打开设备资源节点
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
sint32 exp_rs422_comm_open_dev(void);
/****************************************************************************
* 函数名: start_exp_Can_Comm_Task
* 说明：创建外部CAN接口 接收线程
* 参数: 无
* 返回值:  < 0  错误  其它正常
 ****************************************************************************/
int start_exp_Can_Comm_test_Task(void);
#endif
#endif



