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

#include "gpib.h"
#include "parser.h"
#include "gpib_io.h"

#include "c8051f380.h"
#include "usb_cdc.h"
#include "util.h"
#include "f38x_flash.h"

#include <string.h>

static __code __at (0x7a00) gpib_config_struct gpib_config_saved = {
  {{{0},}, 0,}, // address
  0, // read_after_write
  0, // eoi
  0, // eos
  0, // eot
  (char)0, // eot_char
  0, // listen_only
  1, // is_controller
  (u16)10, // timeout_ms
  0, // status
  0, // debug
};

gpib_config_t gpib_config;

#define DEBUG_ECHO 0x01
#define DEBUG_GPIB_ECHO 0x02
#define DEBUG_VERBOSE 0x04

#define print_str(str) write(str, sizeof(str) - 1)
static void print_terminator(u16 (*write)(char *buf, u16 len)){
  switch(gpib_config.eos){
    case 0: print_str("\r\n"); break;
    case 1: print_str("\r"); break;
    default: print_str("\n"); break;
  }
}
#undef print_str

static u16 (*write_func)(char *, u16) = (u16 (*)(char *, u16))cdc_tx;
static void push_func(char c){write_func(&c, 1);}

#define print_str(str) write_func(str, sizeof(str) - 1)
static void print_header(){
  print_str("++");
}

static void print_space(){
  print_str(" ");
}
#undef print_str

static void print_u16(u16 v){
  char buf[5];
  u8 i = sizeof(buf);
  while(1){
    buf[--i] = '0' + (v % 10);
    if(v < 10){break;}
    v /= 10;
  }
  write_func(&buf[i], sizeof(buf) - i);
}

static void print_address(enum commant_t cmd, __xdata address_t *address){
  u8 i;
  if(gpib_config.debug & DEBUG_VERBOSE){
    print_header();
    write_func(command_str[cmd], strlen(command_str[cmd]));
    print_space();
  }
  for(i = 0; i < address->valid_items; ++i){
    if(i > 0){print_space();}
    print_u16(address->item[i][0]);
    if(gpib_config.address.item[i][1] > 0){
      print_space();
      print_u16(address->item[i][1]);
    }
  }
  print_terminator(write_func);
}

static void print_1arg(enum commant_t cmd, u16 arg1){
  if(gpib_config.debug & DEBUG_VERBOSE){
    print_header();
    write_func(command_str[cmd], strlen(command_str[cmd]));
    print_space();
  }
  print_u16(arg1);
  print_terminator(write_func);
}

static void dump_config(){
  print_address(CMD_ADDR, &gpib_config.address);
  print_1arg(CMD_AUTO, gpib_config.read_after_write);
  print_1arg(CMD_EOI, gpib_config.eoi);
  print_1arg(CMD_EOS, gpib_config.eos);
  print_1arg(CMD_EOT_ENABLE, gpib_config.eot);
  print_1arg(CMD_EOT_CHAR, (u8)(gpib_config.eot_char));
  print_1arg(CMD_LON, gpib_config.listen_only);
  print_1arg(CMD_MODE, gpib_config.is_controller);
  print_1arg(CMD_READ_TMO_MS, gpib_config.timeout_ms);
  print_1arg(CMD_STATUS, gpib_config.status);
}

#define renew_arg0(type) \
static u8 renew_arg0_ ## type(parsed_info_t *info, __xdata type *res, type max){ \
  if((info->args > 0) && (info->arg[0] >= 0) && (info->arg[0] <= max)){ \
    type tmp = (type)info->arg[0]; \
    if(*res != tmp){ \
      *res = tmp; \
      return 1; \
    } \
  } \
  return 0; \
}
renew_arg0(u8)
renew_arg0(u16)

static u8 check_address(__xdata int *buf, u8 size, u8 *res){
  u8 changes = 0;
  if((size > 0) && (buf[0] >= 0) && (buf[0] <= 30)){
    res[0] = buf[0];
    changes++;
    if((size > 1) && (buf[1] >= 96) && (buf[1] <= 126)){
      res[1] = buf[1];
      changes++;
    }else{
      res[1] = 0;
    }
  }
  return changes;
}

