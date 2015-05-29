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

#ifndef __CDC_H__
#define __CDC_H__

#include "type.h"
//#define CDC_IS_REPLACED_BY_FTDI

/**
 * Header Functional Descriptor
 */
typedef struct {
  BYTE bLength;
  BYTE bDescriptorType;
  BYTE bDescriptorSubtype;
  WORD bcdCDC;
} cdc_header_func_descriptor_t;

/**
 * Call Management Functional Descriptor
 */
typedef struct {
  BYTE bLength;
  BYTE bDescriptorType;
  BYTE bDescriptorSubtype;
  BYTE bmCapabilities;
  BYTE bDataInterface;
} cdc_call_man_func_descriptor_t;

/**
 * Abstract Control Management Functional Descriptor
 */
typedef struct {
  BYTE bLength;
  BYTE bDescriptorType;
  BYTE bDescriptorSubtype;
  BYTE bmCapabilities;
} cdc_abst_control_mana_descriptor_t;

/**
 * Union Functional Descriptor
 */
typedef struct {
  BYTE bLength;
  BYTE bDescriptorType;
  BYTE bDescriptorSubtype;
  BYTE bMasterInterface;
  BYTE bSlaveInterface0;
} cdc_union_func_descriptor_t;

typedef struct {
  cdc_header_func_descriptor_t header;
  cdc_abst_control_mana_descriptor_t abst_control_mana;
  cdc_union_func_descriptor_t union_func;
  cdc_call_man_func_descriptor_t call_ma;
} cdc_descriptor_t;

#define cdc_config_length() sizeof(cdc_descriptor_t)

typedef struct {
  DWORD_t baudrate;
  BYTE stopbit;
  BYTE parity;
  BYTE databit;
} cdc_line_coding_t;

// Descriptor type (Class specific)
#define DSC_TYPE_CS_INTERFACE       0x24
#define DSC_SUBTYPE_CS_HEADER_FUNC  0x00
#define DSC_SUBTYPE_CS_CALL_MAN     0x01
#define DSC_SUBTYPE_CS_ABST_CNTRL   0x02
#define DSC_SUBTYPE_CS_UNION_FUNC   0x06

// CDC ACM class specifc requests
#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_RESPONSE 0x01
#define SET_LINE_CODING           0x20
#define GET_LINE_CODING           0x21
#define SET_CONTROL_LINE_STATE    0x22
#define SEND_BREAK                0x23

void usb_CDC_req();
void cdc_polling();
u16 cdc_tx(u8 *buf, u16 size);
u16 cdc_rx(u8 *buf, u16 size);

#define CDC_COM_EP_IN  1
#define CDC_DATA_EP_IN  2
#define CDC_DATA_EP_OUT 2

#ifndef CONCAT2_
#define CONCAT2_(a,b) (a ## b)
#endif
#ifndef CONCAT2
#define CONCAT2(a,b) CONCAT2_(a, b) 
#endif

#define CDC_DATA_EP_IN_PACKET_SIZE  (CONCAT2(PACKET_SIZE_EP, CDC_DATA_EP_IN))
#define CDC_DATA_EP_OUT_PACKET_SIZE  (CONCAT2(PACKET_SIZE_EP, CDC_DATA_EP_OUT))

#endif
