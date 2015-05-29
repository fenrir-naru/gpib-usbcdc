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

#ifndef  _USB_ISR_H_
#define  _USB_ISR_H_

#include "c8051F380.h"
#include "main.h"
#include <string.h>

// Define wIndex bitmaps
#define is_EP_IN(ep_address) ((ep_address) & (0x80))
#define EP_NUMBER(ep_address) ((ep_address) & ~(0x80))
#define IN_EP1              0x81        // Index values used by Set and Clear feature
#define OUT_EP1             0x01        // commands for Endpoint_Halt
#define IN_EP2              0x82
#define OUT_EP2             0x02
#define IN_EP3              0x83
#define OUT_EP3             0x03

// Define wValue bitmaps for Standard Feature Selectors
#define DEVICE_REMOTE_WAKEUP    0x01        // Remote wakeup feature(not used)
#define ENDPOINT_HALT           0x00        // Endpoint_Halt feature selector

// Device Request Direction (bmRequestType bit7)
#define DRD_MASK    0x80    // Mask for device request direction
#define DRD_OUT     0x00    // OUT: host to device
#define DRD_IN      0x80    // IN:  device to host

// Device Request Type (bmRequestType bit 6-5)
#define DRT_MASK    0x60    // Mask for device request type
#define DRT_STD     0x00    // Standard device request
#define DRT_CLASS   0x20    // Class specific request
#define DRT_VENDOR  0x40    // Vendor specific request

// Device Request Recipient (bmRequestType bit4-0)
#define DRR_MASK      0x1F    // Mask for device request recipient
#define DRR_DEVICE    0x00    // Device
#define DRR_INTERFACE 0x01    // Interface
#define DRR_ENDPOINT  0x02    // Endpoint

// Define bmRequestType bitmaps
#define OUT_DEVICE    (DRD_OUT | DRT_STD | DRR_DEVICE)
#define IN_DEVICE     (DRD_IN  | DRT_STD | DRR_DEVICE)
#define OUT_INTERFACE (DRD_OUT | DRT_STD | DRR_INTERFACE)
#define IN_INTERFACE  (DRD_IN  | DRT_STD | DRR_INTERFACE)
#define OUT_ENDPOINT  (DRD_OUT | DRT_STD | DRR_ENDPOINT)
#define IN_ENDPOINT   (DRD_IN  | DRT_STD | DRR_ENDPOINT)

#define OUT_CL_INTERFACE  (DRD_OUT | DRT_CLASS | DRR_INTERFACE)
#define IN_CL_INTERFACE   (DRD_IN  | DRT_CLASS | DRR_INTERFACE)

#define OUT_VR_INTERFACE  (DRD_OUT | DRT_VENDOR | DRR_INTERFACE)
#define IN_VR_INTERFACE   (DRD_IN  | DRT_VENDOR | DRR_INTERFACE)

// Setup Packet Type Definition
typedef __xdata struct {
   BYTE bmRequestType; // Request recipient, type, and direction
   BYTE bRequest;      // Specific standard request number
   WORD_t wValue;      // varies according to request
   WORD_t wIndex;      // varies according to request
   WORD_t wLength;     // Number of bytes to transfer
} ep0_setup_t;
extern ep0_setup_t ep0_setup;

typedef __xdata struct {
  BYTE *buf;
  unsigned int size;
  __code void (*callback)();
} ep0_data_t;
extern ep0_data_t ep0_data;

extern volatile __bit ep0_request_completed;

#define ep0_register_data(_ptr, _size) { \
  ep0_data.buf = _ptr; \
  ep0_data.size = _size; \
  ep0_data.callback = NULL; \
}
#define ep0_reserve_data(_ptr, _size, _func) { \
  ep0_data.buf = _ptr; \
  ep0_data.size = _size; \
  ep0_data.callback = _func; \
}

// Define Endpoint States
#define EP_IDLE                 0x00        // This signifies Endpoint Idle State
#define EP_TX                   0x01        // Endpoint Transmit State
#define EP_RX                   0x02        // Endpoint Receive State
#define EP_HALT                 0x03        // Endpoint Halt State (return stalls)
#define EP_STALL                0x04        // Endpoint Stall (send procedural stall next status phase)
#define EP_ADDRESS              0x05        // Endpoint Address (change FADDR during next status phase)

#define EP_OWNED_BY_USER        0x80        // Endpoint state owned by user
#define ep_strip_owner(state) ((state) & ~EP_OWNED_BY_USER)

extern BYTE __xdata usb_ep_stat[7];
#define DIR_IN 0x00
#define DIR_OUT 0x04
#define usb_ep0_status usb_ep_stat[0]
#define usb_ep_status(dir, ep_number) \
usb_ep_stat[((dir == DIR_OUT) ? (3 + ep_number) : (ep_number))]

unsigned int usb_write(BYTE* ptr_buf, unsigned int count, unsigned char index);
unsigned int usb_fill(BYTE buf, unsigned int count, unsigned char index);
unsigned int usb_read(BYTE* ptr_buf, unsigned int count, unsigned char index);
unsigned int usb_skip(unsigned int count, unsigned char index);
void usb_flush(unsigned char index);
unsigned int usb_count_ep_out(unsigned char index);

void usb_status_lock(unsigned char dir, unsigned char ep_index);
void usb_status_unlock(unsigned char dir, unsigned char ep_index);
void usb_stall(unsigned char dir, unsigned char ep_index, __code void(*)());
void usb_clear_halt(unsigned char dir, unsigned char ep_index);
void usb0_init();

// Define device states
typedef __xdata enum {
  DEV_ATTACHED = 0x00,    // Device is in Attached State
  DEV_POWERED = 0x01,     // Device is in Powered State
  DEV_DEFAULT = 0x02,     // Device is in Default State
  DEV_ADDRESS = 0x03,     // Device is in Addressed State
  DEV_CONFIGURED = 0x04,  // Device is in Configured State
  DEV_SUSPENDED = 0x05,   // Device is in Suspended State
} usb_state_t;
extern usb_state_t usb_state;

typedef enum {
  USB_INACTIVE,
  USB_CABLE_CONNECTED,
  USB_CDC_READY,
  USB_CDC_ACTIVE,
} usb_mode_t;
extern volatile usb_mode_t usb_mode;

extern volatile __xdata BYTE usb_frame_num;

void usb_polling();

void usb_isr() __interrupt (INTERRUPT_USB0);

#ifdef USE_ASM_FOR_SFR_MANIP
#define CRITICAL_USB0(func) \
{ \
  {__asm anl _EIE1,SHARP ~0x02 __endasm; } \
  { \
    func; \
  } \
  {__asm orl _EIE1,SHARP 0x02 __endasm; } \
}
#else
#define CRITICAL_USB0(func) \
{ \
  EIE1 &= ~(0x02); \
  { \
    func; \
  } \
  EIE1 |= (0x02); \
}
#endif

#endif /* _USB_ISR_H_ */
