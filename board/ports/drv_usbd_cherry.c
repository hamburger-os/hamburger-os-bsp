#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>

#include "usbd_core.h"
#include "usbd_cdc.h"

#define DBG_TAG "usbd"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef USBD_USING_CMSIS_DAP
#include "DAP_config.h"
#include "DAP.h"
#endif

#ifdef USBD_USING_CMSIS_DAP
#define DAP_IN_EP  0x81
#define DAP_OUT_EP 0x02

#define CDC_IN_EP  0x83
#define CDC_OUT_EP 0x04
#define CDC_INT_EP 0x85
#endif

#ifdef USBD_USING_CDC
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83
#endif

#define USBD_VID           0xd6e7
#define USBD_PID           0x3507
#define USBD_MAX_POWER     500
#define USBD_LANGID_STRING 1033

#ifdef USBD_USING_CDC
#define USB_CONFIG_SIZE             (9 + CDC_ACM_DESCRIPTOR_LEN)
#elif defined(USBD_USING_CMSIS_DAP)
#define CMSIS_DAP_INTERFACE_SIZE    (9 + 7 + 7)
#define USB_CONFIG_SIZE             (9 + CMSIS_DAP_INTERFACE_SIZE + CDC_ACM_DESCRIPTOR_LEN)
#endif

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

#ifdef HAL_PCD_MODULE_ENABLED
static PCD_HandleTypeDef hpcd_USB;

