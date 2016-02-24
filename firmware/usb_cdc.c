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

#include <stdio.h>
#include <string.h>

#include "c8051f380.h"
#include "main.h"
#include "f38x_usb_register.h"
#include "f38x_usb.h"

#include "usb_cdc.h"
#include "util.h"

// bitmap for set_line_state
#define CDC_DTR     0x01
#define CDC_RTS     0x02

// bitmap for update_line_state
#define CDC_DCD     0x01  // 
#define CDC_DSR     0x02  // data set ready
#define CDC_BREAK   0x04  // break reception
#define CDC_RI      0x08  // ring indicator
#define CDC_FRAME   0x10  // frame error
#define CDC_PARITY  0x20  // parity error
#define CDC_OVRRUN  0x40  // overrun error
#define CDC_CTS     0x80  // clear to send

static u16 cdc_rx_size();

void cdc_polling(){
  static __xdata u8 previous_frame_num = usb_frame_num & 0xF0;
  u8 current_frame_num = usb_frame_num & 0xF0; // per 16 frames

  if(previous_frame_num == current_frame_num){
    return;
  }
  previous_frame_num = current_frame_num;

  cdc_tx(NULL, 0); // flush

#ifndef CDC_IS_REPLACED_BY_FTDI
  /*{
    // ResponseAvailable
    static const __code u8 buf[] = {
      0xA1, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    usb_write(buf, sizeof(buf), CDC_COM_EP_IN);
  }*/
  {
    // SerialState
    static const __code u8 buf[] = {
      0xA1, 0x20, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
      0x03, 0x00
    };
    usb_write(buf, sizeof(buf), CDC_COM_EP_IN);
  }
#endif
}

// line coding structure and its default value
static cdc_line_coding_t __xdata uart_line_coding = {
  {le_u32(115200)}, // baudrate
  0,      // stop bit:  0-1, 1-1.5, 2-2
  0,      // parity:    0-none, 1-odd, 2-even, 3-mark, 4-space
  8       // data bits: 5,6,7,8,16
};

static __xdata u8 ep0_data_buf[8];

// called at the end of data stage
static void set_line_coding_complete(){
  if(memcmp(&uart_line_coding, ep0_data_buf, sizeof(cdc_line_coding_t)) != 0){
    memcpy(&uart_line_coding, ep0_data_buf, sizeof(cdc_line_coding_t));
  }
}

// Line status output
static volatile __bit uart_DTR = FALSE;        // Line status output
static volatile __bit uart_RTS = FALSE;

static void set_line_state(u8 st){
  uart_DTR = (st & CDC_DTR);
  uart_RTS = (st & CDC_RTS);
}

static void send_break(u16 dur){}

#ifdef CDC_IS_REPLACED_BY_FTDI

#define PORT_RESET        0x00
#define MODEM_CTRL        0x01
#define SET_FLOW_CTRL     0x02
#define SET_BAUD_RATE     0x03
#define SET_DATA          0x04
#define GET_MODEM_STATUS  0x05
#define SET_EVENT_CHAR    0x06
#define SET_ERROR_CHAR    0x07
#define SET_LATENCY_TIMER 0x09
#define GET_LATENCY_TIMER 0x10
#define READ_EEPROM       0x90
#define WRITE_EEPROM      0x91
#define ERACE_EEPROM      0x92

