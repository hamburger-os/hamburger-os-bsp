#ifndef MUE_SV_H
#define MUE_SV_H

/* ==========================================================================
 *
 *  File      : MUE_SV.H
 *
 *  Purpose   : MVB UART Emulation - Supervision Interface (SV)
 *
 *  Project   : General TCN Driver Software
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
 * ==========================================================================
 */


/* ==========================================================================
 *
 *  Project specific Definitions used for Conditional Compiling
 *
 * ==========================================================================
 */
#ifdef TCN_PRJ
#   include "tcn_prj.h"
#endif


/* ==========================================================================
 *
 *  Include Files
 *
 * ==========================================================================
 */

/* --------------------------------------------------------------------------
 *  TCN Software Architecture
 * --------------------------------------------------------------------------
 */
#include "tcn_def.h"

/* --------------------------------------------------------------------------
 *  MVB UART Emulation
 * --------------------------------------------------------------------------
 */
#include "mue_def.h"


/* ==========================================================================
 *
 *  Constants
 *
 * ==========================================================================
 */

/* --------------------------------------------------------------------------
 *  Miscellaneous
 * --------------------------------------------------------------------------
 */
#define MUE_SV_DEVICE_NUMBER_MAX        4096
TCN_DECL_PUBLIC TCN_DECL_CONST WORD16   mue_sv_device_number_max;


/* --------------------------------------------------------------------------
 *  Constants : MUE_SV_DEVICE_CONFIG_xxx, mue_sv_device_config_xxx
 *
 *  Purpose   : MVB device configuration.
 *
 *  Remarks   : - Used for parameter 'device_config' of procedure
 *                'mue_sv_put_device_config'.
 *              - Used for parameter 'p_device_config' of procedure
 *                'mue_sv_get_device_config'.
 * --------------------------------------------------------------------------
 */
#define MUE_SV_DEVICE_CONFIG_LINE_MASK  0xC000 /* mask for MVB line bits   */
#define MUE_SV_DEVICE_CONFIG_LINE_A     0x8000 /* MVB line A               */
#define MUE_SV_DEVICE_CONFIG_LINE_B     0x4000 /* MVB line B               */
#define MUE_SV_DEVICE_CONFIG_BA_EN      0x2000 /* bus administrator (opt.) */
#define MUE_SV_DEVICE_CONFIG_LED_DNR    0x1000 /* LED (influence on DNR)   */
#define MUE_SV_DEVICE_CONFIG_DA_MASK    0x0FFF /* mask for device address  */

TCN_DECL_PUBLIC TCN_DECL_CONST BITSET16 mue_sv_device_config_line_mask;
TCN_DECL_PUBLIC TCN_DECL_CONST BITSET16 mue_sv_device_config_line_a;
TCN_DECL_PUBLIC TCN_DECL_CONST BITSET16 mue_sv_device_config_line_b;
TCN_DECL_PUBLIC TCN_DECL_CONST BITSET16 mue_sv_device_config_ba_en;
TCN_DECL_PUBLIC TCN_DECL_CONST BITSET16 mue_sv_device_config_led_dnr;
TCN_DECL_PUBLIC TCN_DECL_CONST BITSET16 mue_sv_device_config_da_mask;


/* --------------------------------------------------------------------------
 *  Constants : MUE_SV_PORT_STATUS_xxx, mue_sv_port_status_xxx
 *
 *  Purpose   : Status information related to a MVB device status port.
 *
 *  Remarks   : - Used for parameter 'p_port_status' of procedure
 *                'mue_sv_get_device_status'.
 * --------------------------------------------------------------------------
 */
#define MUE_SV_PORT_STATUS_SINK_MASK    0xC0 /* mask for sink bits         */
#define MUE_SV_PORT_STATUS_SINK_A       0x80 /* received on MVB line A     */
#define MUE_SV_PORT_STATUS_SINK_B       0x40 /* received on MVB line B     */