void usb_dc_low_level_init(void)
{
#if defined (STM32F100xB) || defined (STM32F100xE) || defined (STM32F101x6) || \
    defined (STM32F101xB) || defined (STM32F101xE) || defined (STM32F101xG) || defined (STM32F102x6) || defined (STM32F102xB) || defined (STM32F103x6) || \
    defined (STM32F103xB) || defined (STM32F103xE) || defined (STM32F103xG) || defined (STM32F105xC) || defined (STM32F107xC)

    hpcd_USB.Instance = USB;
    hpcd_USB.Init.dev_endpoints = 8;
    hpcd_USB.Init.speed = PCD_SPEED_FULL;
    hpcd_USB.Init.low_power_enable = DISABLE;
    hpcd_USB.Init.lpm_enable = DISABLE;
    hpcd_USB.Init.battery_charging_enable = DISABLE;
    if (HAL_PCD_Init(&hpcd_USB) != HAL_OK)
    {
        Error_Handler();
    }
#endif

#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)

    hpcd_USB.Instance = USB_OTG_FS;
    hpcd_USB.Init.dev_endpoints = 4;
    hpcd_USB.Init.speed = PCD_SPEED_FULL;
    hpcd_USB.Init.dma_enable = DISABLE;
    hpcd_USB.Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd_USB.Init.Sof_enable = DISABLE;
    hpcd_USB.Init.low_power_enable = DISABLE;
    hpcd_USB.Init.lpm_enable = DISABLE;
    hpcd_USB.Init.vbus_sensing_enable = DISABLE;
    hpcd_USB.Init.use_dedicated_ep1 = DISABLE;
    if (HAL_PCD_Init(&hpcd_USB) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}
#endif

#ifdef USBD_USING_CMSIS_DAP
#define WCID_VENDOR_CODE 0x01

__ALIGN_BEGIN const uint8_t WCID_StringDescriptor_MSOS[18] __ALIGN_END = {
    ///////////////////////////////////////
    /// MS OS string descriptor
    ///////////////////////////////////////
    0x12,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    /* MSFT100 */
    'M', 0x00, 'S', 0x00, 'F', 0x00, 'T', 0x00, /* wcChar_7 */
    '1', 0x00, '0', 0x00, '0', 0x00,            /* wcChar_7 */
    WCID_VENDOR_CODE,                           /* bVendorCode */
    0x00,                                       /* bReserved */
};

__ALIGN_BEGIN const uint8_t WINUSB_WCIDDescriptor[40] __ALIGN_END = {
    ///////////////////////////////////////
    /// WCID descriptor
    ///////////////////////////////////////
    0x28, 0x00, 0x00, 0x00,                   /* dwLength */
    0x00, 0x01,                               /* bcdVersion */
    0x04, 0x00,                               /* wIndex */
    0x01,                                     /* bCount */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* bReserved_7 */

    ///////////////////////////////////////
    /// WCID function descriptor
    ///////////////////////////////////////
    0x00, /* bFirstInterfaceNumber */
    0x01, /* bReserved */
    /* WINUSB */
    'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00, /* cCID_8 */
    /*  */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* cSubCID_8 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,             /* bReserved_6 */
};

__ALIGN_BEGIN const uint8_t WINUSB_IF0_WCIDProperties[142] __ALIGN_END = {
    ///////////////////////////////////////
    /// WCID property descriptor
    ///////////////////////////////////////
    0x8e, 0x00, 0x00, 0x00, /* dwLength */
    0x00, 0x01,             /* bcdVersion */
    0x05, 0x00,             /* wIndex */
    0x01, 0x00,             /* wCount */

    ///////////////////////////////////////
    /// registry propter descriptor
    ///////////////////////////////////////
    0x84, 0x00, 0x00, 0x00, /* dwSize */
    0x01, 0x00, 0x00, 0x00, /* dwPropertyDataType */
    0x28, 0x00,             /* wPropertyNameLength */
    /* DeviceInterfaceGUID */
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00,  /* wcName_20 */
    'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00,  /* wcName_20 */
    't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00,  /* wcName_20 */
    'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00,  /* wcName_20 */
    'U', 0x00, 'I', 0x00, 'D', 0x00, 0x00, 0x00, /* wcName_20 */
    0x4e, 0x00, 0x00, 0x00,                      /* dwPropertyDataLength */
    /* {CDB3B5AD-293B-4663-AA36-1AAE46463776} */
    '{', 0x00, 'C', 0x00, 'D', 0x00, 'B', 0x00, /* wcData_39 */
    '3', 0x00, 'B', 0x00, '5', 0x00, 'A', 0x00, /* wcData_39 */
    'D', 0x00, '-', 0x00, '2', 0x00, '9', 0x00, /* wcData_39 */
    '3', 0x00, 'B', 0x00, '-', 0x00, '4', 0x00, /* wcData_39 */
    '6', 0x00, '6', 0x00, '3', 0x00, '-', 0x00, /* wcData_39 */
    'A', 0x00, 'A', 0x00, '3', 0x00, '6', 0x00, /* wcData_39 */
    '-', 0x00, '1', 0x00, 'A', 0x00, 'A', 0x00, /* wcData_39 */
    'E', 0x00, '4', 0x00, '6', 0x00, '4', 0x00, /* wcData_39 */
    '6', 0x00, '3', 0x00, '7', 0x00, '7', 0x00, /* wcData_39 */
    '6', 0x00, '}', 0x00, 0x00, 0x00,           /* wcData_39 */
};

static struct usb_msosv1_descriptor msosv1_desc = {
    .string = (uint8_t *)WCID_StringDescriptor_MSOS,
    .string_len = 18,
    .vendor_code = WCID_VENDOR_CODE,
    .compat_id = (uint8_t *)WINUSB_WCIDDescriptor,
    .compat_id_len = sizeof(WINUSB_WCIDDescriptor),
    .comp_id_property = (uint8_t *)WINUSB_IF0_WCIDProperties,
    .comp_id_property_len = sizeof(WINUSB_IF0_WCIDProperties),
};

static const uint8_t daplink_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    /* Configuration 0 */
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x03, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    /* Interface 0 */
    USB_INTERFACE_DESCRIPTOR_INIT(0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02),
    /* Endpoint OUT 1 */
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_IN_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
    /* Endpoint IN 2 */
    USB_ENDPOINT_DESCRIPTOR_INIT(DAP_OUT_EP, USB_ENDPOINT_TYPE_BULK, DAP_PACKET_SIZE, 0x00),
    CDC_ACM_DESCRIPTOR_INIT(0x01, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 0x00),
    /* String 0 (LANGID) */
    USB_LANGID_INIT(USBD_LANGID_STRING),
    /* String 1 (Manufacturer) */
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    /* String 2 (Product) */
    0x28,                       // bLength
    USB_DESCRIPTOR_TYPE_STRING, // bDescriptorType
    'C', 0x00,                  // wcChar0
    'h', 0x00,                  // wcChar1
    'e', 0x00,                  // wcChar2
    'r', 0x00,                  // wcChar3
    'r', 0x00,                  // wcChar4
    'y', 0x00,                  // wcChar5
    'U', 0x00,                  // wcChar6
    'S', 0x00,                  // wcChar7
    'B', 0x00,                  // wcChar8
    ' ', 0x00,                  // wcChar9
    'C', 0x00,                  // wcChar10
    'M', 0x00,                  // wcChar11
    'S', 0x00,                  // wcChar12
    'I', 0x00,                  // wcChar13
    'S', 0x00,                  // wcChar14
    '-', 0x00,                  // wcChar15
    'D', 0x00,                  // wcChar16
    'A', 0x00,                  // wcChar17
    'P', 0x00,                  // wcChar18
    /* String 3 (Serial Number) */
    0x1A,                       // bLength
    USB_DESCRIPTOR_TYPE_STRING, // bDescriptorType
    '0', 0,                     // wcChar0
    '1', 0,                     // wcChar1
    '2', 0,                     // wcChar2
    '3', 0,                     // wcChar3
    '4', 0,                     // wcChar4
    '5', 0,                     // wcChar5
    'A', 0,                     // wcChar6
    'B', 0,                     // wcChar7
    'C', 0,                     // wcChar8
    'D', 0,                     // wcChar9
    'E', 0,                     // wcChar10
    'F', 0,                     // wcChar11
#ifdef CONFIG_USB_HS
    /* Device Qualifier */
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x10,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    /* End */
    0x00
};

