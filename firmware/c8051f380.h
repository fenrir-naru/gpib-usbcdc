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

/*-----------------------------------------------------------------------------
C8051F380.H

Header file for Silabs C8051F380/1/2/3/4/5/6 and C8051F387 devices.
-----------------------------------------------------------------------------*/


#ifndef __C8051F380_H__
#define __C8051F380_H__

/* SFRPage 0x00 and SFRPage 0x0F Registers */
__sfr __at (0x80) P0            ; /* Port 0 Latch                             */
__sfr __at (0x81) SP            ; /* Stack Pointer                            */
__sfr __at (0x82) DPL           ; /* Data Pointer Low                         */
__sfr __at (0x83) DPH           ; /* Data Pointer High                        */
__sfr __at (0x84) EMI0TC        ; /* External Memory Interface Timing         */
__sfr __at (0x85) EMI0CF        ; /* External Memory Interface Configuration  */
__sfr __at (0x86) OSCLCN        ; /* Internal Low-Frequency Oscillator Control */
__sfr __at (0x87) PCON          ; /* Power Control                            */
__sfr __at (0x88) TCON          ; /* Timer/Counter Control                    */
__sfr __at (0x89) TMOD          ; /* Timer/Counter Mode                       */
__sfr __at (0x8A) TL0           ; /* Timer/Counter 0 Low                      */
__sfr __at (0x8B) TL1           ; /* Timer/Counter 1 Low                      */
__sfr __at (0x8C) TH0           ; /* Timer/Counter 0 High                     */
__sfr __at (0x8D) TH1           ; /* Timer/Counter 1 High                     */
__sfr __at (0x8E) CKCON         ; /* Clock Control                            */
__sfr __at (0x8F) PSCTL         ; /* Program Store R/W Control                */
__sfr __at (0x90) P1            ; /* Port 1 Latch                             */
__sfr __at (0x91) TMR3CN        ; /* Timer/Counter 3 Control         (page 0) */
__sfr __at (0x92) TMR3RLL       ; /* Timer/Counter 3 Reload Low      (page 0) */
__sfr __at (0x93) TMR3RLH       ; /* Timer/Counter 3 Reload High     (page 0) */
__sfr __at (0x94) TMR3L         ; /* Timer/Counter 3 Low                      */
__sfr __at (0x95) TMR3H         ; /* Timer/Counter 3 High                     */
__sfr __at (0x96) USB0ADR       ; /* USB0 Indirect Address Register           */
__sfr __at (0x97) USB0DAT       ; /* USB0 Data Register                       */
__sfr __at (0x91) TMR4CN        ; /* Timer/Counter 4 Control         (page F) */
__sfr __at (0x92) TMR4RLL       ; /* Timer/Counter 4 Reload Low      (page F) */
__sfr __at (0x93) TMR4RLH       ; /* Timer/Counter 4 Reload High     (page F) */
__sfr __at (0x98) SCON0         ; /* UART0 Control                            */
__sfr __at (0x99) SBUF0         ; /* UART0 Data Buffer                        */
__sfr __at (0x9A) CPT1CN        ; /* Comparator 1 Control                     */
__sfr __at (0x9B) CPT0CN        ; /* Comparator 0 Control                     */
__sfr __at (0x9C) CPT1MD        ; /* Comparator 1 Mode                        */
__sfr __at (0x9D) CPT0MD        ; /* Comparator 0 Mode                        */
__sfr __at (0x9E) CPT1MX        ; /* Comparator 1 Mux                         */
__sfr __at (0x9F) CPT0MX        ; /* Comparator 0 Mux                         */
__sfr __at (0xA0) P2            ; /* Port 2 Latch                             */
__sfr __at (0xA1) SPI0CFG       ; /* SPI0 Configuration                       */
__sfr __at (0xA2) SPI0CKR       ; /* SPI0 Clock rate control                  */
__sfr __at (0xA3) SPI0DAT       ; /* SPI0 Data Buffer                         */
__sfr __at (0xA4) P0MDOUT       ; /* Port 0 Output Mode                       */
__sfr __at (0xA5) P1MDOUT       ; /* Port 1 Output Mode                       */
__sfr __at (0xA6) P2MDOUT       ; /* Port 2 Output Mode                       */
__sfr __at (0xA7) P3MDOUT       ; /* Port 3 Output Mode                       */
__sfr __at (0xA8) IE            ; /* Interrupt Enable                         */
__sfr __at (0xA9) CLKSEL        ; /* Clock Select                             */
__sfr __at (0xAA) EMI0CN        ; /* EMIF control                             */
__sfr __at (0xAA) _XPAGE        ; /* XDATA/PDATA PAGE                         */
__sfr __at (0xAC) SBCON1        ; /* UART1 Baud Rate Generator Control        */
__sfr __at (0xAE) P4MDOUT       ; /* Port 4 Mode                              */
__sfr __at (0xAF) PFE0CN        ; /* Prefetch Engine Control                  */
__sfr __at (0xB0) P3            ; /* Port 3 Latch                             */
__sfr __at (0xB1) OSCXCN        ; /* External Oscillator Control              */
__sfr __at (0xB2) OSCICN        ; /* Internal Oscillator Control              */
__sfr __at (0xB3) OSCICL        ; /* Internal Oscillator Calibration          */
__sfr __at (0xB4) SBRLL1        ; /* UART1 Baud Rate Generator Low            */
__sfr __at (0xB5) SBRLH1        ; /* UART1 Baud Rate Generator High           */
__sfr __at (0xB6) FLSCL         ; /* Flash Scale                              */
__sfr __at (0xB7) FLKEY         ; /* Flash Lock and Key                       */
__sfr __at (0xB8) IP            ; /* Interrupt Priority                       */
__sfr __at (0xB9) CLKMUL        ; /* Clock Multiplier                (page 0) */
__sfr __at (0xBA) AMX0N         ; /* AMUX0 Negative Channel Select            */
__sfr __at (0xBB) AMX0P         ; /* AMUX0 Positive Channel Select            */
__sfr __at (0xBC) ADC0CF        ; /* ADC0 Configuration              (page 0) */
__sfr __at (0xBD) ADC0L         ; /* ADC0 Low                                 */
__sfr __at (0xBE) ADC0H         ; /* ADC0 High                                */
__sfr __at (0xBF) SFRPAGE       ; /* SFR Page Select                          */
__sfr __at (0xB9) SMBTC         ; /* SMBus0/1 Timing Control         (page F) */
__sfr __at (0xC0) SMB0CN        ; /* SMBus0 Control                  (page 0) */
__sfr __at (0xC1) SMB0CF        ; /* SMBus0 Configuration            (page 0) */
__sfr __at (0xC2) SMB0DAT       ; /* SMBus0 Data                     (page 0) */
__sfr __at (0xC3) ADC0GTL       ; /* ADC0 Greater-Than Compare Low            */
__sfr __at (0xC4) ADC0GTH       ; /* ADC0 Greater-Than Compare High           */
__sfr __at (0xC5) ADC0LTL       ; /* ADC0 Less-Than Compare Word Low          */
__sfr __at (0xC6) ADC0LTH       ; /* ADC0 Less-Than Compare Word High         */
__sfr __at (0xC7) P4            ; /* Port 4 Latch                             */
__sfr __at (0xC0) SMB1CN        ; /* SMBus1 Control                  (page F) */
__sfr __at (0xC1) SMB1CF        ; /* SMBus1 Configuration            (page F) */
__sfr __at (0xC2) SMB1DAT       ; /* SMBus1 Data                     (page F) */
__sfr __at (0xC8) TMR2CN        ; /* Timer/Counter 2 Control         (page 0) */
__sfr __at (0xC9) REG01CN       ; /* Regulator Control               (page 0) */
__sfr __at (0xCA) TMR2RLL       ; /* Timer/Counter 2 Reload Low      (page 0) */
__sfr __at (0xCB) TMR2RLH       ; /* Timer/Counter 2 Reload High     (page 0) */
__sfr __at (0xCC) TMR2L         ; /* Timer/Counter 2 Low             (page 0) */
__sfr __at (0xCD) TMR2H         ; /* Timer/Counter 2 High            (page 0) */
__sfr __at (0xCE) SMB0ADM       ; /* SMBus0 Address Mask             (page 0) */
__sfr __at (0xCF) SMB0ADR       ; /* SMBus0 Address                  (page 0) */
__sfr __at (0xC8) TMR5CN        ; /* Timer/Counter 5 Control         (page F) */
__sfr __at (0xCA) TMR5RLL       ; /* Timer/Counter 5 Reload Low      (page F) */
__sfr __at (0xCB) TMR5RLH       ; /* Timer/Counter 5 Reload High     (page F) */
__sfr __at (0xCC) TMR5L         ; /* Timer/Counter 5 Low             (page F) */
__sfr __at (0xCD) TMR5H         ; /* Timer/Counter 5 High            (page F) */
__sfr __at (0xCE) SMB1ADM       ; /* SMBus1 Address Mask             (page F) */
__sfr __at (0xCF) SMB1ADR       ; /* SMBus1 Address                  (page F) */
__sfr __at (0xD0) PSW           ; /* Program Status Word                      */
__sfr __at (0xD1) REF0CN        ; /* Voltage Reference Control                */
__sfr __at (0xD2) SCON1         ; /* UART1 Control                            */
__sfr __at (0xD3) SBUF1         ; /* UART1 Data Buffer                        */
__sfr __at (0xD4) P0SKIP        ; /* Port 0 Skip                              */
__sfr __at (0xD5) P1SKIP        ; /* Port 1 Skip                              */
__sfr __at (0xD6) P2SKIP        ; /* Port 2 Skip                              */
__sfr __at (0xD7) USB0XCN       ; /* USB0 Transceiver Control                 */
__sfr __at (0xD8) PCA0CN        ; /* PCA0 Control                             */
__sfr __at (0xD9) PCA0MD        ; /* PCA0 Mode                                */
__sfr __at (0xDA) PCA0CPM0      ; /* PCA Module 0 Mode Register               */
__sfr __at (0xDB) PCA0CPM1      ; /* PCA Module 1 Mode Register               */
__sfr __at (0xDC) PCA0CPM2      ; /* PCA Module 2 Mode Register               */
__sfr __at (0xDD) PCA0CPM3      ; /* PCA Module 3 Mode Register               */
__sfr __at (0xDE) PCA0CPM4      ; /* PCA Module 4 Mode Register               */
__sfr __at (0xDF) P3SKIP        ; /* Port 3 Skip                              */
__sfr __at (0xE0) ACC           ; /* Accumulator                              */
__sfr __at (0xE1) XBR0          ; /* Port I/O Crossbar Control 0              */
__sfr __at (0xE2) XBR1          ; /* Port I/O Crossbar Control 1              */
__sfr __at (0xE3) XBR2          ; /* Port I/O Crossbar Control 2              */
__sfr __at (0xE4) IT01CF        ; /* INT0/INT1 Configuration         (page 0) */
__sfr __at (0xE5) SMOD1         ; /* UART1 Mode                               */
__sfr __at (0xE6) EIE1          ; /* Extended Interrupt Enable 2              */
__sfr __at (0xE7) EIE2          ; /* Extended Interrupt Enable 2              */
__sfr __at (0xE4) CKCON1        ; /* Clock Control 1                 (page F) */
__sfr __at (0xE8) ADC0CN        ; /* ADC0 Control                             */
__sfr __at (0xE9) PCA0CPL1      ; /* PCA0 Capture 2 Low                       */
__sfr __at (0xEA) PCA0CPH1      ; /* PCA0 Capture 2 High                      */
__sfr __at (0xEB) PCA0CPL2      ; /* PCA0 Capture 3 Low                       */
__sfr __at (0xEC) PCA0CPH2      ; /* PCA0 Capture 3 High                      */
__sfr __at (0xED) PCA0CPL3      ; /* PCA0 Capture 4 Low                       */
__sfr __at (0xEE) PCA0CPH3      ; /* PCA0 Capture 4 High                      */
__sfr __at (0xEF) RSTSRC        ; /* Reset Source Configuration/Status        */
__sfr __at (0xF0) B             ; /* B Register                               */
__sfr __at (0xF1) P0MDIN        ; /* Port 0 Input Mode                        */
__sfr __at (0xF2) P1MDIN        ; /* Port 1 Input Mode                        */
__sfr __at (0xF3) P2MDIN        ; /* Port 2 Input Mode                        */
__sfr __at (0xF4) P3MDIN        ; /* Port 3 Input Mode                        */
__sfr __at (0xF5) P4MDIN        ; /* Port 4 Input Mode                        */
__sfr __at (0xF6) EIP1          ; /* External Interrupt Priority 1            */
__sfr __at (0xF7) EIP2          ; /* External Interrupt Priority 2            */
__sfr __at (0xF8) SPI0CN        ; /* SPI0 Control                             */
__sfr __at (0xF9) PCA0L         ; /* PCA0 Counter Low                         */
__sfr __at (0xFA) PCA0H         ; /* PCA0 Counter High                        */
__sfr __at (0xFB) PCA0CPL0      ; /* PCA0 Capture 0 Low                       */
__sfr __at (0xFC) PCA0CPH0      ; /* PCA0 Capture 0 High                      */
__sfr __at (0xFD) PCA0CPL4      ; /* PCA0 Capture 4 Low                       */
__sfr __at (0xFE) PCA0CPH4      ; /* PCA0 Capture 4 High                      */
__sfr __at (0xFF) VDM0CN        ; /* VDD Monitor Control                      */

