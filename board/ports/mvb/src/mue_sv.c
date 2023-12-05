/* ==========================================================================
 *
 *  File      : MUE_SV.C
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
#include "tcn_def.h"
#include "mue_def.h"
#include "mue_osl.h"
#include "mue_acc.h"
#include "mue_sv.h"
/* --------------------------------------------------------------------------
 *  Miscellaneous
 * --------------------------------------------------------------------------
 */
TCN_DECL_CONST \
    WORD16 mue_sv_device_number_max = MUE_SV_DEVICE_NUMBER_MAX;
/* --------------------------------------------------------------------------
 *  Constants : mue_sv_device_config_xxx
 *
 *  Purpose   : MVB device configuration.
 *
 *  Remarks   : - See also constants 'MUE_SV_DEVICE_CONFIG_xxx'.
 *              - Used for parameter 'device_config' of procedure
 *                'mue_sv_put_device_config'.
 *              - Used for parameter 'p_device_config' of procedure
 *                'mue_sv_get_device_config'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_CONST \
    BITSET16 mue_sv_device_config_line_mask = MUE_SV_DEVICE_CONFIG_LINE_MASK;
TCN_DECL_CONST \
    BITSET16 mue_sv_device_config_line_a    = MUE_SV_DEVICE_CONFIG_LINE_A;
TCN_DECL_CONST \
    BITSET16 mue_sv_device_config_line_b    = MUE_SV_DEVICE_CONFIG_LINE_B;
TCN_DECL_CONST \
    BITSET16 mue_sv_device_config_ba_en     = MUE_SV_DEVICE_CONFIG_BA_EN;
TCN_DECL_CONST \
    BITSET16 mue_sv_device_config_led_dnr   = MUE_SV_DEVICE_CONFIG_LED_DNR;
TCN_DECL_CONST \
    BITSET16 mue_sv_device_config_da_mask   = MUE_SV_DEVICE_CONFIG_DA_MASK;
/* --------------------------------------------------------------------------
 *  Constants : mue_sv_port_status_xxx
 *
 *  Purpose   : Status information related to a MVB device status port.
 *
 *  Remarks   : - See also constants 'MUE_SV_PORT_STATUS_xxx'.
 *              - Used for parameter 'p_port_status' of procedure
 *                'mue_sv_get_device_status'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_CONST \
    BITSET8 mue_sv_port_status_sink_mask = MUE_SV_PORT_STATUS_SINK_MASK;
TCN_DECL_CONST \
    BITSET8 mue_sv_port_status_sink_a    = MUE_SV_PORT_STATUS_SINK_A;
TCN_DECL_CONST \
    BITSET8 mue_sv_port_status_sink_b    = MUE_SV_PORT_STATUS_SINK_B;
/* ==========================================================================
 *
 *  Local Constants
 *
 * ==========================================================================
 */
/* --------------------------------------------------------------------------
 *  Constants : mue_sv_command_xxx
 *
 *  Purpose   : Supervision commands.
 * --------------------------------------------------------------------------
 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_command_put_device_config = 'C'; /* 0x43 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_command_get_device_config = 'S'; /* 0x53 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_command_put_ba_config     = 'A'; /* 0x41 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_command_get_device_status = 'D'; /* 0x44 */

//TCN_DECL_LOCAL TCN_DECL_CONST \
//WORD8 mli_cmd_word_get_port_config = 'I'; /* 0x49 */
/* --------------------------------------------------------------------------
 *  Constants : mue_sv_size_xxx
 *
 *  Purpose   : Supervision command sizes.
 * --------------------------------------------------------------------------
 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_tx_put_device_config = 3;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_rx_put_device_config = 0;

TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_tx_get_device_config = 1;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_rx_get_device_config = 2;

TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_tx_put_ba_config     = 3;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_rx_put_ba_config     = 0;

TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_tx_get_device_status = 3;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_sv_size_rx_get_device_status = 5;
/* ==========================================================================
 *
 *  Public Procedures
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
TCN_DECL_PUBLIC  MUE_RESULT  mue_sv_put_device_config(BITSET16 device_config)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = \
        mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = \
            mue_acc_tx_value8(mue_sv_command_put_device_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value16(device_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_trigger();
    } /* if (MUE_RESULT_OK == mue_result) */

    /* ----------------------------------------------------------------------
     *  RX
     * ----------------------------------------------------------------------
     */
    /* NOTE: nothing to receive */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = \
            mue_acc_rx_wait(mue_sv_size_rx_put_device_config);
    } /* if (MUE_RESULT_OK == mue_result) */
//    if (MUE_RESULT_OK == mue_result)
//    {
//        mue_result = mue_acc_rx_close();
//    } /* if (MUE_RESULT_OK == mue_result) */

    return(mue_result);

} /* mue_sv_put_device_config */
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
TCN_DECL_PUBLIC  MUE_RESULT  mue_sv_get_device_config(BITSET16 *p_device_config)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = \
        mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = \
            mue_acc_tx_value8( mue_sv_command_get_device_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_trigger();
    } /* if (MUE_RESULT_OK == mue_result) */

    /* ----------------------------------------------------------------------
     *  RX
     * ----------------------------------------------------------------------
     */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = \
            mue_acc_rx_wait(mue_sv_size_rx_get_device_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_rx_value16(p_device_config);
    } /* if (MUE_RESULT_OK == mue_result) */
//    if (MUE_RESULT_OK == mue_result)
//    {
//        mue_result = mue_acc_rx_close();
//    } /* if (MUE_RESULT_OK == mue_result) */

    return(mue_result);

} /* mue_sv_put_device_config */
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
TCN_DECL_PUBLIC  MUE_RESULT  mue_sv_put_ba_config(WORD16 ba_list_entry)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = \
            mue_acc_tx_value8(mue_sv_command_put_ba_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value16( ba_list_entry);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_trigger();
    } /* if (MUE_RESULT_OK == mue_result) */

    /* ----------------------------------------------------------------------
     *  RX
     * ----------------------------------------------------------------------
     */
    /* NOTE: nothing to receive */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = \
            mue_acc_rx_wait( mue_sv_size_rx_put_ba_config);
    } /* if (MUE_RESULT_OK == mue_result) */
//    if (MUE_RESULT_OK == mue_result)
//    {
//        mue_result = mue_acc_rx_close();
//    } /* if (MUE_RESULT_OK == mue_result) */

    return(mue_result);

} /* mue_sv_put_ba_config */


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
)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = \
        mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = \
            mue_acc_tx_value8(mue_sv_command_get_device_status);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value16(device_address);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_trigger();
    } /* if (MUE_RESULT_OK == mue_result) */

    /* ----------------------------------------------------------------------
     *  RX
     * ----------------------------------------------------------------------
     */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_rx_wait(mue_sv_size_rx_get_device_status);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_rx_value8(p_port_status);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        if ((TRUE == check_received) && \
            (0x00 == (*p_port_status & mue_sv_port_status_sink_mask)))
        {
//            mue_result = mue_acc_rx_ignore();
        } /* ((TRUE == check_received) && (...)) */
        else
        {
            mue_result = mue_acc_rx_value16(p_device_status);
            if (MUE_RESULT_OK == mue_result)
            {
                if (NULL != p_freshness)
                {
                    mue_result = mue_acc_rx_value16( p_freshness);
                } 
            } 
        }
    } 
    return(mue_result);

}
