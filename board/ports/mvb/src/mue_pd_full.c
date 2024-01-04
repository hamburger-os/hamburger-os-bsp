/* ==========================================================================
 *
 *  File      : MUE_PD_FULL.C
 *
 *  Purpose   : MVB UART Emulation - Process Data Interface (PD_FULL)
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
#include "mue_pd_full.h"
/* ==========================================================================
 *
 *  Public Constants
 *
 * ==========================================================================
 */

/* --------------------------------------------------------------------------
 *  Miscellaneous
 * --------------------------------------------------------------------------
 */
TCN_DECL_CONST \
    WORD16    mue_pd_full_port_number_max = MUE_PD_FULL_PORT_NUMBER_MAX;

TCN_DECL_CONST \
    UNSIGNED8 mue_pd_full_port_size_max   = MUE_PD_FULL_PORT_SIZE_MAX;
/* --------------------------------------------------------------------------
 *  Constants : mue_pd_full_port_config_xxx
 *
 *  Purpose   : MVB process data port configuration.
 *
 *  Remarks   : - See also constants 'MUE_PD_FULL_PORT_CONFIG_xxx'.
 *              - Used for parameter 'port_config' of procedure
 *                'mue_pd_full_put_port_config'.
 *              - Used for parameter 'p_port_config' of procedure
 *                'mue_pd_full_get_port_config'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_src     = \
                            MUE_PD_FULL_PORT_CONFIG_SRC;
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_fc_mask = \
                            MUE_PD_FULL_PORT_CONFIG_FC_MASK;
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_fc4     = \
                            MUE_PD_FULL_PORT_CONFIG_FC4;
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_fc3     = \
                            MUE_PD_FULL_PORT_CONFIG_FC3;
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_fc2     = \
                            MUE_PD_FULL_PORT_CONFIG_FC2;
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_fc1     = \
                            MUE_PD_FULL_PORT_CONFIG_FC1;
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_fc0     = \
                            MUE_PD_FULL_PORT_CONFIG_FC0;
TCN_DECL_CONST BITSET16 mue_pd_full_port_config_la_mask = \
                            MUE_PD_FULL_PORT_CONFIG_LA_MASK;
/* --------------------------------------------------------------------------
 *  Constants : mue_pd_full_port_status_xxx
 *
 *  Purpose   : Status information related to a MVB process data port.
 *
 *  Remarks   : - See also constants 'MUE_PD_FULL_PORT_STATUS_xxx'.
 *              - Used for parameter 'port_status' of procedure
 *                'mue_pd_full_get_port_data'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_CONST BITSET8  mue_pd_full_port_status_sink_mask = \
                            MUE_PD_FULL_PORT_STATUS_SINK_MASK;
TCN_DECL_CONST BITSET8  mue_pd_full_port_status_sink_a    = \
                            MUE_PD_FULL_PORT_STATUS_SINK_A;
TCN_DECL_CONST BITSET8  mue_pd_full_port_status_sink_b    = \
                            MUE_PD_FULL_PORT_STATUS_SINK_B;
/* --------------------------------------------------------------------------
 *  Constants : mue_pd_command_xxx
 *
 *  Purpose   : Process data commands.
 * --------------------------------------------------------------------------
 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_command_put_port_config = 'H'; /* 0x48 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_command_get_port_config = 'I'; /* 0x49 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_command_put_port_data   = 'P'; /* 0x50 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_command_get_port_data   = 'G'; /* 0x47 */

TCN_DECL_LOCAL TCN_DECL_CONST \
WORD8 mli_cmd_word_put_other_config = 'K'; /* 0x4B */
/* --------------------------------------------------------------------------
 *  Constants : mue_pd_full_size_xxx
 *
 *  Purpose   : Process data command sizes (PD_FULL).
 * --------------------------------------------------------------------------
 */
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_tx_put_port_config = 3;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_rx_put_port_config = 0;

TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_tx_get_port_config = 3;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_rx_get_port_config = 2;

TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_tx_put_port_data   = 35;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_rx_put_port_data   = 0;

TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_tx_get_port_data   = 3;
TCN_DECL_LOCAL TCN_DECL_CONST \
    WORD8 mue_pd_full_size_rx_get_port_data   = 35;
/* --------------------------------------------------------------------------
 *  Procedure : mue_pd_full_put_port_config
 *
 *  Purpose   : Puts configuration related to a MVB process data port to the
 *              traffic store of a MVB controller (PD_FULL).
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_pd_full_put_port_config
 *              (
 *                  BITSET16    port_config
 *              );
 *
 *  Input     : p_bus_ctrl  - pointer to bus controller specific values
 *              port_config - port configuration; any combination of
 *                            constants 'MUE_PD_FULL_PORT_CONFIG_xxx' or
 *                            constants 'mue_pd_full_port_config_xxx'
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - See MVB UART Emulation command 'H'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 *              - A MVB process data port is identified by its logical
 *                address, which is part of parameter 'port_config'
 *                (see also constant 'MUE_PD_FULL_PORT_CONFIG_LA_MASK').
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_pd_full_put_port_config(BITSET16 port_config)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value8(mue_pd_command_put_port_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value16(port_config);
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
        mue_result = mue_acc_rx_wait(mue_pd_full_size_rx_put_port_config);
    } /* if (MUE_RESULT_OK == mue_result) */
