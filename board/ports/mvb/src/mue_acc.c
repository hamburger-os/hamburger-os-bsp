/* ==========================================================================
 *
 *  File      : MUE_ACC.C
 *
 *  Purpose   : MVB UART Emulation - Access Interface
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
 *  Pre-processor Definitions:
 *  --------------------------
 *  - MUE_ACC_HW_LE   - the MVB UART Emulation of the MVB interface board
 *                      is configured to handle words (16-bit values) with
 *                      little-endian number representation; this applies
 *                      to parameters and data of all MVB UART Emulation
 *                      commands
 *                      (default is big-endian number representation)
 *  - MUE_ACC_PRINT   - debug printouts
 *
 * ==========================================================================
 */
#if defined (MUE_ACC_HW_LE) && defined (TCN_LE)
#   undef  MUE_ACC_BYTE_SWAP
#endif
#if defined (MUE_ACC_HW_LE) && !defined (TCN_LE)
#   define MUE_ACC_BYTE_SWAP
#endif
#if !defined (MUE_ACC_HW_LE) && defined (TCN_LE)
#   define MUE_ACC_BYTE_SWAP
#endif
#if !defined (MUE_ACC_HW_LE) && !defined (TCN_LE)
#   undef  MUE_ACC_BYTE_SWAP
#endif

#define MUE_ACC_BYTE_SWAP 1
#define MUE_ACC_HW_LE     1
#include "tcn_def.h"
/* --------------------------------------------------------------------------
 *  MVB UART Emulation
 * --------------------------------------------------------------------------
 */
#include "mue_def.h"
#include "mue_osl.h"
#include "mue_cch.h"
#include "mue_acc.h"
/* --------------------------------------------------------------------------
 *  Data buffer for communication channel (CCH)
 * --------------------------------------------------------------------------
 */
