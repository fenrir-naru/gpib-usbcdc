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

#ifndef __GPIB_IO_H__
#define __GPIB_IO_H__

#include "type.h"

void gpib_io_init();
void gpib_io_set_timeout();

#define GPIB_WRITE_USE_EOI 0x01

u8 gpib_putchar(char c, u8 flags);
u16 gpib_write(char *buf, u16 length, u8 flags);

#define GPIB_GETCHAR_IS_ERROR(x) (x < 0)
#define GPIB_GETCHAR_IS_EOI(x) (x >= 0x100)
#define GPIB_GETCHAR_IS_CMD(x) (x >= 0x200)
#define GPIB_GETCHAR_TO_DATA(x) ((u8)(x & 0xFF))

int gpib_getchar();

#define GPIB_READ_UNTIL_EOI 0x01

u16 gpib_read(void (*push)(char), u8 flags);

enum uniline_message_t {
  GPIB_UNI_CMD_START,
  GPIB_UNI_CMD_END,
  GPIB_UNI_BUS_CLEAR_START,
  GPIB_UNI_BUS_CLEAR_END,
  GPIB_UNI_CHECK_SRQ_ASSERT,
  GPIB_UNI_SRQ_ASSERT,
  GPIB_UNI_SRQ_DEASSERT,
};

u8 gpib_uniline(enum uniline_message_t msg);

#endif /* __GPIB_IO_H__ */
