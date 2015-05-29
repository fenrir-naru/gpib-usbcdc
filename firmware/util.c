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

#include "util.h"
#include <string.h>
#include <limits.h>

#ifdef ENDIAN_SWAP_BY_FUNC
u32 swap_u32(u32 dw){
  DWORD_t dw2;
  u8 tmp;
  dw2.i = dw;
  tmp = dw2.c[0];
  dw2.c[0] = dw2.c[3];
  dw2.c[3] = tmp;
  tmp = dw2.c[1];
  dw2.c[1] = dw2.c[2];
  dw2.c[2] = tmp;
  return dw2.i;
}
u16 swap_u16(u16 w){
  WORD_t w2;
  u8 tmp;
  w2.i = w;
  tmp = w2.c[0];
  w2.c[0] = w2.c[1];
  w2.c[1] = tmp;
  return w2.i;
}
#endif

void wait_8n6clk(unsigned char i){
  while(i--);
/*_asm
  mov r2,dpl            ; 2clk
start_wait_10n4:
  mov ar3,r2            ; 2clk
  dec r2                ; 1clk
  mov a,r3              ; 1clk
  jnz start_wait_10n4   ; 2/4clk
  ret                   ; 6clk
_endasm;*/
}

/**
 * Delay function with declared wait time in microseconds
 * 
 * @param count time in us
 */
void _wait_us(unsigned int count){
  while(count--) wait_8n6clk(5); // 46clk + about 45clk * n
/*_asm
  mov r6,dpl             ; 2clk
  mov r7,dph             ; 2clk
00101$:
  mov ar4,r6             ; 2clk
  mov ar5,r7             ; 2clk
  dec r6                 ; 1clk
  cjne  r6,#0xFF,00113$  ; 3/5clk
  dec r7                 ; 1clk
00113$:
  mov a,r4               ; 1clk
  orl a,r5               ; 1clk
  jz  00104$             ; 2/4clk
  mov dpl,#0x04          ; 3clk
  push  ar7              ; 2clk
  push  ar6              ; 2clk
  lcall _wait_8n6clk     ; 5clk
  pop ar6                ; 2clk
  pop ar7                ; 2clk
  sjmp  00101$           ; 4clk
00104$:
  ret                    ; 6clk
_endasm;
*/
}

/**
 * Delay function with declared wait time in milliseconds
 * 
 * @param count - time in ms
 */
void wait_ms(unsigned int count){
  while(count--) wait_us(1000);
}