TCN_DECL_LOCAL WORD8        mue_cch_data_buf[35];
TCN_DECL_LOCAL UNSIGNED16   mue_cch_data_cnt;
TCN_DECL_LOCAL WORD8        *mue_cch_data_ptr;
/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_init
 *
 *  Purpose   : Initialises the access to the UART (check for availability).
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_init
 *              (
 *                  void
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_init(void)
{
    MUE_RESULT  mue_result = MUE_RESULT_OK;

    mue_cch_data_cnt = 0;
    mue_cch_data_ptr = (WORD8*)mue_cch_data_buf;

    return(mue_result);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_close
 *
 *  Purpose   : Closes the access to the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_close
 *              (
 *                  void
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_close(void)
{
    MUE_RESULT  mue_result = MUE_RESULT_OK;

    mue_cch_data_cnt = 0;
    mue_cch_data_ptr = (WORD8*)mue_cch_data_buf;

    return(mue_result);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_clean_up
 *
 *  Purpose   : Perform a clean-up of the UART communication
 *              (synchronise access protocol of the MVB UART Emulation).
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_clean_up
 *              (
 *                  void
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - If a process using the MVB UART Emulation was interrupted
 *                the access protocol of the MVB UART Emulation remains in
 *                one of the following three states (Sx):
 *                S1: The MVB UART Emulation is ready to receive a new
 *                    command character -> nothing must be done
 *                    * any transmitted data bytes with value 0x00 (see
 *                      state S2) will be ignored
 *                S2: A MVB UART Emulation command is still in progress and
 *                    requests more data bytes (in minimum the command
 *                    character was sent) -> synchronisation is necessary
 *                    * transmits 34 data bytes with value 0x00 (command 'P'
 *                      requests the max. number of data bytes; PD_16=33,
 *                      PD_16F=33, PD_FULL=34, i.e. 34 data bytes); after
 *                      this the MVB UART Emulation may enter the state S3
 *                S3: A MVB UART Emulation command is still in progress and
 *                    transmits more data bytes -> synchronisation is
 *                    necessary (Parallel Host Interface only)
 *                    * transmit any command character which flushes the
 *                      transmit queue of the MVB UART Emulation
 *                    or (alternative)
 *                    * receive until nothing left (command 'G' transmits
 *                      the max. number of data bytes; PD_16=33, PD_16F=35,
 *                      PD_FULL=35, i.e. 35 data bytes)
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_clean_up(void)
{
    MUE_RESULT  mue_result;

    mue_result = mue_cch_clean_up();

    return(mue_result);
}
/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_tx_start
 *
 *  Purpose   : Starts a transmission over the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_tx_start
 *              (
 *                  void
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              size       - number of bytes to transmit over the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_tx_start(void)
{
    mue_cch_data_cnt = 0;
    mue_cch_data_ptr = (WORD8*)mue_cch_data_buf;

    return(MUE_RESULT_OK);
}
/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_tx_value8
 *
 *  Purpose   : Transmits a 8-bit value over the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_tx_value8
 *              (
 *                  WORD8       value8
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              value8     - 8-bit value to transmit over the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for general parameters.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_tx_value8(WORD8 value8)
{
    mue_cch_data_cnt++;
    *mue_cch_data_ptr++ = value8;

    return(MUE_RESULT_OK);
}
/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_tx_value16
 *
 *  Purpose   : Transmits a 16-bit value over the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_tx_value16
 *              (
 *                  WORD16      value16
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              value16    - 16-bit value to transmit over the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for general parameters.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_tx_value16(WORD16 value16)
{
    WORD8       *p_word8 = (WORD8*)&value16;

#ifdef MUE_ACC_BYTE_SWAP

    /* first low then high byte */
    mue_cch_data_cnt++;
    *mue_cch_data_ptr++ = p_word8[1];
    mue_cch_data_cnt++;
    *mue_cch_data_ptr++ = p_word8[0];

#else /* #ifdef MUE_ACC_BYTE_SWAP */

    /* first high then low byte */
    mue_cch_data_cnt++;
    *mue_cch_data_ptr++ = *p_word8++;
    mue_cch_data_cnt++;
    *mue_cch_data_ptr++ = *p_word8;

#endif /* #else */

    return(MUE_RESULT_OK);
}
/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_tx_array
 *
 *  Purpose   : Transmits data of a buffer over the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_tx_array
 *              (
 *                  UNSIGNED16  size,
 *                  void        *p_buffer
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              size       - size of the buffer (number of bytes to
 *                           transmit over the UART)
 *              p_buffer   - pointer to buffer that contains data
 *                           to transmit over the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for data related to a MVB process data port
 *                (size = 2, 4, 8, 16 or 32 bytes).
 *              - Used for data related to a MVB message data frame
 *                (size = 32 bytes).
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_tx_array(UNSIGNED16 size,  void *p_buffer)
{
    UNSIGNED16  i;
    WORD8       *p_word8 = (WORD8*)p_buffer;

#ifdef MUE_ACC_HW_LE

    for (i=0; i<size; i+=2)
    {
        mue_cch_data_cnt++;
        *mue_cch_data_ptr++ = p_word8[i+1]; /* low  byte */
        mue_cch_data_cnt++;
        *mue_cch_data_ptr++ = p_word8[i+0]; /* high byte */
    } 

#else 

    for (i=0; i<size; i+=2)
    {
        mue_cch_data_cnt++;
        *mue_cch_data_ptr++ = *p_word8++; /* high byte */
        mue_cch_data_cnt++;
        *mue_cch_data_ptr++ = *p_word8++; /* low  byte */
    } 

#endif 

    return(MUE_RESULT_OK);
}
/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_tx_fill
 *
 *  Purpose   : Transmits fill bytes over the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_tx_fill
 *              (
 *                  UNSIGNED16  size
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              size       - number of fill bytes to transmit over the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for fill bytes related to a MVB process data port
 *                (size = 0, 16, 24, 28 or 30 bytes).
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_tx_fill(UNSIGNED16  size)
{
    UNSIGNED16  i;

    for (i=0; i<size; i++)
    {
        mue_cch_data_cnt++;
        *mue_cch_data_ptr++ = 0x00;
    } 
    return(MUE_RESULT_OK);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_tx_trigger
 *
 *  Purpose   : Triggers a transmission over the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_tx_trigger
 *              (
 *                  void
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_tx_trigger(void)
{
    MUE_RESULT  mue_result;
    UNSIGNED8   cch_size;

    cch_size = (UNSIGNED8)mue_cch_data_cnt;
    mue_result =  mue_cch_transmit((UNSIGNED8)cch_size,  (WORD8*)mue_cch_data_buf);

    return(mue_result);
}
/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_rx_wait
 *
 *  Purpose   : Waits, until something to receive from the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_rx_wait
 *              (
 *                  UNSIGNED16  size
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              size       - number of bytes to receive from the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_rx_wait(UNSIGNED16 size)
{
    MUE_RESULT  mue_result;
    UNSIGNED8   cch_size;

    mue_cch_data_cnt = 0;
    mue_cch_data_ptr = (WORD8*)mue_cch_data_buf;

    cch_size = (UNSIGNED8)size;
    mue_result =  mue_cch_receive((UNSIGNED8)cch_size, (WORD8*)mue_cch_data_buf);

    return(mue_result);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_rx_value8
 *
 *  Purpose   : Receives a 8-bit value from the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_rx_value8
 *              (
 *                  WORD8       *p_value8
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *
 *  Output    : p_value8   - pointer to buffer that receives a 8-bit value
 *                           from the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for general parameters.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_rx_value8( WORD8 *p_value8)
{
    mue_cch_data_cnt++;
    *p_value8 = *mue_cch_data_ptr++;

    return(MUE_RESULT_OK);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_rx_value16
 *
 *  Purpose   : Receives a 16-bit value from the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_rx_value16
 *              (
 *                  WORD16      *p_value16
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *
 *  Output    : p_value16  - pointer to buffer that receives a 16-bit value
 *                             from the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for general parameters.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_rx_value16(WORD16 *p_value16)
{
    WORD8       *p_word8 = (WORD8*)p_value16;


#ifdef MUE_ACC_BYTE_SWAP

    /* first low then high byte */
    mue_cch_data_cnt++;
    p_word8[1] = *mue_cch_data_ptr++;
    mue_cch_data_cnt++;
    p_word8[0] = *mue_cch_data_ptr++;

#else /* #ifdef MUE_ACC_BYTE_SWAP */

    /* first high then low byte */
    mue_cch_data_cnt++;
    *p_word8++ = *mue_cch_data_ptr++;
    mue_cch_data_cnt++;
    *p_word8   = *mue_cch_data_ptr++;

#endif /* #else */

    return(MUE_RESULT_OK);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_rx_array
 *
 *  Purpose   : Receives data into a buffer from the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_rx_array
 *              (
 *                  UNSIGNED16  size,
 *                  void        *p_buffer
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              size       - size of the buffer (number of bytes to receive
 *                           from the UART)
 *
 *  Output    : p_buffer   - pointer to buffer that receives data from
 *                           the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for data related to a MVB process data port
 *                (size = 2, 4, 8, 16 or 32 bytes).
 *              - Used for data related to a MVB message data frame
 *                (size = 32 bytes).
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_rx_array(UNSIGNED16 size, void  *p_buffer)
{
    UNSIGNED16  i;
    WORD8       *p_word8 = (WORD8*)p_buffer;

#ifdef MUE_ACC_HW_LE

    for (i=0; i<size; i+=2)
    {
        mue_cch_data_cnt++;
        p_word8[i+1] = *mue_cch_data_ptr++; /* low  byte */
        mue_cch_data_cnt++;
        p_word8[i+0] = *mue_cch_data_ptr++; /* high byte */
    } 

#else 

    for (i=0; i<size; i+=2)
    {
        mue_cch_data_cnt++;
        *p_word8++ = *mue_cch_data_ptr++; /* high byte */
        mue_cch_data_cnt++;
        *p_word8++ = *mue_cch_data_ptr++; /* low  byte */
    } 

#endif 

    return(MUE_RESULT_OK);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_rx_fill
 *
 *  Purpose   : Receives fill bytes from the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_rx_fill
 *              (
 *                  UNSIGNED16  size
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              size       - number of fill bytes to receive from the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for fill bytes related to a MVB process data port
 *                (size = 0, 16, 24, 28 or 30 bytes).
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_rx_fill(UNSIGNED16 size)
{
    UNSIGNED16  i;

    for (i=0; i<size; i++)
    {
        mue_cch_data_cnt++;
        mue_cch_data_ptr++;
    } 

    return(MUE_RESULT_OK);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_rx_ignore
 *
 *  Purpose   : Ignore surplus data (fill bytes) from the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_rx_ignore
 *              (
 *                  void
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *              size       - number of fill bytes to ignore from the UART
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - Used for surplus bytes related to a MVB device status or
 *                a MVB process data port.
 *              - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_rx_ignore(void)
{
    return(MUE_RESULT_OK);
}


/* --------------------------------------------------------------------------
 *  Procedure : mue_acc_rx_close
 *
 *  Purpose   : Closes a receive block from the UART.
 *
 *  Syntax    : TCN_DECL_PUBLIC
 *              MUE_RESULT
 *              mue_acc_rx_close
 *              (
 *                  void
 *              );
 *
 *  Input     : p_bus_ctrl - pointer to bus controller specific values
 *
 *  Return    : result code; any MUE_RESULT
 *
 *  Remarks   : - A MVB controller is identified by 'p_bus_ctrl'.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_acc_rx_close(void)
{
    return(MUE_RESULT_OK);
}