TCN_DECL_PUBLIC TCN_DECL_CONST BITSET8  mue_sv_port_status_sink_mask;
TCN_DECL_PUBLIC TCN_DECL_CONST BITSET8  mue_sv_port_status_sink_a;
TCN_DECL_PUBLIC TCN_DECL_CONST BITSET8  mue_sv_port_status_sink_b;


/* --------------------------------------------------------------------------
 *  Bit Constants : MUE_SV_DEVICE_STATUS_BIT_xxx
 *
 *  Purpose       : MVB device status.
 *
 *  Remarks       : - See 'Device_Status_Response' in IEC 61375-1.
 *                  - Used for parameter 'p_device_status' of procedure
 *                    'mue_sv_get_device_status'.
 * --------------------------------------------------------------------------
 */

/* --------------------------------------------------------------------------
 *  capabilities
 * --------------------------------------------------------------------------
 */
#define MUE_SV_DEVICE_STATUS_BIT_SP     0x8000 /* special device           */
#define MUE_SV_DEVICE_STATUS_BIT_BA     0x4000 /* bus administrator cap.   */
#define MUE_SV_DEVICE_STATUS_BIT_GW     0x2000 /* gateway capability       */
#define MUE_SV_DEVICE_STATUS_BIT_MD     0x1000 /* message data capability  */

/* --------------------------------------------------------------------------
 *  class specific (generic)
 * --------------------------------------------------------------------------
 */
#define MUE_SV_DEVICE_STATUS_BIT_CS0    0x0800
#define MUE_SV_DEVICE_STATUS_BIT_CS1    0x0400
#define MUE_SV_DEVICE_STATUS_BIT_CS2    0x0200
#define MUE_SV_DEVICE_STATUS_BIT_CS3    0x0100

/* --------------------------------------------------------------------------
 *  class specific (bus administrator)
 * --------------------------------------------------------------------------
 */
#define MUE_SV_DEVICE_STATUS_BIT_AX1    0x0800 /* acutalisation key        */
#define MUE_SV_DEVICE_STATUS_BIT_AX0    0x0400 /* actualisation key (LSB)  */
#define MUE_SV_DEVICE_STATUS_BIT_ACT    0x0200 /* device is actualised     */
#define MUE_SV_DEVICE_STATUS_BIT_MAS    0x0100 /* device is current master */

/* --------------------------------------------------------------------------
 *  class specific (gateway without bus administrator capability)
 * --------------------------------------------------------------------------
 */
#define MUE_SV_DEVICE_STATUS_BIT_STD    0x0800 /* static  disturbance      */
#define MUE_SV_DEVICE_STATUS_BIT_DYD    0x0400 /* dynamic disturbance      */

/* --------------------------------------------------------------------------
 *  common flags
 * --------------------------------------------------------------------------
 */
#define MUE_SV_DEVICE_STATUS_BIT_LAT    0x0080 /* line A trusted           */
#define MUE_SV_DEVICE_STATUS_BIT_RLD    0x0040 /* redundant line disturbed */
#define MUE_SV_DEVICE_STATUS_BIT_SSD    0x0020 /* some  system disturbance */
#define MUE_SV_DEVICE_STATUS_BIT_SDD    0x0010 /* somde device disturbance */
#define MUE_SV_DEVICE_STATUS_BIT_ERD    0x0008 /* extended reply timeout   */
#define MUE_SV_DEVICE_STATUS_BIT_FRC    0x0004 /* forced device            */
#define MUE_SV_DEVICE_STATUS_BIT_DNR    0x0002 /* device not ready         */
#define MUE_SV_DEVICE_STATUS_BIT_SER    0x0001 /* system reserved          */


/* ==========================================================================
 *
 *  Procedures
 *
 * ==========================================================================
 */

