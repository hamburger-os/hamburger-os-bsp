/* ==========================================================================
 *
 *  File      : MUE_CCH.C
 *
 *  Purpose   : MVB UART Emulation - Communication Channel
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
#include "board.h"

/* ==========================================================================
 *
 *  Pre-processor Definitions:
 *  --------------------------
 *  - MUE_CCH_PRINT_FCT_CALL      - printout of function call
 *                                  (0=disable, 1=enable)
 *  - MUE_CCH_PRINT_FCT_RETURN    - printout of function return
 *                                  (0=disable, 1=enable)
 *  - MUE_CCH_PRINT_FCT_PARAMETER - printout of function parameter
 *                                  (0=disable, 1=enable)
 *  - MUE_CCH_PRINT_FCT_INFO      - printout of function information
 *                                  (0=disable, 1=enable)
 *  - MUE_CCH_PRINT_REG           - printout of register access
 *                                  (0=disable, 1=enable)
 *
 * ==========================================================================
 */
#define MUE_CCH_PRINT_FCT_CALL      0
#define MUE_CCH_PRINT_FCT_RETURN    0
#define MUE_CCH_PRINT_FCT_PARAMETER 0
#define MUE_CCH_PRINT_FCT_INFO      0
#define MUE_CCH_PRINT_REG           0
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
#include "mue_osl.h"
#include "mue_cch.h"
/* --------------------------------------------------------------------------
 *  Register Access of MVB UART Emulation
 * --------------------------------------------------------------------------
 */
#include <string.h> /* memset    */
#include "stm32h7xx_hal.h"
/* ==========================================================================
 *
 *  Local Variables
 *
 * ==========================================================================
 */

/* --------------------------------------------------------------------------
 *  Access to register set of MVB UART Emulation:
 *  - cch_reg_data            : address of MVB UART Emulation register "DATA"
 *  - cch_reg_stat            : address of MVB UART Emulation register "STAT"
 *  - cch_reg_stat_txready    : mask value of register "STAT" indicating
 *                              "ready to transmit"
 *  - cch_reg_stat_rxready    : mask value of register "STAT" indicating
 *                              "ready to receive"
 *  - cch_time_start          : used for timeout
 
 * --------------------------------------------------------------------------
 */
#if 1
 /*This register is used reading and writing RX and TX data*/
TCN_DECL_LOCAL TCN_DECL_CONST \
    unsigned long  cch_reg_data = (unsigned long)(0x68000000 + (0x00 << 1)); /* reg-index 0 */

/*this register is used for reading the UARTs line status(write has no effect)*/
/*bit7(TRDY) BIT6 - BIT1 (NULL) BIT0(RRDY)*/
TCN_DECL_LOCAL TCN_DECL_CONST \
    unsigned long  cch_reg_stat = (unsigned long)(0x68000000 + (0x01 << 1)); /* reg-index 1 */

#else
 /*This register is used reading and writing RX and TX data*/
TCN_DECL_LOCAL TCN_DECL_CONST \
    unsigned long  cch_reg_data = MVB_CMD; /* reg-index 0 */

/*this register is used for reading the UARTs line status(write has no effect)*/
/*bit7(TRDY) BIT6 - BIT1 (NULL) BIT0(RRDY)*/
TCN_DECL_LOCAL TCN_DECL_CONST \
    unsigned long  cch_reg_stat = (MVB_CMD - 2); /* reg-index 1 */
#endif
/*Transmitter ready to take additional data.if this bit is set then at least 35 bytes of internal FIFO
  are available  I.E. it is legal to find TRDY =1 once, then up to 35 bytes may be transferred to RXTXD 
	without reading TRDY again*/
TCN_DECL_LOCAL TCN_DECL_CONST \
    UNSIGNED8   cch_reg_stat_txready = (UNSIGNED8)0x80;
		
/* Received Data Ready if this bit is set then RX data is available to read from RXTXD since the read data is 
   generated by the UART emulation protocal  as one single block all bytes of a received parameter block
	 become available at the same time in orrder to save time ,it is legal to read RRDY once the read the number
	 of RX bytes as defined for the different protocal commands*/
TCN_DECL_LOCAL TCN_DECL_CONST \
    UNSIGNED8   cch_reg_stat_rxready = (UNSIGNED8)0x01;

/* ==========================================================================
 *
 *  Local Procedures
 *
 * ==========================================================================
 */
 #if     FMC_SRAM_MODE
typedef struct mvb_info 
{
  void volatile *hw_addr;
  void volatile *hw_addr_cmd;
	
}S_MVB_INFO;

