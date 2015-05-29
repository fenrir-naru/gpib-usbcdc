/*
 * Copyright (c) 2015, M.Naruoka (fenrir)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the naruoka.org nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "gpib_io.h"
#include "c8051f380.h"
#include "main.h"
#include "gpib.h"

#define TE 0x10
#define SC 0x20
#define DC 0x40
#define PE 0x80

#define SRQ 0x01
#define ATN 0x02
#define EOI 0x04
#define DAV 0x08
#define NRFD 0x10
#define NDAC 0x20
#define IFC 0x40
#define REN 0x80

#ifdef USE_ASM_FOR_SFR_MANIP
#define p0_hiz(port)   {__asm orl _P0,SHARP  (port) __endasm; }
#define p0_low(port)   {__asm anl _P0,SHARP ~(port) __endasm; }
#define p2_hiz(port)   {__asm orl _P2,SHARP  (port) __endasm; }
#define p2_low(port)   {__asm anl _P2,SHARP ~(port) __endasm; }
#else
#define p0_hiz(port)   (P0 |=  (port))
#define p0_low(port)   (P0 &= ~(port))
#define p2_hiz(port)   (P2 |=  (port))
#define p2_low(port)   (P2 &= ~(port))
#endif

static __bit is_talker;

static void set_talker(){
  if(!is_talker){
    is_talker = TRUE;
    p2_hiz(EOI | DAV | NRFD | NDAC);
    p0_hiz(TE | PE); /* R[NDAC,NRFD], T[Data,DAV] */
  }
}

static void set_listener(){
  if(is_talker){
    is_talker = FALSE;
    P1 = 0xFF; /* Float all data lines */
    p2_hiz(EOI | DAV | NRFD | NDAC);
    p0_low(TE | PE); /* R[Data,DAV], T[NDAC,NRFD] */
    p2_low(NRFD | NDAC);
  }
}

// Puts all the GPIB pins into their correct initial states.
void gpib_io_init(){
  // Initialize as a listener
  is_talker = TRUE;
  set_listener();

  p2_hiz(ATN | IFC | SRQ | REN);
  if(gpib_config.is_controller){
    p2_low(REN);
    p0_low(DC); // T[ATN], R[SRQ]
    p0_hiz(SC); // T[REN,IFC]
  }else{
    p0_hiz(DC); // T[SRQ], R[ATN]
    p0_low(SC); // R[REN,IFC]
  }
}

static __xdata u8 timeout_10ms_max;

void gpib_io_set_timeout(){
  if(gpib_config.timeout_ms == 0){timeout_10ms_max = 0xFF;} // if 0, wait forever
  else{
    u16 div10 = (gpib_config.timeout_ms + 9) / 10;
    if(div10 >= 0xFF){timeout_10ms_max = 0xFE;} // max timeout is 0xFE * 10 = 2540 ms
    timeout_10ms_max = (u8)(div10);
  }
}

u8 gpib_uniline(enum uniline_message_t msg){
  switch(msg){
    case GPIB_UNI_CMD_START: p2_low(ATN); break;
    case GPIB_UNI_CMD_END: p2_hiz(ATN); break;
    case GPIB_UNI_BUS_CLEAR_START: p2_low(IFC); break;
    case GPIB_UNI_BUS_CLEAR_END: p2_hiz(IFC); break;
    case GPIB_UNI_CHECK_SRQ_ASSERT: return !(P2 & SRQ);
    case GPIB_UNI_SRQ_ASSERT: p2_low(SRQ); break;
    case GPIB_UNI_SRQ_DEASSERT: p2_hiz(SRQ); break;
  }
  return 0;
}

static u8 wait_p2(u8 state, u8 mask){
  timeout_10ms = 0;
  do{
    if((P2 & mask) == state){return 0;}
  }while(timeout_10ms_max >= timeout_10ms);
  return 1; // timeout
}

static u8 putchar_internal(u8 c, u8 flags){

  P1 = (c ^ 0xFF); // Put the byte on the data lines
  if(flags & GPIB_WRITE_USE_EOI){
    p2_low(EOI); // Assert EOI
  }

  // Make sure that NRFD is high
  if(wait_p2(NRFD, NRFD)){
    gpib_io_init();
    return 0;
  }
  p2_low(DAV); // Inform listeners that the data is ready to be read

  // Wait for NDAC to go high, all listeners have accepted the byte
  if(wait_p2(NDAC, NDAC)){
    gpib_io_init();
    return 0;
  }
  p2_hiz(DAV | EOI); // Byte has been accepted by all, indicate byte is no longer valid

  return 1;
}

u8 gpib_putchar(char c, u8 flags){

  set_talker();
  if(putchar_internal(c, flags) == 0){return 0;}

  return 1;
}

u16 gpib_write(char *buf, u16 length, u8 flags){
  u16 remain = length;
  u8 flags_without_eoi = flags & ~(GPIB_WRITE_USE_EOI);

  if(length == 0){return 0;}

  set_talker();
  do{ // Loop through each character, write to bus
    if(putchar_internal(*buf++, (remain >= 1) ? flags_without_eoi : flags) == 0){
      return length - remain;
    }
  }while(--remain);

  return length - remain;
}

static int getchar_internal(){
  int res;
  
  // Assuming both NDAC and NRFD are LOW.

  // Raise NRFD, telling the talker we are ready for the next byte
  p2_hiz(NRFD);
  // Wait for DAV to go low (talker informing us the byte is ready)
  if(wait_p2(0, DAV)){
    gpib_io_init();
    return -1; // timeout
  }
  // Assert NRFD, informing talker to not change the data lines
  p2_low(NRFD);

  // Read port, and flip all bits since GPIB uses negative logic.
  res = (u8)(P1 ^ 0xFF);
  if(!(P2 & EOI)){res += 0x100;} // END
  if(!(P2 & ATN)){res += 0x200;} // CMD

  // Un-assert NDAC, informing talker that we have accepted the byte
  p2_hiz(NDAC);
  // Wait for DAV to go high (talker knows that we have read the byte)
  if(wait_p2(DAV, DAV)){
    gpib_io_init();
    return -1; // timeout
  }
  // Low NDAC to prepare for next byte
  p2_low(NDAC);

  return res;
}

int gpib_getchar(){
  set_listener();
  return getchar_internal();
}

u16 gpib_read(void (*push)(char), u8 flags){
  u16 read_count = 0;
  int res;
  char c;
  __bit last_char_is_cr = FALSE;

  // Make correspond receiving and transmitting terminators, different from Prologic impl.
  u8 terminator
      = (flags & GPIB_READ_UNTIL_EOI) ? 3 : gpib_config.eos;

  set_listener();
  while(1){
    res = getchar_internal();
    if(GPIB_GETCHAR_IS_ERROR(res)){break;}
    read_count++;
    c = GPIB_GETCHAR_TO_DATA(res);
    push(c);
    if(GPIB_GETCHAR_IS_EOI(res)){
      if(gpib_config.eot){
        push(gpib_config.eot_char);
      }
      break;
    }
    if(c == '\r'){
      if(terminator == 1){break;}
      last_char_is_cr = TRUE;
    }else{
      if(c == '\n'){
        if((terminator == 2)
            || ((last_char_is_cr) && (terminator == 0))){break;}
      }
      last_char_is_cr = FALSE;
    }
  }

  return read_count;
}