/* --------------------------------------------------------------------------
 *  Procedure : mue_sv_put_device_config
 *
 *  Purpose   : Puts MVB device related configuration to a MVB controller.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_sv_put_device_config
 *              (
 *                  BITSET16    device_config
 *              );
 *
 *  Input     : p_bus_ctrl    - pointer to bus controller specific values
 *              device_config - device configuration; any combination
 *                              of constants 'MUE_SV_DEVICE_CONFIG_xxx'
 *                              or constants 'mue_sv_device_config_xxx'
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - See MVB UART Emulation command 'C'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 *              - If the MVB interface shall be switched off for special
 *                purposes, the following sequence must be obeyed in order
 *                not to transmit undefined data to the MVB:
 *                1. Switch off Line_A and Line_B without changing device
 *                   address.
 *                2. Second, wait approximately 250ms (= time for longest
 *                   MVB telegram) until a potential transmission from the
 *                   MVB interface to MVB has finished.
 *                3. Now the device address and source port data may be
 *                   changed (and these changes will not go to the MVB!).
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_sv_put_device_config(BITSET16 device_config);


/* --------------------------------------------------------------------------
 *  Procedure : mue_sv_get_device_config
 *
 *  Purpose   : Gets MVB device related configuration back from a MVB
 *              controller.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_sv_get_device_config
 *              (
 *                  BITSET16    *p_device_config
 *              );
 *
 *  Input     : p_bus_ctrl      - pointer to bus controller specific values
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Output    : p_device_config - pointer to buffer that receives the device
 *                                configuration; any combination of
 *                                constants 'MUE_SV_DEVICE_CONFIG_xxx' or
 *                                constants 'mue_sv_device_config_xxx'
 *
 *  Remarks   : - See MVB UART Emulation command 'S'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_sv_get_device_config(BITSET16 *p_device_config);


/* --------------------------------------------------------------------------
 *  Procedure : mue_sv_put_ba_config
 *
 *  Purpose   : Puts one list entry of a MVB bus administrator configuration
 *              to a MVB controller.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_sv_put_ba_config
 *              (
 *                  WORD16      ba_list_entry
 *              );
 *
 *  Input     : p_bus_ctrl    - pointer to bus controller specific values
 *              ba_list_entry - one list entry of a bus administrator
 *                              configuration
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - See MVB UART Emulation command 'A'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 *              - This is an optional procedure not supported by all MVB
 *                controllers.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_sv_put_ba_config(WORD16 ba_list_entry);


/* --------------------------------------------------------------------------
 *  Procedure : mue_sv_get_device_status
 *
 *  Purpose   : Gets the device status related to a MVB device from the
 *              traffic store of a MVB controller.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_sv_get_device_status
 *              (
 *                  WORD16      device_address,
 *                  BOOLEAN1    check_received,
 *                  BITSET8     *p_port_status,
 *                  BITSET16    *p_device_status,
 *                  UNSIGNED16  *p_freshness
 *              );
 *
 *  Input     : p_bus_ctrl      - pointer to bus controller specific values
 *              device_address  - device identifier (12-bit device address)
 *              check_received  - check if new data was received from MVB
 *                                since last get (TRUE or FALSE)
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Output    : p_port_status   - pointer to buffer that receives the
 *                                status information releated to a MVB
 *                                device status port;
 *                                any combination of constants
 *                                'MUE_SV_PORT_STATUS_xxx' or constants
 *                                'mue_sv_port_status_xxx'
 *              p_device_status - pointer to buffer that receives the data
 *                                related to a MVB device status port;
 *                                any combination of bit constants
 *                                'MUE_SV_DEVICE_STATUS_BIT_xxx'
 *              p_freshness     - pointer to buffer that receives the
 *                                freshness timer related to a MVB device
 *                                status port (or NULL)
 *
 *  Remarks   : - See MVB UART Emulation command 'D'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 *              - If parameter 'check_received' is FALSE, then the
 *                parameters 'p_device_status' (and 'p_freshness') will
 *                return always a value. Otherwise it depends on the value
 *                of parameter 'p_port_status' (bits SINK_A, SINK_B).
 *              - This is an optional procedure supported only by MVB
 *                controllers with MVB UART Emulation 'MDFULL'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_sv_get_device_status
(
    WORD16      device_address,
    BOOLEAN1    check_received,
    BITSET8     *p_port_status,
    BITSET16    *p_device_status,
    UNSIGNED16  *p_freshness
);

#endif /* #ifndef MUE_SV_H */
