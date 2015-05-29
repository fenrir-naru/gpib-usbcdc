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

#ifndef __PARSER_H__
#define __PARSER_H__

#include "type.h"

enum command_t {
  CMD_ADDR,
  CMD_AUTO,
  CMD_CLR,
  CMD_EOI,
  CMD_EOS,
  CMD_EOT_ENABLE,
  CMD_EOT_CHAR,
  CMD_IFC,
  CMD_LLO,
  CMD_LOC,
  CMD_LON,
  CMD_MODE,
  CMD_READ,
  CMD_READ_TMO_MS,
  CMD_RST,
  CMD_SAVECFG,
  CMD_SPOLL,
  CMD_SRQ,
  CMD_STATUS,
  CMD_TRG,
  CMD_VER,
  CMD_HELP,
  CMD_DEBUG,
  CMD_INPUTABLE,
  CMD_ERROR = CMD_INPUTABLE,
  CMD_TALK,
};

extern const __code char * __code command_str[CMD_INPUTABLE];

void parser_reset();

typedef __xdata struct {
  enum command_t cmd;
  int args;
  int arg[30];
} parsed_info_t;

extern void run_command(parsed_info_t *info);
void parse(char c);

#define ARG_ERR -1
#define ARG_EOI 256

#endif /* __PARSER_H__ */