/*
 * Ref) FT232R
 * 0000 : 00 40 03 04 01 60 00 00 
 *        A0 2D 08 00 00 00 98 0A   @. .. `. .. .. .. .. ..
 * 0010 : A2 20 C2 12 23 10 05 00 
 *        0A 03 46 00 54 00 44 00   .. .. .. .. .. .F .T .D
 * 0020 : 49 00 20 03 46 00 54 00 
 *        32 00 33 00 32 00 52 00   .I .. .F .T .2 .3 .2 .R
 * 0030 : 20 00 55 00 53 00 42 00 
 *        20 00 55 00 41 00 52 00   .. .U .S .B .. .U .A .R
 * 0040 : 54 00 12 03 41 00 36 00 
 *        52 00 48 00 47 00 55 00   .T .. .A .6 .R .H .G .U
 * 0050 : 37 00 53 00 00 00 01 00 
 *        00 00 00 00 00 00 00 00   .7 .S .. .. .. .. .. ..
 * 0060 : 00 00 00 00 00 00 00 00 
 *        00 00 00 00 00 00 00 00   .. .. .. .. .. .. .. ..
 * 0070 : 00 00 00 00 00 00 00 00 
 *        00 00 00 00 00 00 E2 18   .. .. .. .. .. .. .. ..
 * 0080 : 29 04 D6 FB 00 00 75 E2 
 *        0F 60 42 00 00 00 00 00   .. .. .. .u `. .B .. ..
 * 0090 : 00 00 00 00 00 00 00 00 
 *        36 41 51 51 5A 4C 55 55   .. .. .. .. A6 QQ LZ UU
 */

static const __code unsigned char ftdi_rom[] = {
  0x00, 0x40, 0xC4, 0x10, 0x97, 0x01, 0x00, 0x00,  // 0x08
  0xA0, 0x2D, 0x08, 0x00, 0x00, 0x00, 0x98, 0x0E,  // 0x10
  0xA6, 0x12, 0xB8, 0x10, 0x23, 0x10, 0x05, 0x00,  // 0x18
  0x0E, 0x03, 0x66, 0x00, 0x65, 0x00, 0x6E, 0x00,  // 0x20
  0x72, 0x00, 0x69, 0x00, 0x72, 0x00, 0x12, 0x03,  // 0x28
  0x53, 0x00, 0x79, 0x00, 0x6C, 0x00, 0x70, 0x00,  // 0x30
  0x68, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00,  // 0x38
  0x10, 0x03, 0x41, 0x00, 0x36, 0x00, 0x52, 0x00,  // 0x40
  0x48, 0x00, 0x47, 0x00, 0x37, 0x00, 0x53, 0x00,  // 0x48
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0x50
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0x58
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0x60
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0x68
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0x70
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 0x78
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x91,  // 0x80
  
  0x23, 0x04, 0xDC, 0xFB, 0x00, 0x00, 0xD8, 0x07, 
  0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x33, 0x41, 0x37, 0x4F, 0x46, 0x30, 0x55, 0x36,
  
  /*
  0x00, 0x6c, 0x6f, 0x67, 0x2e, 0x64, 0x61, 0x74,  // 0x88
  0x00, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2e,  // 0x90
  0x64, 0x61, 0x74, 0x00, 0x12, 0x01, 0x00, 0x02,  // 0x98
  0xef, 0x02, 0x01, 0x40, 0xc4, 0x10, 0x97, 0x01,  // 0xA0
  0x00, 0x00, 0x01, 0x02, 0x03, 0x01, 0x09, 0x02,  // 0xA8
  0x3f, 0x00, 0x02, 0x01, 0x00, 0x80, 0x0f, 0x08,  // 0xB0
  0x0b, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x09,  // 0xB8
  0x04, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 0x00,  // 0xC0
  0x07, 0x05, 0x82, 0x03, 0x40, 0x00, 0x01, 0x07,  // 0xC8
  0x05, 0x02, 0x02, 0x40, 0x00, 0x00, 0x09, 0x04,  // 0xD0
  0x01, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00, 0x07,  // 0xD8
  0x05, 0x83, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05,  // 0xE0
  0x03, 0x02, 0x40, 0x00, 0x00, 0x04, 0x03, 0x09,  // 0xE8
  0x04, 0x2a, 0x03, 0x53, 0x00, 0x69, 0x00, 0x6c,  // 0xF0
  0x00, 0x69, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6e,  // 0xF8
  0x00, 0x20, 0x00, 0x4c, 0x00, 0x61, 0x00, 0x62,  // 0x100
  */
};  // 128 (+ 128) bytes TODO: where is PnP flag?