/* 16-Bit Register definitions */
__sfr16 __at (0x8382) DP        ; /* Data pointer                             */
__sfr16 __at (0x9392) TMR3RL    ; /* Timer 3 reload register                  */
__sfr16 __at (0x9392) TMR4RL    ; /* Timer 4 reload register         (page F) */
__sfr16 __at (0x9594) TMR3      ; /* Timer 3 register                         */
__sfr16 __at (0x9594) TMR4      ; /* Timer 4 register                (page F) */
__sfr16 __at (0xB5B4) SBRL1     ; /* UART1 Baud Rate Generator                */
__sfr16 __at (0xBEBD) ADC0      ; /* ADC 0 data                               */
__sfr16 __at (0xC4C3) ADC0GT    ; /* ADC 0 greater than compare               */
__sfr16 __at (0xC6C5) ADC0LT    ; /* ADC 0 less than compare                  */
__sfr16 __at (0xCBCA) TMR2RL    ; /* Timer\Counter 2 reload data              */
__sfr16 __at (0xCBCA) TMR5RL    ; /* Timer\Counter 5 reload data     (page F) */
__sfr16 __at (0xCDCC) TMR2      ; /* Timer\Counter 2 data                     */
__sfr16 __at (0xCDCC) TMR5      ; /* Timer\Counter 5 data                     */
__sfr16 __at (0xEAE9) PCA0CP1   ; /* PCA module 1 capture/compare data        */
__sfr16 __at (0xECEB) PCA0CP2   ; /* PCA module 2 capture/compare data        */
__sfr16 __at (0xEEED) PCA0CP3   ; /* PCA module 3 capture/compare data        */
__sfr16 __at (0xFAF9) PCA0      ; /* PCA 0 counter data                       */
__sfr16 __at (0xFCFB) PCA0CP0   ; /* PCA 0 capture data                       */
__sfr16 __at (0xFEFD) PCA0CP4   ; /* PCA 4 capture data                       */

