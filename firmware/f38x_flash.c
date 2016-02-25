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

#include "f38x_flash.h"
#include "c8051f380.h"
#include "main.h"
#include "util.h"

static __bit ea_orig;
static __xdata u8 FLSCL_orig, VDM0CN_orig, RSTSRC_orig;

/*
 * @see http://community.silabs.com/t5/8-bit-MCU-Knowledge-Base/FLASH-Corruption/ta-p/110449
 * @see http://community.silabs.com/t5/8-bit-MCU-Knowledge-Base/All-about-the-VDD-Monitor/ta-p/110805
 */

static void prologue(){
  ea_orig = EA; // Preserve EA
  EA = 0; // Disable interrupts

  FLSCL_orig = FLSCL & 0x90;
  if(SYSCLK > 25000000UL){ // check clock speed
    FLSCL = (FLSCL_orig | 0x10);
  }

  VDM0CN_orig = VDM0CN;
  if(!(VDM0CN & 0x80)){
    VDM0CN = (VDM0CN_orig | 0x80); // Enable VDD monitor
    wait_us(100);
  }
  RSTSRC_orig = RSTSRC & 0xA6;
  RSTSRC = (RSTSRC_orig | 0x02); // Enable VDD monitor as a reset source; do not use a bit-wise OP
}

static void epilogue(){
  RSTSRC = RSTSRC_orig;
  VDM0CN = VDM0CN_orig;
  FLSCL = FLSCL_orig;
  EA = ea_orig; // Restore interrupts
}

#define set_keys() { \
  FLKEY = 0xA5; \
  FLKEY = 0xF1; \
}

#define write_byte(addr, byte) { \
  *((u8 __xdata *)addr) = byte; \
}

#define read_byte(addr) *((u8 __code *)(addr))

static u8 check_erased(flash_address_t addr, u16 size){
  u8 already_erased = TRUE;
  while(size--){
    if(read_byte(addr) != 0xFF){
      already_erased = FALSE;
      break;
    }
    addr++;
  }
  return already_erased;
}

void flash_erase_page(flash_address_t addr){
#if 1
  addr &= ~((flash_address_t)FLASH_PAGESIZE - 1);
  if(check_erased(addr, FLASH_PAGESIZE)){return;}
#endif

  prologue();

  set_keys();
  PSCTL |= 0x03; // PSWE = 1, PSEE = 1
  write_byte(addr, 0);
  PSCTL &= ~0x03; // PSWE = 0, PSEE = 0

  epilogue();
}

u16 flash_write(flash_address_t dst, u8 *src, u16 size){
  u16 size_orig = size;
  u8 b[2];
  do{
    if(size == 0){break;}
    prologue();
#if 0
    do{
      b[0] = *(src++);
      set_keys();
      PSCTL |= 0x01; // PSWE = 1
      write_byte(dst, b[0]);
      PSCTL &= ~0x01; // PSWE = 0
      dst++;
    }while(--size);
#elif 1
    if(dst & 0x1){ // odd
      b[0] = *(src++);
      PSCTL |= 0x01; // PSWE = 1
      set_keys();
      write_byte(dst, b[0]);
      PSCTL &= ~0x01; // PSWE = 0
      dst++;
      size--;
    }
    PFE0CN |= 0x01;
    while(size >= 2){
      u16 dst_odd = dst + 1;
      b[0] = *(src++);
      b[1] = *(src++);
      PSCTL |= 0x01; // PSWE = 1
      set_keys();
      write_byte(dst, b[0]);
      set_keys();
      write_byte(dst_odd, b[1]);
      PSCTL &= ~0x01; // PSWE = 0
      dst = dst_odd + 1;
      size -= 2;
    }
    PFE0CN &= ~0x01;
    if(size > 0){
      b[0] = *src;
      PSCTL |= 0x01; // PSWE = 1
      set_keys();
      write_byte(dst, b[0]);
      PSCTL &= ~0x01; // PSWE = 0
    }
#endif
    epilogue();
  }while(0);
  return size_orig;
}

static void page_copy(flash_address_t dst, flash_address_t src){
  flash_erase_page(dst);
  flash_write(dst, (u8 __code *)src, FLASH_PAGESIZE);
}

u16 flash_update(flash_address_t dst, u8 *src, u16 size){
  flash_address_t dst_align = dst & ~((flash_address_t)FLASH_PAGESIZE - 1);
  u16 size_orig = size;

  if((size > 0) && (dst_align != dst)){ // first page
    u16 copy1_size = dst - dst_align;
    u16 copy2_size = FLASH_PAGESIZE - copy1_size;
    u16 copy3_size;
    if(copy2_size > size){
      copy3_size = copy2_size - size;
      copy2_size = size;
    }else{
      copy3_size = 0;
    }

    // check requirement for copy
    if(check_erased(dst_align, copy1_size)
        && check_erased(dst_align + FLASH_PAGESIZE - copy3_size, copy3_size)){
      copy1_size = 0;
      copy3_size = 0;
    }else{
      page_copy(FLASH_SCRATCH_PAGE, dst_align);
    }

    flash_erase_page(dst_align);
    flash_write(dst_align, (u8 __code *)FLASH_SCRATCH_PAGE, copy1_size);
    flash_write(dst, src, copy2_size);
    dst += copy2_size;
    src += copy2_size;
    size -= copy2_size;
    flash_write(dst, (u8 __code *)(FLASH_SCRATCH_PAGE + FLASH_PAGESIZE - copy3_size), copy3_size);
  }

  while(size >= FLASH_PAGESIZE){ // updating intermediate pages every page
    flash_erase_page(dst);
    flash_write(dst, src, FLASH_PAGESIZE);
    dst += FLASH_PAGESIZE;
    src += FLASH_PAGESIZE;
    size -= FLASH_PAGESIZE;
  }

  if(size > 0){ // last page
    u16 copy_size = FLASH_PAGESIZE - size;
    if(check_erased(dst + size, copy_size)){
      copy_size = 0;
    }else{
      page_copy(FLASH_SCRATCH_PAGE, dst);
    }

    flash_erase_page(dst);
    flash_write(dst, src, size);
    dst += size;
    flash_write(dst, (u8 __code *)(FLASH_SCRATCH_PAGE + size), copy_size);
  }

  return size_orig;
}