static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Request[DAP_PACKET_SIZE];
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Response[DAP_PACKET_SIZE];
#endif

#ifdef USBD_USING_CDC
static const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x1C,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'C', 0x00,                  /* wcChar10 */
    'D', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x02,
    0x02,
    0x01,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};
#endif

#if defined(USBD_USING_CDC) || defined(USBD_USING_CMSIS_DAP)
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t cdc_read_buffer[CDC_MAX_MPS];
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t cdc_write_buffer[CDC_MAX_MPS];
#endif

#if defined(USBD_USING_CDC) || defined(USBD_USING_CMSIS_DAP)
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
    uint8_t mode;//1,input;0,output
};

static rt_device_t  cdc_uart = NULL;
static uint8_t      is_cdc_open = 0;
static rt_mq_t      cdc_mq = NULL;
static struct cdc_line_coding cdc_uart_config;

static struct usbd_interface cdc_interface[2];
static volatile bool cdc_ep_tx_busy_flag = false;
#endif

#ifdef USBD_USING_CMSIS_DAP
static struct usbd_interface dap_interface;
static volatile bool dap_ep_tx_busy_flag = false;
#endif



void usbd_configure_done_callback(void)
{
    /* setup first out ep read transfer */
#if defined(USBD_USING_CDC) || defined(USBD_USING_CMSIS_DAP)
    usbd_ep_start_read(CDC_OUT_EP, cdc_read_buffer, CDC_MAX_MPS);
#endif

#ifdef USBD_USING_CMSIS_DAP
    usbd_ep_start_read(DAP_OUT_EP, USB_Request, DAP_PACKET_SIZE);
#endif
}

#if defined(USBD_USING_CDC) || defined(USBD_USING_CMSIS_DAP)
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_err_t result;

    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;
    msg.mode = 1;

    result = rt_mq_send(cdc_mq, &msg, sizeof(msg));
    return result;
}

static void cdc_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_size_t rx_mps = 1;
    rt_size_t rx_length = 0;

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        /* 从消息队列中读取消息 */
        result = rt_mq_recv(cdc_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            if (msg.mode == 1)
            {
                while(rx_mps > 0)
                {
                    /* 从串口读取数据 */
                    rx_mps = rt_device_read(cdc_uart, 0, cdc_write_buffer, CDC_MAX_MPS);
                    if (rx_mps > 0)
                    {
                        cdc_ep_tx_busy_flag = true;
                        usbd_ep_start_write(CDC_IN_EP, cdc_write_buffer, rx_mps);
                        while(cdc_ep_tx_busy_flag);
                    }
                    rx_length += rx_mps;
                }
                rx_mps = 1;
                rx_length = 0;
            }
            else if (msg.mode == 0)
            {
                uint32_t n = msg.size;
                while (n > CDC_MAX_MPS)
                {
                    /* setup next out ep read transfer */
                    usbd_ep_start_read(CDC_OUT_EP, cdc_read_buffer, CDC_MAX_MPS);
                    rt_device_write(cdc_uart, 0, cdc_read_buffer, CDC_MAX_MPS);
                    n -= CDC_MAX_MPS;
                }
                usbd_ep_start_read(CDC_OUT_EP, cdc_read_buffer, n);
                rt_device_write(cdc_uart, 0, cdc_read_buffer, n);
            }
        }
    }
}

static void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    struct rx_msg msg;
    msg.dev = cdc_uart;
    msg.size = nbytes;
    msg.mode = 0;

    rt_mq_send(cdc_mq, &msg, sizeof(msg));
}

static void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
    if ((nbytes % CDC_MAX_MPS) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(CDC_IN_EP, NULL, 0);
    } else {
        cdc_ep_tx_busy_flag = false;
    }
}

void usbd_cdc_acm_get_line_coding(uint8_t intf, struct cdc_line_coding *line_coding)
{
    *line_coding = cdc_uart_config;
}