/* Bit Definitions for SFRPage 0x00 and SFRPage 0x0F Registers */
/* TCON 0x88 */
__sbit __at (0x8F) TF1          ; /* Timer 1 Overflow Flag                    */
__sbit __at (0x8E) TR1          ; /* Timer 1 On/Off Control                   */
__sbit __at (0x8D) TF0          ; /* Timer 0 Overflow Flag                    */
__sbit __at (0x8C) TR0          ; /* Timer 0 On/Off Control                   */
__sbit __at (0x8B) IE1          ; /* Ext. Interrupt 1 Edge Flag               */
__sbit __at (0x8A) IT1          ; /* Ext. Interrupt 1 Type                    */
__sbit __at (0x89) IE0          ; /* Ext. Interrupt 0 Edge Flag               */
__sbit __at (0x88) IT0          ; /* Ext. Interrupt 0 Type                    */

/* SCON0 0x98 */
__sbit __at (0x9F) S0MODE       ; /* Serial Port 0 Operation Mode             */
                                   /* Bit 6 unused                           */
__sbit __at (0x9D) MCE0         ; /* Multiprocessor Communication Enable      */
__sbit __at (0x9C) REN0         ; /* UART0 RX Enable                          */
__sbit __at (0x9B) TB80         ; /* Ninth Transmission Bit                   */
__sbit __at (0x9A) RB80         ; /* Ninth Receive Bit                        */
__sbit __at (0x99) TI0          ; /* UART0 TX Interrupt Flag                  */
__sbit __at (0x98) RI0          ; /* UART0 RX Interrupt Flag                  */