static __xdata address_t *get_address(parsed_info_t *info){
  static __xdata address_t address;
  u8 i = 0;
  address.valid_items = 0;
  while(info->args > 0){
    u8 captured = check_address(&(info->arg[i]), (u8)info->args, address.item[address.valid_items]);
    if(captured == 0){
      captured = 1; // skip invalid argument.
    }else{
      address.valid_items++;
    }
    i += captured;
    info->args -= captured;
  }
  return (address.valid_items > 0) ? &address : NULL;
}

enum gpib_command_t {
  GPIB_CMD_DCL = 0x14,
  GPIB_CMD_UNL = 0x3F,
  GPIB_CMD_UNT = 0x5F,
  GPIB_CMD_GET = 0x08,
  GPIB_CMD_SDC = 0x04,
  GPIB_CMD_LLO = 0x11,
  GPIB_CMD_GTL = 0x01,
  GPIB_CMD_SPE = 0x18,
  GPIB_CMD_SPD = 0x19,
};
#define GPIB_CMD_LAD(x) (x | 0x20)
#define GPIB_CMD_IS_LAD_OR_UNL(x) ((x & 0x60) == 0x20)
#define GPIB_CMD_TAD(x) (x | 0x40)
#define GPIB_CMD_IS_TAD_OR_UNT(x) ((x & 0x60) == 0x40)

static void gpib_cmd(u8 cmd, __xdata address_t *new_listener){
  gpib_uniline(GPIB_UNI_CMD_START);
  if(new_listener){
    u8 i;
    gpib_putchar(GPIB_CMD_UNL, 0);
    for(i = 0; i < new_listener->valid_items; i++){
      gpib_putchar(GPIB_CMD_LAD(new_listener->item[i][0]), 0);
      if(new_listener->item[i][1] != 0){
        gpib_putchar(new_listener->item[i][1], 0);
      }
    }
  }
  gpib_putchar(cmd, 0);
  gpib_uniline(GPIB_UNI_CMD_END);
}

static u16 gpib_write_auto_eoi(char *buf, u16 length){
  return gpib_write(buf, length,
      gpib_config.eoi ? GPIB_WRITE_USE_EOI : 0);
}

static __bit talking;
static void force_end_talking(){
  if(talking){
    talking = FALSE;
    print_terminator(gpib_write_auto_eoi);
  }
}

static __bit talkable_as_device;
static __bit listening_as_device;
static __bit serial_polling_as_device;

static void device_init(){
  talkable_as_device = FALSE;
  listening_as_device = gpib_config.listen_only ? TRUE : FALSE;
  serial_polling_as_device = FALSE;
  if(gpib_config.status & 0x40){
    gpib_uniline(GPIB_UNI_SRQ_ASSERT); // SRQ
  }
}
static __bit listening;

