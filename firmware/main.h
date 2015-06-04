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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "type.h"

//#define _USB_LOW_SPEED_                      // Change this comment to make Full/Low speed

// SYSCLK frequency in Hz
#define SYSCLK    48000000UL

extern volatile __xdata u32 global_ms;
extern volatile __xdata u32 tickcount;

extern volatile __xdata u8 sys_state;
#define SYS_PERIODIC_ACTIVE 0x01
#define SYS_GPIB_CONTROLLER 0x10
#define SYS_GPIB_TALKED 0x20
#define SYS_GPIB_LISTENED 0x40

extern volatile u8 timeout_10ms;

// Define Endpoint Packet Sizes
#ifdef _USB_LOW_SPEED_
// This value can be 8,16,32,64 depending on device speed, see USB spec
#define PACKET_SIZE_EP0     0x40
#else
#define PACKET_SIZE_EP0     0x40
#endif /* _USB_LOW_SPEED_ */ 

// Can range 0 - 1024 depending on data and transfer type
#define PACKET_SIZE_EP1 0x0010
#define PACKET_SIZE_EP2 0x0040
#define PACKET_SIZE_EP3 0x0040

#define CRITICAL_GLOBAL(func) \
{ \
  EA = 0; \
  { \
    func; \
  } \
  EA = 1; \
}


#if (defined(__SDCC_REVISION) && (__SDCC_REVISION > 4818)) \
    || (defined(SDCC_REVISION) && (SDCC_REVISION > 4818)) // upper than sdcc-2.7.0? 
#define USE_ASM_FOR_SFR_MANIP
#define SHARP #
#endif

#endif      /* __MAIN_H__ */