// HEADER0 -- Modem Status
#define HEADER0_SIGN  0x01      // 0.B0..B3 Reserved
#define HEADER0_CTS   (1 << 4)  // 0.B4 Clear to Send (CTS)
#define HEADER0_DSR   (1 << 5)  // 0.B5 Data Set Ready (DSR)
#define HEADER0_RI    (1 << 6)  // 0.B6 Ring Indicator (RI)
#define HEADER0_RLSD  (1 << 7)  // 0.B7 Receive Line Signal Detect (RLSD)

// HEADER1 -- Line Status
#define HEADER1_DR    1         // 1.B0 Data Ready (DR)
#define HEADER1_OE    (1 << 1)  // 1.B1 Overrun Error (OE)
#define HEADER1_PE    (1 << 2)  // 1.B2 Parity Error (PE)
#define HEADER1_FE    (1 << 3)  // 1.B3 Framing Error (FE)
#define HEADER1_BI    (1 << 4)  // 1.B4 Break Interrupt (BI)
#define HEADER1_THRE  (1 << 5)  // 1.B5 Transmitter Holding Register (THRE)
#define HEADER1_TEMT  (1 << 6)  // 1.B6 Transmitter Empty (TEMT)
#define HEADER1_FIFO  (1 << 7)  // 1.B7 Error in RCVR FIFO

#else

/**
 * Send_Encapsulated_Command
 * 
 * Nothing to do other than unloading the data sent in the data stage.
 */
static void cdc_Send_Encapsulated_Command(){
  if((ep0_setup.bmRequestType == OUT_CL_INTERFACE)
      && (ep0_setup.wValue.i == 0 )
      && (ep0_setup.wLength.i <= sizeof(cdc_line_coding_t))){
    
    ep0_reserve_data(
        ep0_data_buf,
        min(sizeof(ep0_data_buf), ep0_setup.wLength.i),
        NULL
      );
    usb_ep0_status = EP_RX;
    ep0_request_completed = TRUE;
  }
}

/**
 * Get_Encapsulated_Command
 * 
 * Return a zero-length packet
 */
static void cdc_Get_Encapsulated_Command(){
  if((ep0_setup.bmRequestType == IN_CL_INTERFACE)
    && (ep0_setup.wValue.i == 0)){
             
    ep0_register_data(ep0_data_buf, 0); // Send ZLP
    usb_ep0_status = EP_TX;
    ep0_request_completed = TRUE;
  }
}

/**
 * Set_Line_Coding
 * 
 * Unload the line coding structure (7 bytes) sent in the data stage.
 * Apply this setting to the UART
 * Flush the communication buffer
 * 
 * Line Coding Structure (7 bytes)
 * 0-3 dwDTERate    Data terminal rate (baudrate), in bits per second (LSB first)
 * 4   bCharFormat  Stop bits: 0 - 1 Stop bit, 1 - 1.5 Stop bits, 2 - 2 Stop bits
 * 5   bParityType  Parity:    0 - None, 1 - Odd, 2 - Even, 3 - Mark, 4 - Space
 * 6   bDataBits    Data bits: 5, 6, 7, 8, 16
 */
static void cdc_Set_Line_Coding(){
  if((ep0_setup.bmRequestType == OUT_CL_INTERFACE)
    && (ep0_setup.wValue.i == 0)
    && (ep0_setup.wLength.i == sizeof(cdc_line_coding_t))){
    
    ep0_reserve_data(
        ep0_data_buf,
        sizeof(cdc_line_coding_t),
        set_line_coding_complete
      );
    usb_ep0_status = EP_RX;
    ep0_request_completed = TRUE;
  }
}

/**
 * Get_Line_Coding
 * 
 * Return the line coding structure
 */
static void cdc_Get_Line_Coding(){
  if((ep0_setup.bmRequestType == IN_CL_INTERFACE)
      && (ep0_setup.wValue.i == 0)
      && (ep0_setup.wLength.i == sizeof(cdc_line_coding_t))){

    ep0_register_data((u8 *)(&uart_line_coding), sizeof(cdc_line_coding_t));
    usb_ep0_status = EP_TX;
    ep0_request_completed = TRUE;
  }
}