//    if (MUE_RESULT_OK == mue_result)
//    {
//        mue_result = mue_acc_rx_close();
//    } /* if (MUE_RESULT_OK == mue_result) */

    return(mue_result);

} /* mue_pd_full_put_port_config */


MUE_RESULT
TEST_cmd_put_other_config
(
    BITSET8     mix_config,
    BITSET16    t_reply,
    BITSET16    t_ignore,
    BITSET16    t_ms
)
{
    MUE_RESULT  mli_result;

    // TX
    mli_result = mue_acc_tx_start();
    if (MUE_RESULT_OK == mli_result)
    {
        mli_result = mue_acc_tx_value8(mli_cmd_word_put_other_config);
    }
    if (MUE_RESULT_OK == mli_result)
    {
        mli_result = mue_acc_tx_value8(mix_config);
    }
    if (MUE_RESULT_OK == mli_result)
    {
        mli_result = mue_acc_tx_value16(t_reply);
    }
    if (MUE_RESULT_OK == mli_result)
    {
        mli_result = mue_acc_tx_value16(t_ignore);
    }
    if (MUE_RESULT_OK == mli_result)
    {
        mli_result = mue_acc_tx_value16(t_ms);
    }
		
    if (MUE_RESULT_OK == mli_result)
    {
        mli_result = mue_acc_tx_trigger();
    } /* if (MUE_RESULT_OK == mue_result) */

    // RX : nothing to receive 

    return(mli_result);

}
/* --------------------------------------------------------------------------
 *  Procedure : mue_pd_full_get_port_config
 *
 *  Purpose   : Gets configuration related to a MVB process data port
 *              back from the traffic store of a MVB controller (PD_FULL).
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_pd_full_get_port_config
 *              (
 *                  WORD16      port_address,
 *                  BITSET16    *p_port_config
 *              );
 *
 *  Input     : p_bus_ctrl    - pointer to bus controller specific values
 *              port_address  - port identifier (12-bit logical address)
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Output    : p_port_config - pointer to buffer that receives the port
 *                              configuration; any combination of
 *                              constants 'MUE_PD_FULL_PORT_CONFIG_xxx' or
 *                              constants 'mue_pd_full_port_config_xxx'
 *
 *  Remarks   : - See MVB UART Emulation command 'I'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 *              - A MVB process data port is identified by its logical
 *                address (see parameter 'port_address').
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_pd_full_get_port_config(WORD16 port_address, BITSET16 *p_port_config)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value8(mue_pd_command_get_port_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value16(port_address);
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
        mue_result = mue_acc_rx_wait(mue_pd_full_size_rx_get_port_config);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_rx_value16(p_port_config);
    } /* if (MUE_RESULT_OK == mue_result) */
//    if (MUE_RESULT_OK == mue_result)
//    {
//        mue_result = mue_acc_rx_close();
//    } /* if (MUE_RESULT_OK == mue_result) */

    return(mue_result);

} /* mue_pd_full_get_port_config */


/* --------------------------------------------------------------------------
 *  Procedure : mue_pd_full_put_port_data
 *
 *  Purpose   : Puts data related to a MVB process data port to the traffic
 *              store of a MVB controller (PD_FULL).
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_pd_full_put_port_data
 *              (
 *                  WORD16      port_address,
 *                  UNSIGNED8   port_size,
 *                  void        *p_port_data
 *              );
 *
 *  Input     : p_bus_ctrl   - pointer to bus controller specific values
 *              port_address - port identifier (12-bit logical address)
 *              port_size    - size of the port (2, 4, 8, 16 or 32 bytes)
 *              p_port_data  - pointer to buffer that contains the data
 *                             related to a MVB process data port;
 *                             up to 32 bytes, which is the largest size
 *                             of a port
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - See MVB UART Emulation command 'P'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 *              - A MVB process data port is identified by its logical
 *                address (see parameter 'port_address').
 *              - The size of a MVB process data port may have 2, 4, 8, 16
 *                or 32 bytes (see parameter 'p_port_data').
 *              - The MVB UART Emulation handles the data of a MVB process
 *                data port (parameter 'p_port_data') as an array of WORD16.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_pd_full_put_port_data(WORD16 port_address, UNSIGNED8 port_size, void *p_port_data)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value8(mue_pd_command_put_port_data);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value16(port_address);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_array(port_size, p_port_data);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_fill((UNSIGNED16)(32-port_size));
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
        mue_result = mue_acc_rx_wait( mue_pd_full_size_rx_put_port_data);
    } /* if (MUE_RESULT_OK == mue_result) */