/* IE 0xA8 */
__sbit __at (0xAF) EA           ; /* Global Interrupt Enable                  */
__sbit __at (0xAE) ESPI0        ; /* SPI0 Interrupt Enable                    */
__sbit __at (0xAD) ET2          ; /* Timer 2 Interrupt Enable                 */
__sbit __at (0xAC) ES0          ; /* UART0 Interrupt Enable                   */
__sbit __at (0xAB) ET1          ; /* Timer 1 Interrupt Enable                 */
__sbit __at (0xAA) EX1          ; /* External Interrupt 1 Enable              */
__sbit __at (0xA9) ET0          ; /* Timer 0 Interrupt Enable                 */
__sbit __at (0xA8) EX0          ; /* External Interrupt 0 Enable              */

/* IP 0xB8 */
                                   /* Bit 7 unused                           */
__sbit __at (0xBE) PSPI0        ; /* SPI0 Interrupt Priority                  */
__sbit __at (0xBD) PT2          ; /* Timer 2 Priority                         */
__sbit __at (0xBC) PS0          ; /* UART0 Priority                           */
__sbit __at (0xBB) PT1          ; /* Timer 1 Priority                         */
__sbit __at (0xBA) PX1          ; /* External Interrupt 1 Priority            */
__sbit __at (0xB9) PT0          ; /* Timer 0 Priority                         */
__sbit __at (0xB8) PX0          ; /* External Interrupt 0 Priority            */