/**
 * Set_ControlLine_State
 * 
 * Set/reset RTS/DTR according to wValue
 * wValue
 *  bit 1  RTS
 *  bit 0  DTR
 */
static void cdc_Set_ControlLine_State(){
  if((ep0_setup.bmRequestType == OUT_CL_INTERFACE)
    && (ep0_setup.wLength.i == 0)){
    set_line_state(ep0_setup.wValue.c[LSB] & (CDC_RTS | CDC_DTR));
    ep0_request_completed = TRUE;
  }
}

/**
 * Send_Break
 * 
 * Send break from UART TX port, for wValue (msec) duration.
 * wValue
 *  0xFFFF: continuous break
 *  0x0000: stop break
 */
static void cdc_Send_Break(){
  if ((ep0_setup.bmRequestType == OUT_CL_INTERFACE)
    && (ep0_setup.wLength.i == 0) ){
    send_break(ep0_setup.wValue.i);
    ep0_request_completed = TRUE;
  }
}

#endif

u16 cdc_tx(u8 *buf, u16 size){
  static __bit require_ZLP = FALSE;
#ifdef CDC_IS_REPLACED_BY_FTDI
#define TX_BUF_HEADER 2
  static __xdata u8 tx_packet[CDC_DATA_EP_IN_PACKET_SIZE-1] = {
      (HEADER0_SIGN | HEADER0_RI),    // 0x41 
      (HEADER1_THRE | HEADER1_TEMT)}; // 0x60
#else
#define TX_BUF_HEADER 0
  static __xdata u8 tx_packet[CDC_DATA_EP_IN_PACKET_SIZE];
#endif
  static __xdata u8 margin = sizeof(tx_packet) - TX_BUF_HEADER;
  u16 written = 0;
  u8 retry;
  if(size == 0){ // flush
    if(margin < (sizeof(tx_packet) - TX_BUF_HEADER)){
      if(usb_write(tx_packet, sizeof(tx_packet) - margin, CDC_DATA_EP_IN) > 0){
        if((sizeof(tx_packet) == CDC_DATA_EP_IN_PACKET_SIZE) && (margin == 0)){
          require_ZLP = TRUE;
        }else{
          require_ZLP = FALSE;
        }
        margin = sizeof(tx_packet) - TX_BUF_HEADER;
      }
    }
    if(require_ZLP){
      require_ZLP = FALSE;
      usb_write(NULL, 0, CDC_DATA_EP_IN);
    }
    return 0;
  }
  do{
    if(size < margin){
      memcpy(&(tx_packet[sizeof(tx_packet) - margin]), buf, size);
      written += size;
      margin -= size;
      break;
    }
    memcpy(&(tx_packet[sizeof(tx_packet) - margin]), buf, margin);
    buf += margin;
    size -= margin;
    for(retry = 0; retry < 200; ++retry){ // timeout is approximately 1ms.
      if(usb_write(tx_packet, sizeof(tx_packet), CDC_DATA_EP_IN)){
        if(sizeof(tx_packet) == CDC_DATA_EP_IN_PACKET_SIZE){
          require_ZLP = TRUE;
        }
        break;
      }
      wait_us(5);
    }
    written += margin;
    margin = sizeof(tx_packet) - TX_BUF_HEADER;
  }while(size);
  return written;
}

#ifdef CDC_IS_REPLACED_BY_FTDI
static __bit cdc_rx_need_skip = 1;

static u16 cdc_rx_size(){
  
  // The first byte is a control byte.
  u16 stored = usb_count_ep_out(CDC_DATA_EP_OUT); 
  if(cdc_rx_need_skip){
    switch(stored){
      //case 1:
        //usb_skip(1, CDC_DATA_EP_OUT);
      case 0:
        return 0;
      default:
        //usb_skip(1, CDC_DATA_EP_OUT);
        //stored--;
        cdc_rx_need_skip = 0;
    }
  }
  // The first byte has been already skipped.
  return stored;
}

