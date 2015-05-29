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
#include <stdlib.h>

#include "main.h"
#include "util.h"

#include "c8051f380.h"
#include "f38x_usb.h"
#include "gpib.h"

volatile __xdata u32 global_ms = 0;
volatile __xdata u32 tickcount = 0;
volatile __xdata u8 sys_state = 0;
volatile u8 timeout_10ms = 0;

void sysclk_init();
void port_init();
void timer_init();

#ifdef USE_ASM_FOR_SFR_MANIP
#define led_on(i)     {__asm orl _P0,SHARP  (0x01 << (i - 1)) __endasm; }
#define led_off(i)    {__asm anl _P0,SHARP ~(0x01 << (i - 1)) __endasm; }
#define led_toggle(i) {__asm xrl _P0,SHARP  (0x01 << (i - 1)) __endasm; }
#else
#define led_on(i)     (P0 |=  (0x01 << (i - 1)))
#define led_off(i)    (P0 &= ~(0x01 << (i - 1)))
#define led_toggle(i) (P0 ^=  (0x01 << (i - 1)))
#endif

void main() {
  sysclk_init(); // Initialize oscillator
  wait_ms(1000);
  port_init(); // Initialize crossbar and GPIO

  gpib_init();

  timer_init();
  
  EA = 1; // Global Interrupt enable

  usb0_init();

  while (1) {
    gpib_polling();
    usb_polling();

    if(sys_state & SYS_PERIODIC_ACTIVE){
      sys_state &= ~SYS_PERIODIC_ACTIVE;
      led_toggle(1);
    }

    (sys_state & SYS_GPIB_CONTROLLER) ? led_on(2) : led_off(2);
    (sys_state & SYS_GPIB_TALKING) ? led_on(3) : led_off(3);
    (sys_state & SYS_GPIB_LISTENING) ? led_on(4) : led_off(4);
  }
}

// System clock selections (SFR CLKSEL)
#define SYS_INT_OSC              0x00        // Select to use internal oscillator
#define SYS_4X_MUL               0x03        // Select to use internal oscillator
#define SYS_EXT_OSC              0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2             0x02

// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK             0x00        // Select 4x clock multiplier, for USB Full Speed
#define USB_INT_OSC_DIV_2        0x10        // See Data Sheet section 13. Oscillators
#define USB_EXT_OSC              0x20
#define USB_EXT_OSC_DIV_2        0x30
#define USB_EXT_OSC_DIV_3        0x40
#define USB_EXT_OSC_DIV_4        0x50

void sysclk_init(){
  REF0CN = 0x07;
  
  // Configure internal oscillator for its maximum frequency and enable missing clock detector
  OSCICN |= 0x03;

#ifdef _USB_LOW_SPEED_
  CLKSEL  = SYS_INT_OSC; // Select System clock
  CLKSEL |= USB_INT_OSC_DIV_2; // Select USB clock
#else
  // Select internal oscillator as input to clock multiplier
  CLKMUL  = 0x00;
  CLKMUL |= 0x80; // Enable clock multiplier
  wait_ms(1); // Delay for clock multiplier to begin
  CLKMUL |= 0xC0; // Initialize the clock multiplier
  wait_ms(1);
  while(!(CLKMUL & 0x20)); // Wait for multiplier to lock
  CLKSEL = SYS_4X_MUL;
  CLKSEL |= USB_4X_CLOCK; // Select USB clock
#endif  /* _USB_LOW_SPEED_ */
}


void port_init() {
  
  // Default port state
  // Pn = 1 (High), PnMDIN = 1 (not analog), PnMDOUT = 0 (open-drain) => Hi-Z
  
  // P0
  // 0-7 => GPIO
  P0MDIN = 0xFF;
  P0MDOUT = 0x0F; // 0-3 => push-pull, 4-7 => open-drain
  P0 = 0xF0;
  P0SKIP = 0xFF;
  
  // P1 => GPIO
  P1MDIN = 0xFF;
  P1MDOUT = 0x00; // open-drain
  P1 = 0xFF;
  P1SKIP = 0xFF;
  
  // P2 = GPIO
  P2MDIN = 0xFF; 
  P2MDOUT = 0x00; // open-drain
  P2 = 0xFF;
  P2SKIP = 0xFF;
  
  // P3
  P3MDIN = 0xFF;
  P3MDOUT = 0xFF;
  P3 = 0x00;

  XBR0 = 0x00;
  XBR2 = 0x00;
  XBR1 = 0x40;  // Enable crossbar
}

void timer_init(){
  TMR3CN = 0x00;    // Stop Timer3; Clear TF3;
  CKCON &= ~0xC0;   // Timer3 clocked based on T3XCLK;
  TMR3RL = (0x10000 - (SYSCLK/12/100));  // Re-initialize reload value (100Hz, 10ms)
  TMR3 = 0xFFFF;    // Set to reload immediately
  EIE1 |= 0x80;     // Enable Timer3 interrupts(ET3)
  TMR3CN |= 0x04;   // Start Timer3(TR3)
}

/**
 * interrupt routine invoked when timer3 overflow
 * 
 */
void interrupt_timer3() __interrupt (INTERRUPT_TIMER3) {
  TMR3CN &= ~0x80; // Clear interrupt
  global_ms += 10;
  tickcount++;
  timeout_10ms++;
  sys_state |= SYS_PERIODIC_ACTIVE;
}

unsigned char _sdcc_external_startup(){
  PCA0MD &= ~0x40; ///< Disable Watchdog timer
  return 0;
}