//    if (MUE_RESULT_OK == mue_result)
//    {
//        mue_result = mue_acc_rx_close();
//    } /* if (MUE_RESULT_OK == mue_result) */

    return(mue_result);

}


/* --------------------------------------------------------------------------
 *  Procedure : mue_pd_full_get_port_data
 *
 *  Purpose   : Gets data related to a MVB process data port from the
 *              traffic store of a MVB controller (PD_FULL).
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_pd_full_get_port_data
 *              (
 *                  WORD16      port_address,
 *                  BOOLEAN1    check_received,
 *                  UNSIGNED8   port_size,
 *                  BITSET8     *p_port_status,
 *                  void        *p_port_data,
 *                  UNSIGNED16  *p_port_freshness
 *              );
 *
 *  Input     : p_bus_ctrl       - pointer to bus controller specific values
 *              port_address     - port identifier (12-bit logical address)
 *              check_received   - check if new data was received from MVB
 *                                 since last get (TRUE or FALSE)
 *              port_size        - size of the port (2, 4, 8, 16 or 32 bytes)
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Output    : p_port_status    - pointer to buffer that receives the
 *                                 status information releated to a MVB
 *                                 process data port;
 *                                 any combination of
 *                                 constants 'MUE_PD_FULL_PORT_STATUS_xxx' or
 *                                 constants 'mue_pd_full_port_status_xxx'
 *              p_port_data      - pointer to buffer that receives the data
 *                                 related to a MVB process data port;
 *                                 up to 32 bytes, which is the largest size
 *                                 of a port
 *              p_port_freshness - pointer to buffer that receives the
 *                                 freshness timer related to a MVB process
 *                                 data port (or NULL)
 *
 *  Remarks   : - See MVB UART Emulation command 'G'.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 *              - A MVB process data port is identified by its logical
 *                address (see parameter 'port_address').
 *              - If parameter 'check_received' is FALSE, then the
 *                parameters 'p_port_data' and 'p_port_freshness'
 *                will return always a value.
 *                Otherwise it depends on the value of parameter
 *                'p_port_status' (bits SINK_A, SINK_B).
 *              - The size of a MVB process data port may have 2, 4, 8, 16
 *                or 32 bytes (see parameter 'p_port_data').
 *              - The MVB UART Emulation handles the data of a MVB process
 *                data port (parameter 'p_port_data') as an array of WORD16.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_pd_full_get_port_data
(
    WORD16      port_address,
    BOOLEAN1    check_received,
    UNSIGNED8   port_size,
    BITSET8     *p_port_status,
    void        *p_port_data,
    UNSIGNED16  *p_port_freshness
)
{
    MUE_RESULT  mue_result;

    /* ----------------------------------------------------------------------
     *  TX
     * ----------------------------------------------------------------------
     */
    mue_result = mue_acc_tx_start();
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value8(mue_pd_command_get_port_data);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_tx_value16(port_address);
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
        mue_result = mue_acc_rx_wait(mue_pd_full_size_rx_get_port_data);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        mue_result = mue_acc_rx_value8(p_port_status);
    } /* if (MUE_RESULT_OK == mue_result) */
    if (MUE_RESULT_OK == mue_result)
    {
        if ((TRUE == check_received) && (0x00 == (*p_port_status & mue_pd_full_port_status_sink_mask)))
        {
//            mue_result = mue_acc_rx_ignore();
        } /* if ((TRUE == check_received) && (...)) */
        else
        {
            mue_result = mue_acc_rx_array(port_size, p_port_data);
            if (MUE_RESULT_OK == mue_result)
            {
                if (NULL != p_port_freshness)
                {
                    mue_result = mue_acc_rx_fill((UNSIGNED16)(32-port_size));
                    if (MUE_RESULT_OK == mue_result)
                    {
                        mue_result = mue_acc_rx_value16(p_port_freshness);
                    } /* if (MUE_RESULT_OK == mue_result) */
                } /* if (NULL != p_port_freshness) */
//                else
//                {
//                    mue_result = mue_acc_rx_ignore();
//                } /* else */
            } /* if (MUE_RESULT_OK == mue_result) */
        } /* else */
    } /* if (MUE_RESULT_OK == mue_result) */
//    if (MUE_RESULT_OK == mue_result)
//    {
//        mue_result = mue_acc_rx_close();
//    } /* if (MUE_RESULT_OK == mue_result) */

    return(mue_result);

} /* mue_pd_full_put_port_data */
