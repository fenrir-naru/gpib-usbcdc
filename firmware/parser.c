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

#include "parser.h"

#include <string.h>

#if !defined(LOCAL_TEST)
#define LOCAL_TEST 0
#endif

#if LOCAL_TEST
#include <stdio.h>
#endif

const __code char * __code command_str[] = {
  "addr",
  "auto",
  "clr",
  "eoi",
  "eos",
  "eot_enable",
  "eot_char",
  "ifc",
  "llo",
  "loc",
  "lon",
  "mode",
  "read",
  "read_tmo_ms",
  "rst",
  "savecfg",
  "spoll",
  "srq",
  "status",
  "trg",
  "ver",
  "help",
  "debug",
};

static enum command_t check_cmd(__xdata char *str, u8 len){
#if LOCAL_TEST
  {
    int i;
    printf("check_cmd: ");
    for(i = 0; i < len; ++i){
      putchar(str[i]);
    }
    printf("\n");
  }
#endif
#define check_str(cmd) { \
  if(memcmp(command_str[cmd], str, len) == 0){ \
    return cmd; \
  } \
}
  switch(len){
    case 3:
      check_str(CMD_CLR);
      check_str(CMD_EOI);
      check_str(CMD_EOS);
      check_str(CMD_IFC);
      check_str(CMD_LLO);
      check_str(CMD_LOC);
      check_str(CMD_LON);
      check_str(CMD_RST);
      check_str(CMD_SRQ);
      check_str(CMD_TRG);
      check_str(CMD_VER);
      break;
    case 4:
      check_str(CMD_ADDR);
      check_str(CMD_AUTO);
      check_str(CMD_MODE);
      check_str(CMD_READ);
      check_str(CMD_HELP);
      break;
    case 5:
      check_str(CMD_SPOLL);
      check_str(CMD_DEBUG);
      break;
    case 6:
      check_str(CMD_STATUS);
      break;
    case 7:
      check_str(CMD_SAVECFG);
      break;
    case 8:
      check_str(CMD_EOT_CHAR);
      break;
    case 10:
      check_str(CMD_EOT_ENABLE);
      break;
    case 11:
      check_str(CMD_READ_TMO_MS);
      break;
  }
#undef check_str
  return CMD_ERROR;
}

static int check_arg(__xdata char *str, u8 len){
  int res;
#if defined(__SDCC) || defined(SDCC) || LOCAL_TEST
  int i;
  for(res = 0, i = 0; i < len; ++i){
    int v = str[i] - '0';
    if((v < 0) || (v > 9)){return ARG_ERR;}
    res *= 10;
    res += v;
  }
#else
  char *end;
  str[len] = '\0';
  res = strtol(str, &end, 0);
  if((((__xdata char *)end) - str) != len){
    return ARG_ERR;
  }
#endif
  return res;
}

static __xdata enum {
  THROUGH,
  ON_FIRST_CHAR_PLUS,
  ON_COMMAND_OR_ARG,
} state = THROUGH;
static __bit on_escape = FALSE;
static __bit last_char_is_cr = FALSE;

void parser_reset(){
  state = THROUGH;
  on_escape = FALSE;
  last_char_is_cr = FALSE;
}

#define PARSED_INFO_BUFFERD_ARGS \
  (sizeof(((parsed_info_t *)(0))->arg) / sizeof(((parsed_info_t *)(0))->arg[0]))