/* SMB0CN 0xC0 SFR Page=0 */
__sbit __at (0xC7) MASTER0      ; /* SMBus0 Master/Slave Indicator            */
__sbit __at (0xC6) TXMODE0      ; /* SMBus0 Transmit Mode Indicator           */
__sbit __at (0xC5) STA0         ; /* SMBus0 Start Flag                        */
__sbit __at (0xC4) STO0         ; /* SMBus0 Stop Flag                         */
__sbit __at (0xC3) ACKRQ0       ; /* SMBus0 Acknowledge Request               */
__sbit __at (0xC2) ARBLOST0     ; /* SMBus0 Arbitration Lost Indicator        */
__sbit __at (0xC1) ACK0         ; /* SMBus0 Acknowledge                       */
__sbit __at (0xC0) SI0          ; /* SMBus0 Interrupt Flag                    */

/* SMB1CN 0xC0 SFR Page=F */
__sbit __at (0xC7) MASTER1      ; /* SMBus1 Master/Slave Indicator            */
__sbit __at (0xC6) TXMODE1      ; /* SMBus1 Transmit Mode Indicator           */
__sbit __at (0xC5) STA1         ; /* SMBus1 Start Flag                        */
__sbit __at (0xC4) STO1         ; /* SMBus1 Stop Flag                         */
__sbit __at (0xC3) ACKRQ1       ; /* SMBus1 Acknowledge Request               */
__sbit __at (0xC2) ARBLOST1     ; /* SMBus1 Arbitration Lost Indicator        */
__sbit __at (0xC1) ACK1         ; /* SMBus1 Acknowledge                       */
__sbit __at (0xC0) SI1          ; /* SMBus1 Interrupt Flag                    */

/* TMR2CN 0xC8 SFR Page=0 */
__sbit __at (0xCF) TF2H         ; /* Timer 2 High-Byte Overflow Flag          */
__sbit __at (0xCE) TF2L         ; /* Timer 2 Low-Byte  Overflow Flag          */
__sbit __at (0xCD) TF2LEN       ; /* Timer 2 Low-Byte Flag Enable             */
__sbit __at (0xCC) TF2CEN       ; /* Timer 2 Capture Enable                   */
__sbit __at (0xCB) T2SPLIT      ; /* Timer 2 Split-Mode Enable                */
__sbit __at (0xCA) TR2          ; /* Timer2 Run Enable                        */
__sbit __at (0xC9) T2CSS        ; /* Timer 2 Capture Source Select            */
__sbit __at (0xC8) T2XCLK       ; /* Timer 2 Clk/8 Clock Source               */

/* TMR2CN 0xC8 SFR Page=F */
__sbit __at (0xCF) TF5H         ; /* Timer 5 High-Byte Overflow Flag          */
__sbit __at (0xCE) TF5L         ; /* Timer 5 Low-Byte  Overflow Flag          */
__sbit __at (0xCD) TF5LEN       ; /* Timer 5 Low-Byte Flag Enable             */
                                   /* Bit 4 unused                           */
__sbit __at (0xCB) T5SPLIT      ; /* Timer 5 Split-Mode Enable                */
__sbit __at (0xCA) TR5          ; /* Timer 5 Run Enable                       */
                                   /* Bit 1 unused                           */
__sbit __at (0xC8) T5XCLK       ; /* Timer 5 Clk/8 Clock Source               */

/* PSW 0xD0 */
__sbit __at (0xD7) CY           ; /* Carry Flag                               */
__sbit __at (0xD6) AC           ; /* Auxiliary Carry Flag                     */
__sbit __at (0xD5) F0           ; /* User Flag 0                              */
__sbit __at (0xD4) RS1          ; /* Register Bank Select 1                   */
__sbit __at (0xD3) RS0          ; /* Register Bank Select 0                   */
__sbit __at (0xD2) OV           ; /* Overflow Flag                            */
__sbit __at (0xD1) F1           ; /* User Flag 1                              */
__sbit __at (0xD0) PARITY       ; /* Accumulator Parity Flag                  */