__attribute__((at(0x3000A000))) S_MVB_INFO g_s_mvb_dev;

void  MVB_ADR_init(void)
{
	g_s_mvb_dev.hw_addr = (void volatile *)cch_reg_data;
	g_s_mvb_dev.hw_addr_cmd = (void volatile *)cch_reg_stat;
}

__inline unsigned char mvb_readb(void volatile *addr)
{
  return *(unsigned char *)addr;
}

void mvb_writeb(unsigned char v_u8, void volatile *addr)
{
  *(unsigned char *)addr = v_u8;
	  __DSB();
}
#else

#endif
/* --------------------------------------------------------------------------
 *  Procedure : cch_reg_stat_ready
 *
 *  Purpose   : Check if MVB UART Emulation register "STAT" indicates ready.
 *
 *  Syntax    : UNSIGNED8
 *              cch_reg_stat_ready
 *              (
 *                  UNSIGNED8   reg_stat_mask
 *              );
 *
 *  Input     : card_id       - card identifier
 *              reg_stat_mask - mask indicating ready
 *
 *  Return    : 1 = ready, 0 = not ready
 * --------------------------------------------------------------------------
 */

TCN_DECL_LOCAL  UNSIGNED8  cch_reg_stat_ready(UNSIGNED8 reg_stat_mask)
{
    UNSIGNED8   result;
    UNSIGNED8   reg_stat_value;
    result = 0;

    /* ----------------------------------------------------------------------
     *  Read value from MVB UART Emulation register "STAT".
     * ----------------------------------------------------------------------
     */
#if     FMC_SRAM_MODE
		      reg_stat_value = mvb_readb(g_s_mvb_dev.hw_addr_cmd);
#else
	        reg_stat_value = (unsigned char)*((volatile unsigned char *)cch_reg_stat);
#endif
    /* ----------------------------------------------------------------------
     *  Check for ready.
     * ----------------------------------------------------------------------
     */
    if ((reg_stat_value & reg_stat_mask) == reg_stat_mask)
    {
        result = 1;
    } /* if ((reg_stat_value & reg_stat_mask) == reg_stat_mask) */

    return(result);
}

/* --------------------------------------------------------------------------
 *  Procedure : cch_put_data8
 *
 *  Purpose   : Write a data packet to MVB UART Emulation register "DATA"
 *              using 8-bit access.
 *
 *  Syntax    : void
 *              cch_put_data8
 *              (
 *                  UNSIGNED16  size,
 *                  UNSIGNED8   *p_packet
 *              );
 *
 *  Input     : card_id  - card identifier
 *              size     - size of data packet (number of bytes)
 *              p_packet - pointer to data packet
 * --------------------------------------------------------------------------
 */

TCN_DECL_LOCAL void cch_put_data8(UNSIGNED16 size, UNSIGNED8 *p_packet)
{
    UNSIGNED8 *p8;
    UNSIGNED16  bytes_written;
    p8 = p_packet;
    bytes_written = 0;
    while (bytes_written < size)
    {
        /* ------------------------------------------------------------------
         *  Write value to MVB UART Emulation register "DATA".
         * ------------------------------------------------------------------
         */
#if     FMC_SRAM_MODE
		      mvb_writeb(*p8,g_s_mvb_dev.hw_addr);
#else
	       *(volatile unsigned char *)cch_reg_data = *p8;
			   __DSB();
#endif			
        p8++;
        bytes_written++;
    }
}


/* --------------------------------------------------------------------------
 *  Procedure : cch_get_data8
 *
 *  Purpose   : Read a data packet from MVB UART Emulation register "DATA"
 *              using 8-bit access.
 *
 *  Syntax    : void
 *              cch_get_data8
 *              (
 *                  UNSIGNED16  size,
 *                  UNSIGNED8   *p_packet
 *              );
 *
 *  Input     : card_id  - card identifier
 *              size     - size of data packet (number of bytes)
 *              p_packet - pointer to data packet
 * --------------------------------------------------------------------------
 */
TCN_DECL_LOCAL void cch_get_data8(UNSIGNED16 size, UNSIGNED8 *p_packet)
{
    UNSIGNED8   *p8;
    UNSIGNED16  bytes_read;
    p8 = p_packet;
    bytes_read = 0;
	
    while (bytes_read < size)
    {
        /* ------------------------------------------------------------------
         *  Read value from MVB UART Emulation register "DATA".
         * ------------------------------------------------------------------
         */
#if     FMC_SRAM_MODE
		     *p8 = mvb_readb(g_s_mvb_dev.hw_addr);
#else
			   *p8 = (unsigned char)*((volatile unsigned char *)cch_reg_data);
#endif			

         p8++;
         bytes_read++;
    }
}