/* Parse Prologix protocol and run command */
void parse(char c){
  static __xdata char buf[16];
  static __xdata u8 buf_index;
  static parsed_info_t parsed_info;
  enum {CHAR_NORMAL, CHAR_PLUS, CHAR_TERMINATOR, CHAR_SEPARATOR} c_attr = CHAR_NORMAL;
  __bit current_char_is_cr = FALSE;

  if(on_escape){
    on_escape = FALSE;
  }else{
    switch(c){
      case 0x1B: //'\e': // sdcc do not support '\e'
        on_escape = TRUE;
        return;
      case '+':
        c_attr = CHAR_PLUS;
        break;
      case '\r':
        current_char_is_cr = TRUE;
        c_attr = CHAR_TERMINATOR;
        break;
      case '\n':
        if(last_char_is_cr){
          last_char_is_cr = FALSE;
          return;
        } // reduce crlf.
        c_attr = CHAR_TERMINATOR;
        break;
      case ' ':
        c_attr = CHAR_SEPARATOR;
        break;
    }
  }

  last_char_is_cr = current_char_is_cr;

  switch(state){
    case ON_FIRST_CHAR_PLUS:
      if(c_attr == CHAR_PLUS){
        state = ON_COMMAND_OR_ARG;
        buf_index = 0;
        parsed_info.args = -1;
        break;
      }
      state = THROUGH;
    case THROUGH:
      parsed_info.cmd = CMD_TALK;
      if(c_attr == CHAR_PLUS){
        state = ON_FIRST_CHAR_PLUS;
      }else{
        if(c_attr == CHAR_TERMINATOR){
          parsed_info.args = 0;
        }else{
          parsed_info.args = 1;
          parsed_info.arg[0] = c;
        }
        run_command(&parsed_info);
      }
      break;
    case ON_COMMAND_OR_ARG:
      if(c_attr == CHAR_PLUS){break;}
      if(c_attr == CHAR_NORMAL){
        if(buf_index < (sizeof(buf) - 1)){
          buf[buf_index++] = c;
        }
        break;
      }

      // TERMINATOR or SEPARATOR
      if(buf_index > 0){
        if(parsed_info.args < 0){
          // check command in buf.
          parsed_info.cmd = check_cmd(buf, buf_index);
        }else{
          // check arg in buf.
          while(parsed_info.args <= PARSED_INFO_BUFFERD_ARGS){
            if((parsed_info.cmd == CMD_READ) && (parsed_info.args == 0)){
              if((buf_index == 3) && (memcmp(buf, "eoi", buf_index) == 0)){
                parsed_info.arg[parsed_info.args] = ARG_EOI;
                break;
              }
            }
            parsed_info.arg[parsed_info.args] = check_arg(buf, buf_index);
            break;
          }
        }
        buf_index = 0;
        parsed_info.args++;
      }

      if(c_attr == CHAR_TERMINATOR){
        state = THROUGH;
        //if(parsed_info.cmd == CMD_ERROR){break;}
        run_command(&parsed_info);
      }
      break;
  }
}

#if LOCAL_TEST /* for local test */

#include <ctype.h>

void run_command(parsed_info_t *info){
  printf("exec: %d, %d\n", info->cmd, info->args);
  if(info->cmd == CMD_TALK){
    printf("exec talk");
    if(info->args > 0){
      char c = (char)(info->arg[0]);
      printf(" %c(%d)", isprint(c) ? c : ' ', c);
    }
    printf("\n");
  }else{
    int i;
    int args = sizeof(info->arg) / sizeof(info->arg[0]);

    if(args > info->args){args = info->args;}
    if(args == 0){return;}

    printf("exec with arg:");
    for(i = 0; i < args; ++i){
      printf( " %d", info->arg[i]);
    }
    printf("\n");
  }
}

int main(int argc, char *argv[]){

  if(argc == 1){
    char c;
    while((c = getchar()) != EOF){
      printf("\nin: %c(%d)\n", isprint(c) ? c : ' ', c);
      parse(c);
    }
    printf("\nin: %c(%d)\n", ' ', '\n');
    parse('\n');
    return;
  }

  int i, j;
  for(j = 1; j < argc; j++){
    if(j > 1){
      printf("\nin: %c(%d)\n", ' ', ' ');
      parse(' ');
    }
    for(i = 0; i < strlen(argv[j]); ++i){
      printf("\nin: %c(%d)\n", isprint(argv[j][i]) ? argv[j][i] : ' ', argv[j][i]);
      parse(argv[j][i]);
    }
  }
  if(argc > 1){
    printf("\nin: %c(%d)\n", ' ', '\n');
    parse('\n');
  }

  return 0;
}

#endif