/* PCA0CN 0xD8 */
__sbit __at (0xDF) CF           ; /* PCA0 Counter Overflow Flag               */
__sbit __at (0xDE) CR           ; /* PCA0 Counter Run Control Bit             */
                                   /* Bit 5 unused                           */
__sbit __at (0xDC) CCF4         ; /* PCA0 Module 4 Capture/Compare Flag       */
__sbit __at (0xDB) CCF3         ; /* PCA0 Module 3 Capture/Compare Flag       */
__sbit __at (0xDA) CCF2         ; /* PCA0 Module 2 Capture/Compare Flag       */
__sbit __at (0xD9) CCF1         ; /* PCA0 Module 1 Capture/Compare Flag       */
__sbit __at (0xD8) CCF0         ; /* PCA0 Module 0 Capture/Compare Flag       */

/* ADC0CN 0xE8 */
__sbit __at (0xEF) AD0EN        ; /* ADC0 Enable                              */
__sbit __at (0xEE) AD0TM        ; /* ADC0 Track Mode Bit                      */
__sbit __at (0xED) AD0INT       ; /* ADC0 Conversion Complete Interrupt Flag  */
__sbit __at (0xEC) AD0BUSY      ; /* ADC0 Busy Flag                           */
__sbit __at (0xEB) AD0WINT      ; /* ADC0 Window Compare Interrupt Flag       */
__sbit __at (0xEA) AD0CM2       ; /* ADC0 Start Of Conversion Mode Bit 2      */
__sbit __at (0xE9) AD0CM1       ; /* ADC0 Start Of Conversion Mode Bit 1      */
__sbit __at (0xE8) AD0CM0       ; /* ADC0 Start Of Conversion Mode Bit 0      */

/* SPI0CN 0xF8 */
__sbit __at (0xFF) SPIF         ; /* SPI0 Interrupt Flag                      */
__sbit __at (0xFE) WCOL         ; /* SPI0 Write Collision Flag                */
__sbit __at (0xFD) MODF         ; /* SPI0 Mode Fault Flag                     */
__sbit __at (0xFC) RXOVRN       ; /* SPI0 RX Overrun Flag                     */
__sbit __at (0xFB) NSSMD1       ; /* SPI0 Slave Select Mode 1                 */
__sbit __at (0xFA) NSSMD0       ; /* SPI0 Slave Select Mode 0                 */
__sbit __at (0xF9) TXBMT        ; /* SPI0 TX Buffer Empty Flag                */
__sbit __at (0xF8) SPIEN        ; /* SPI0 Enable                              */

//-----------------------------------------------------------------------------
// Interrupt Priorities
//-----------------------------------------------------------------------------

#define INTERRUPT_INT0             0   // External Interrupt 0
#define INTERRUPT_TIMER0           1   // Timer 0 Overflow
#define INTERRUPT_INT1             2   // External Interrupt 1
#define INTERRUPT_TIMER1           3   // Timer 1 Overflow
#define INTERRUPT_UART0            4   // UART0
#define INTERRUPT_TIMER2           5   // Timer 2 Overflow
#define INTERRUPT_SPI0             6   // SPI0
#define INTERRUPT_SMBUS0           7   // SMBus0 Interface
#define INTERRUPT_USB0           8   // SMBus0 Interface
#define INTERRUPT_ADC0_WINDOW      9   // ADC0 Window Comparison
#define INTERRUPT_ADC0_EOC         10   // ADC0 End Of Conversion
#define INTERRUPT_PCA0            11   // PCA0 Peripheral
#define INTERRUPT_COMPARATOR0     12   // Comparator 0 Comparison
#define INTERRUPT_COMPARATOR1     13   // Comparator 1 Comparison
#define INTERRUPT_TIMER3          14   // Timer 3 Overflow
#define INTERRUPT_VBUS            15   // VBus Interrupt
#define INTERRUPT_UART1           16   // UART1
#define INTERRUPT_SMBUS1           18   // SMBus1 Interface
#define INTERRUPT_TIMER4          19   // Timer 4 Overflow
#define INTERRUPT_TIMER5          20   // Timer 5 Overflow

#endif                                 /* #define __C8051F380_H__            */