void run_command(parsed_info_t *info){
  u8 not_query = (!(gpib_config.debug & DEBUG_VERBOSE)) && (info->args > 0);
  switch(info->cmd){
    case CMD_ADDR: {
      __xdata address_t *new_address = get_address(info);
      if(new_address){
        memcpy(&(gpib_config.address), new_address, sizeof(address_t));
      }
      if(not_query){break;}
      print_address(CMD_ADDR, &gpib_config.address); // return current address
      break;
    }
    case CMD_AUTO:
      renew_arg0_u8(info, &gpib_config.read_after_write, 1);
      if(not_query){break;}
      print_1arg(CMD_AUTO, gpib_config.read_after_write); // return current auto
      break;
    case CMD_CLR:
      if(gpib_config.is_controller){
        force_end_talking();
        gpib_cmd(GPIB_CMD_SDC, &gpib_config.address);
      }
      break;
    case CMD_EOI:
      renew_arg0_u8(info, &gpib_config.eoi, 1);
      if(not_query){break;}
      print_1arg(CMD_EOI, gpib_config.eoi); // return current eoi
      break;
    case CMD_EOS:
      renew_arg0_u8(info, &gpib_config.eos, 3);
      if(not_query){break;}
      print_1arg(CMD_EOS, gpib_config.eos); // return current eos
      break;
    case CMD_EOT_ENABLE:
      renew_arg0_u8(info, &gpib_config.eot, 1);
      if(not_query){break;}
      print_1arg(CMD_EOT_ENABLE, gpib_config.eot); // return current eot
      break;
    case CMD_EOT_CHAR:
      renew_arg0_u8(info, (__xdata char *)&gpib_config.eot_char, 255);
      if(not_query){break;}
      print_1arg(CMD_EOT_CHAR, (u8)gpib_config.eot_char); // return current eos_char
      break;
    case CMD_IFC:
      if(gpib_config.is_controller){
        force_end_talking();
        gpib_uniline(GPIB_UNI_BUS_CLEAR_START);
        wait_ms(1);
        gpib_uniline(GPIB_UNI_BUS_CLEAR_END);
      }
      break;
    case CMD_LLO:
      if(gpib_config.is_controller){
        force_end_talking();
        gpib_cmd(GPIB_CMD_LLO, NULL);
      }
      break;
    case CMD_LOC:
      if(gpib_config.is_controller){
        force_end_talking();
        gpib_cmd(GPIB_CMD_GTL, &gpib_config.address);
      }
      break;
    case CMD_LON:
      renew_arg0_u8(info, &gpib_config.listen_only, 1);
      if(gpib_config.listen_only){listening_as_device = TRUE;}
      if(not_query){break;}
      print_1arg(CMD_LON, gpib_config.listen_only); // return current los
      break;
    case CMD_MODE:
      if(renew_arg0_u8(info, &gpib_config.is_controller, 1)){
        force_end_talking();
        gpib_io_init();
        if(!gpib_config.is_controller){device_init();}
      }
      if(not_query){break;}
      print_1arg(CMD_MODE, gpib_config.is_controller); // return current mode
      break;
    case CMD_READ: // Different from Prologix impl.
      if(!gpib_config.is_controller){break;}
      force_end_talking();
      gpib_uniline(GPIB_UNI_CMD_START);
      gpib_putchar(GPIB_CMD_UNL, 0);
      gpib_putchar(GPIB_CMD_LAD(0), 0); // listener, it's me.
      gpib_putchar(GPIB_CMD_TAD(gpib_config.address.item[0][0]), 0); // talker
      gpib_uniline(GPIB_UNI_CMD_END);
      if((info->args > 0) && (info->arg[0] == ARG_EOI)){
        gpib_read(push_func, GPIB_READ_UNTIL_EOI);
      }else{
        gpib_read(push_func, 0);
      }
      listening = TRUE;
      break;
    case CMD_READ_TMO_MS:
      if(renew_arg0_u16(info, &gpib_config.timeout_ms, 3000)){
        gpib_io_set_timeout();
      }
      if(not_query){break;}
      print_1arg(CMD_READ_TMO_MS, gpib_config.timeout_ms);
      break;
    case CMD_RST:
      RSTSRC = 0x10; // RSTSRC.4(SWRSF) = 1 causes software reset
      break;
    case CMD_SAVECFG:
      // Different from Prologic command, savecfg is performing one-shot save.
      flash_update((flash_address_t)(void __code *)&gpib_config_saved,
          &gpib_config, sizeof(gpib_config));
      if(gpib_config.debug & DEBUG_VERBOSE){dump_config();}
      break;
    case CMD_SPOLL: {
      if(gpib_config.is_controller){
        u8 buf[2];
        int stb;

        force_end_talking();
        gpib_uniline(GPIB_UNI_CMD_START);
        gpib_putchar(GPIB_CMD_SPE, 0);
        memcpy(buf, gpib_config.address.item[0], sizeof(gpib_config.address.item[0]));
        check_address(&(info->arg[0]), (u8)info->args, buf);
        buf[0] = GPIB_CMD_TAD(buf[0]);
        gpib_write(buf, (buf[1] == 0 ? 1 : 2), 0);
        gpib_uniline(GPIB_UNI_CMD_END);

        stb = gpib_getchar();
        if(!GPIB_GETCHAR_IS_ERROR(stb)){
          print_1arg(CMD_SPOLL, GPIB_GETCHAR_TO_DATA(stb));
        }

        buf[0] = GPIB_CMD_UNT;
        buf[1] = GPIB_CMD_SPD;
        gpib_uniline(GPIB_UNI_CMD_START);
        gpib_write(buf, 2, 0);
        gpib_uniline(GPIB_UNI_CMD_END);
      }
      break;
    }
    case CMD_SRQ: {
      if(gpib_config.is_controller){
        print_1arg(CMD_SRQ, gpib_uniline(GPIB_UNI_CHECK_SRQ_ASSERT));
      }
      break;
    }
    case CMD_STATUS:
      if(renew_arg0_u8(info, &gpib_config.status, 0xFF)){
        if(!gpib_config.is_controller){
          gpib_uniline((gpib_config.status & 0x40)
              ? GPIB_UNI_SRQ_ASSERT : GPIB_UNI_SRQ_DEASSERT); // SRQ
        }
      }
      if(not_query){break;}
      print_1arg(CMD_STATUS, gpib_config.status);
      break;
    case CMD_TRG:
      if(!gpib_config.is_controller){break;}
      force_end_talking();
      if(info->args == 0){
        gpib_cmd(GPIB_CMD_GET, &gpib_config.address);
      }else{
        __xdata address_t *new_address = get_address(info);
        if(new_address){
          gpib_cmd(GPIB_CMD_GET, new_address);
        }
      }
      break;
#define print_str(str) write_func(str, sizeof(str) - 1)
    case CMD_VER:
      if(gpib_config.debug & DEBUG_VERBOSE){
        print_header();
        write_func(command_str[CMD_VER], strlen(command_str[CMD_VER]));
        print_space();
      }
      print_str("Fenrir GPIB-USB 1.0 (Prologix version 6.0 compatible)");
      print_terminator(write_func);
      break;
#undef print_str
    case CMD_HELP: {
      u8 i;
      if(gpib_config.debug & DEBUG_VERBOSE){
        print_header();
        write_func(command_str[CMD_HELP], strlen(command_str[CMD_HELP]));
        print_space();
      }
      for(i = 0; i < sizeof(command_str) / sizeof(command_str[0]); ++i){
        if(i > 0){print_space();}
        write_func(command_str[i], strlen(command_str[i]));
      }
      print_terminator(write_func);
      break;
    }
    case CMD_DEBUG:
      renew_arg0_u8(info, &gpib_config.debug, 0xFF);
      if(not_query){break;}
      print_1arg(CMD_DEBUG, gpib_config.debug); // return current debug
      break;
    case CMD_TALK: {
      static __xdata char buf;
      if(talking){
        if(gpib_config.debug & DEBUG_GPIB_ECHO){
          push_func(buf); // print character to be tried to write
        }
        if(info->args == 0){ // terminator
          if(gpib_config.eos == 3){
            gpib_putchar(buf, gpib_config.eoi ? GPIB_WRITE_USE_EOI : 0);
          }else{
            gpib_putchar(buf, 0);
            print_terminator(gpib_write_auto_eoi);
          }
          talking = FALSE;
          if(gpib_config.read_after_write){
            info->cmd = CMD_READ;
            // info->args = 0;
            run_command(info); // valid only for controller.
          }
        }else{
          talking = (gpib_putchar(buf, 0) > 0);
          buf = (u8)(info->arg[0]);
        }
      }else if(info->args > 0){ // ignore terminator when not talking.
        if(gpib_config.is_controller){ // controller
          gpib_cmd(GPIB_CMD_TAD(0), &gpib_config.address); // talker, it's me.
          talking = TRUE;
        }else if(talkable_as_device){ // device
          talking = TRUE;
        }
        buf = (u8)(info->arg[0]);
      }
      break;
    }
  }
}