/* ==========================================================================
 *
 *  Public Procedures
 *
 * ==========================================================================
 */



/* --------------------------------------------------------------------------
 *  Procedure : mue_cch_clean_up
 *
 *  Purpose   : Performs a clean-up of the communication channel.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_cch_clean_up(void)
{
    MUE_RESULT  mue_result;
    UNSIGNED8   packet[35];
    UNSIGNED16  size;

    mue_result = MUE_RESULT_OK;

    /* ----------------------------------------------------------------------
     *  Facts for devices based on MVB UART Emulation with register set:
     *  - The size of the longest MVB UART Emulation Command is 35 bytes.
     *  - If a MVB UART Emulation Command will response with data, then
     *    this data located in the UART RX-FIFO can be cleared by sending
     *    any byte.
     *  Assuming following scenario:
     *  - At least one byte of a MVB UART Emulation Command has been sent.
     *  - In this case 34 bytes must be send to synchronise the MVB UART
     *    Emulation Protocol.
     *  Conclusion:
     *  - Transmit at least 35 bytes, which will synchronise the MVB UART
     *    Emulation Protocol and clear the UART RX-FIFO.
     *  - Don't check for TX-ready before transmitting, since interruption
     *    of a previous transmit communication may result in TX-status
     *    "not ready".
     * ----------------------------------------------------------------------
     */
    if (MUE_RESULT_OK == mue_result)
    {
        size = 35;
        memset((void*)packet, 0, size);
        cch_put_data8(size, packet);
    } 

    return(mue_result);

}


/* --------------------------------------------------------------------------
 *  Procedure : mue_cch_transmit
 *
 *  Purpose   : Transmits data packet to communication channel.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_cch_transmit( UNSIGNED8 size, WORD8 *p_packet)
{
    MUE_RESULT  mue_result;
    UNSIGNED8   txready;

    /* ----------------------------------------------------------------------
     *  Transmit data packet.
     * ----------------------------------------------------------------------
     */
    if (0 == size)
    {
        mue_result = MUE_RESULT_OK;
    } /* if (0 == size) */
    else
    {
        /* ------------------------------------------------------------------
         *  Poll for TX-ready or timeout.
         * ------------------------------------------------------------------
         */
        txready = 0;
        while (1)
        {
            if (cch_reg_stat_ready(cch_reg_stat_txready))
            {
                txready = 1;
                break;
            }
        }

        /* ------------------------------------------------------------------
         *  Put data packet to UART FIFO.
         * ------------------------------------------------------------------
         */
        if (txready)
        {
            cch_put_data8((UNSIGNED16)size, (UNSIGNED8*)p_packet);
            mue_result = MUE_RESULT_OK;
        } /* if (txready) */
        else
        {
            mue_result = MUE_RESULT_TRANSMIT_TIMEOUT;
        } /* else */

    } /* else */

    return(mue_result);

}


/* --------------------------------------------------------------------------
 *  Procedure : mue_cch_receive
 *
 *  Purpose   : Receives data packet from communication channel.
 * --------------------------------------------------------------------------
 */
TCN_DECL_PUBLIC  MUE_RESULT  mue_cch_receive(UNSIGNED8 size, WORD8 *p_packet)
{
    MUE_RESULT  mue_result;
    UNSIGNED8   rxready;

    /* ----------------------------------------------------------------------
     *  Receive data packet.
     * ----------------------------------------------------------------------
     */
    if (0 == size)
    {
        mue_result = MUE_RESULT_OK;
    } /* if (0 == size) */
    else
    {
        /* ------------------------------------------------------------------
         *  Poll for RX-ready or timeout.
         * ------------------------------------------------------------------
         */
        rxready = 0;

        while (1)
        {
            if (cch_reg_stat_ready(cch_reg_stat_rxready))
            {
                rxready = 1;
                break;
            } /* if (cch_reg_stat_ready(...)) */

        } /* while (1) */

        /* ------------------------------------------------------------------
         *  Get data packet from UART FIFO.
         * ------------------------------------------------------------------
         */
        if (rxready)
        {
            cch_get_data8((UNSIGNED16)size, (UNSIGNED8*)p_packet);
            mue_result = MUE_RESULT_OK;
        } /* if (rxready) */
        else
        {
            mue_result = MUE_RESULT_RECEIVE_TIMEOUT;
        } /* else */

    } /* else */

    return(mue_result);

}