u16 cdc_rx(u8 *buf, u16 size){
  // The first byte is a control byte.
  if(cdc_rx_size() <= size){
    cdc_rx_need_skip = 1;
  }
  return usb_read(buf, size, CDC_DATA_EP_OUT);
}
#else

static u16 cdc_rx_size(){
  return usb_count_ep_out(CDC_DATA_EP_OUT);
}

u16 cdc_rx(u8 *buf, u16 size){
  return usb_read(buf, size, CDC_DATA_EP_OUT);
}
#endif

void usb_CDC_req(){
  switch(ep0_setup.bRequest){
#ifdef CDC_IS_REPLACED_BY_FTDI
    case PORT_RESET:
    case SET_FLOW_CTRL:
    case SET_EVENT_CHAR:
    case SET_ERROR_CHAR:
    case SET_LATENCY_TIMER:
    case ERACE_EEPROM:
      ep0_request_completed = TRUE;
      break;
    case MODEM_CTRL:
      set_line_state(ep0_setup.wValue.c[LSB] & (CDC_RTS | CDC_DTR));
      ep0_request_completed = TRUE;
      break;
    case SET_BAUD_RATE: {
      // TODO: bit16 support?
      unsigned int divisor;
      switch(divisor = ((ep0_setup.wValue.c[MSB] & 0xC0) >> 6)){
        case 0x01:
          divisor = 4;
          break;
        case 0x03:
          divisor = 1;
          break;
      }
      divisor += (ep0_setup.wValue.i & 0x3FFF) << 3;
      uart_line_coding.baudrate.i = le_u32(24000000UL / divisor);
      ep0_request_completed = TRUE;
      break;
    }
    case SET_DATA:
      // USB is little endian
      uart_line_coding.stopbit = (ep0_setup.wValue.c[MSB] >> 3) & 0x07;
      uart_line_coding.parity = ep0_setup.wValue.c[MSB] & 0x07;
      uart_line_coding.databit = ep0_setup.wValue.c[LSB];
      ep0_request_completed = TRUE;
      break;
    case GET_MODEM_STATUS:
      if(ep0_setup.wLength.c[LSB] <= 2){
        ep0_data_buf[0] = 0x70;
        ep0_data_buf[1] = 0x00;
        ep0_register_data(ep0_data_buf,
            ep0_setup.wLength.c[LSB]);
        usb_ep0_status = EP_TX;
        ep0_request_completed = TRUE;
      }
      break;
    case GET_LATENCY_TIMER:
      //TODO: ep0_register_data(hoge, 1);
      usb_ep0_status = EP_TX;
      //ep0_request_completed = TRUE;
      break;
    case READ_EEPROM:
      ep0_register_data((u8 *)(&ftdi_rom[(ep0_setup.wIndex.i << 1)]),
          ep0_setup.wLength.c[LSB]);
      usb_ep0_status = EP_TX;
      ep0_request_completed = TRUE;
      break;
    case WRITE_EEPROM:
      //TODO: ep0_reserve_data(ep0_data_buf, ep0_setup.wLength.c[LSB], NULL);
      usb_ep0_status = EP_RX;
      //ep0_request_completed = TRUE;
      break;
#else
    case SEND_ENCAPSULATED_COMMAND:
      cdc_Send_Encapsulated_Command();
      break;
    case GET_ENCAPSULATED_RESPONSE:
      cdc_Get_Encapsulated_Command();
      break;
    case SET_LINE_CODING:
      cdc_Set_Line_Coding();
      break;
    case GET_LINE_CODING:
      cdc_Get_Line_Coding();
      break;
    case SET_CONTROL_LINE_STATE:
      cdc_Set_ControlLine_State();
      break;
    case SEND_BREAK:
      cdc_Send_Break();
      break;
#endif
  }
}
