/*
 * ===================================================================
 *  TS 26.104
 *  REL-5 V5.4.0 2004-03
 *  REL-6 V6.1.0 2004-03
 *  REL-15 V15.1.0 2018-07
 *  3GPP AMR Floating-point Speech Codec
 * ===================================================================
 *
 */

/*
 * interf_dec.h
 *
 *
 * Project:
 *    AMR Floating-Point Codec
 *
 * Contains:
 *    Defines interface to AMR decoder
 *
 */

#ifndef _interf_dec_h_
#define _interf_dec_h_

#include "typedef.h"

/*
 * Function prototypes
 */
/*
 * Conversion from packed bitstream to endoded parameters
 * Decoding parameters to speech
 */
void Decoder_Interface_Decode(void *st,

#ifndef ETSI
                              unsigned char *bits,

#else
                               short *bits,
#endif

                              short *synth, int bfi) AMR_SECTION;

/*
 * Reserve and init. memory
 */
void *Decoder_Interface_init(void) AMR_SECTION;

/*
 * Exit and free memory
 */
void Decoder_Interface_exit(void *state) AMR_SECTION;

#endif