void gpib_init(){
  memcpy(&gpib_config, &gpib_config_saved, sizeof(gpib_config));
  gpib_io_init();
  gpib_io_set_timeout();
  parser_reset();

  talking = FALSE;
  device_init();
}

void gpib_polling(){
  static __xdata char buf[16];
  static __xdata u8 remain = 0;
  static __xdata char * __xdata c;

  listening = FALSE;

  // parse cdc_rx stream
  if(remain == 0){
    remain = (u8)cdc_rx(buf, sizeof(buf));
    c = buf;
  }
  for(; remain > 0; remain--, c++){
    if(gpib_config.debug & DEBUG_ECHO){push_func(*c);}
    parse(*c);
  }

  if((!gpib_config.is_controller) && (!talking)){ // device mode
    do{
      static __bit last_char_is_cr = FALSE;
      int res = gpib_getchar();

      // transfer to routine to parse cdc_rx stream due to error(timeout etc.)
      if(GPIB_GETCHAR_IS_ERROR(res)){break;}

      if(GPIB_GETCHAR_IS_CMD(res)){
        u8 cmd = GPIB_GETCHAR_TO_DATA(res);
        last_char_is_cr = FALSE;
        if(GPIB_CMD_IS_TAD_OR_UNT(cmd)){
          cmd &= 0x1F;
          if(cmd == (GPIB_CMD_UNT & 0x1F)){ // UNT
            talkable_as_device = FALSE;
          }else if((!gpib_config.listen_only)
              && (cmd == gpib_config.address.item[0][0])){
            talkable_as_device = TRUE;

            // transfer to routine to parse cdc_rx stream because controller request sending data.
            if(!serial_polling_as_device){break;}

            if(gpib_putchar(gpib_config.status, 0)){
              gpib_config.status &= ~(0x40);
              gpib_uniline(GPIB_UNI_SRQ_DEASSERT);
            }
          }
        }else if(GPIB_CMD_IS_LAD_OR_UNL(cmd)){
          cmd &= 0x1F;
          if(cmd == (GPIB_CMD_UNL & 0x1F)){ // UNL
            if(!gpib_config.listen_only){listening_as_device = FALSE;}
          }else if(cmd == gpib_config.address.item[0][0]){
            listening_as_device = TRUE;
          }
        }else{
          switch(cmd){
            //case GPIB_CMD_GTL: break;
            //case GPIB_CMD_SDC: break;
            //case GPIB_CMD_GET: break;
            //case GPIB_CMD_LLO: break;
            //case GPIB_CMD_DCL: break;
            case GPIB_CMD_SPE:
              serial_polling_as_device = TRUE;
              break;
            case GPIB_CMD_SPD:
              serial_polling_as_device = FALSE;
              break;
          }
        }
      }else{
        // @see much similarity to gpib_read()
        char c = GPIB_GETCHAR_TO_DATA(res);
        if(listening_as_device){
          listening = TRUE;
          push_func(c);
        }

        if(GPIB_GETCHAR_IS_EOI(res)){
          if(listening_as_device && gpib_config.eot){push_func(gpib_config.eot_char);}
          break;
        }else if(c == '\r'){
          if(gpib_config.eos == 1){break;}
          last_char_is_cr = TRUE;
        }else{
          if(c == '\n'){
            if((gpib_config.eos == 2)
                || ((last_char_is_cr) && (gpib_config.eos == 0))){break;}
          }
          last_char_is_cr = FALSE;
        }
      }
    }while(1);
  }

  write_func(NULL, 0); // flush buffer

  if(gpib_config.is_controller){
    sys_state |= SYS_GPIB_CONTROLLER;
  }else{
    sys_state &= ~SYS_GPIB_CONTROLLER;
  }
  if(talking){
    sys_state |= SYS_GPIB_TALKING;
  }else{
    sys_state &= ~SYS_GPIB_TALKING;
  }
  if(listening){
    sys_state |= SYS_GPIB_LISTENING;
  }else{
    sys_state &= ~SYS_GPIB_LISTENING;
  }
}