void usbd_cdc_acm_set_line_coding(uint8_t intf, struct cdc_line_coding *line_coding)
{
    cdc_uart_config = *line_coding;
    if (line_coding->dwDTERate > 0 && line_coding->bDataBits > 4)
    {
        struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
        config.baud_rate = cdc_uart_config.dwDTERate;       // 修改波特率
        config.data_bits = cdc_uart_config.bDataBits;       // 数据位 8
        config.stop_bits = cdc_uart_config.bCharFormat;     // 停止位 1
        config.parity    = cdc_uart_config.bParityType;     // 无奇偶校验位
#ifdef RT_USING_SERIAL_V2
        config.rx_bufsz  = CDC_MAX_MPS;
        config.tx_bufsz  = CDC_MAX_MPS;
#endif
#ifdef RT_USING_SERIAL_V1
        config.bufsz     = CDC_MAX_MPS;
#endif
        if (rt_device_control(cdc_uart, RT_DEVICE_CTRL_CONFIG, &config) != RT_EOK)
        {
            LOG_E("cdc control error");
        }
        /* 打开串口 */
#ifdef RT_USING_SERIAL_V2
        if (rt_device_open(cdc_uart, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING) != RT_EOK)
#endif
#ifdef RT_USING_SERIAL_V1
        if (rt_device_open(cdc_uart, RT_DEVICE_FLAG_DMA_RX) != RT_EOK)
#endif
        {
            LOG_E("cdc uart open error!");
        }
        /* 设置接收回调函数 */
        rt_device_set_rx_indicate(cdc_uart, uart_input);

        is_cdc_open = 1;
//        LOG_D("cdc open %d %d %d %d", config.baud_rate, config.data_bits, config.stop_bits, config.parity);
    }
    else
    {
        rt_device_close(cdc_uart);

        is_cdc_open = 0;
//        LOG_D("cdc close");
    }
}

static struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

static struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};
#endif

#ifdef USBD_USING_CMSIS_DAP
static void usb_dap_recv_callback(uint8_t ep, uint32_t nbytes)
{
    LOG_D("dap recv: %d", nbytes);
    uint32_t actual_len;
    if (USB_Request[0] == ID_DAP_TransferAbort) {
        usbd_ep_start_read(ep, USB_Request, DAP_PACKET_SIZE);
        DAP_TransferAbort = 1;
        return;
    }
    actual_len = DAP_ProcessCommand(USB_Request, USB_Response);

    usbd_ep_start_write(DAP_IN_EP, USB_Response, actual_len);
    usbd_ep_start_read(ep, USB_Request, DAP_PACKET_SIZE);
}

static void usb_dap_send_callback(uint8_t ep, uint32_t nbytes)
{
    LOG_D("dap send: %d", nbytes);
    if ((nbytes % DAP_PACKET_SIZE) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(DAP_IN_EP, NULL, 0);
    } else {
        dap_ep_tx_busy_flag = false;
    }
}

static struct usbd_endpoint dap_out_ep = {
    .ep_addr = DAP_OUT_EP,
    .ep_cb = usb_dap_recv_callback
};
static struct usbd_endpoint dap_in_ep = {
    .ep_addr = DAP_IN_EP,
    .ep_cb = usb_dap_send_callback
};

#endif

static int cherryusb_device_init(void)
{
#ifdef USBD_USING_CDC
    usbd_desc_register(cdc_descriptor);
#elif defined(USBD_USING_CMSIS_DAP)
    DAP_Setup();

    usbd_desc_register(daplink_descriptor);
    usbd_msosv1_desc_register(&msosv1_desc);
#endif

#ifdef USBD_USING_CMSIS_DAP
    /*!< winusb */
    usbd_add_interface(&dap_interface);
    usbd_add_endpoint(&dap_out_ep);
    usbd_add_endpoint(&dap_in_ep);
#endif

#if defined(USBD_USING_CDC) || defined(USBD_USING_CMSIS_DAP)
    cdc_uart = rt_device_find(CDC_UART_DEV);
    if (cdc_uart == NULL)
    {
        LOG_E("cdc device %s not find!", CDC_UART_DEV);
    }
    /* 初始化消息队列 */
    cdc_mq = rt_mq_create("cdcmq", sizeof(struct rx_msg), 16, RT_IPC_FLAG_FIFO);
    /* 创建 serial 线程 */
    rt_thread_t thread = rt_thread_create("cdc", cdc_thread_entry, NULL, 512, 10, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

    /*!< cdc acm */
    usbd_add_interface(usbd_cdc_acm_init_intf(&cdc_interface[0]));
    usbd_add_interface(usbd_cdc_acm_init_intf(&cdc_interface[1]));
    usbd_add_endpoint(&cdc_out_ep);
    usbd_add_endpoint(&cdc_in_ep);
#endif

    usbd_initialize();

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_ENV_EXPORT(cherryusb_device_init);
