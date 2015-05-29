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

#include "main.h"
#include "usb_descriptor.h"

#include "usb_cdc.h"

#define DESC_DEVICE_bcdUSB        0x0200
#define DESC_DEVICE_bcdDevice     0x0000
#define DESC_DEVICE_iManufacturer 0x01
#define DESC_DEVICE_iProduct      0x02
#define DESC_DEVICE_idVendor      0x10C4 // Silicon Laboratories

/*
 * Descriptor Declarations for CDC-ACM
 */ 
const __code device_descriptor_t DESC_DEVICE = {
  sizeof(device_descriptor_t),   // bLength(0x12)
  DSC_TYPE_DEVICE,                // bDescriptorType
  {DESC_DEVICE_bcdUSB},           // bcdUSB
#ifndef CDC_IS_REPLACED_BY_FTDI
  0x02,                           // bDeviceClass (Communication Class)
  0x00,                           // bDeviceSubClass
  0x00,                           // bDeviceProtocol
#else
  0x00,                           // bDeviceClass (defined at interface level)
  0x00,                           // bDeviceSubClass
  0x00,                           // bDeviceProtocol
#endif
  PACKET_SIZE_EP0,                // bMaxPacketSize0
#ifndef CDC_IS_REPLACED_BY_FTDI
  {DESC_DEVICE_idVendor},         // idVendor
  {0x8A5F},                       // idProduct
  {DESC_DEVICE_bcdDevice},        // bcdDevice
#else
  {0x0403},                       // idVendor  => FTDI
  {0x6001},                       // idProduct => FT232R
  {0x0600},                       // bcdDevice => (FT232R)
#endif
  DESC_DEVICE_iManufacturer,      // iManufacturer
  DESC_DEVICE_iProduct,           // iProduct
  0x04,                           // iSerialNumber
  0x01                            // bNumConfigurations
}; //end of DEVICE

#define DESC_CONFIG_bConfigurationValue 0x01                               
#define DESC_CONFIG_iConfiguration      0x00
#define DESC_CONFIG_bmAttributes        0x80
#define DESC_CONFIG_MaxPower            0xFA // 500mA

const __code configuration_descriptor_t DESC_CONFIG = {
  sizeof(configuration_descriptor_t),// Length(0x09)
  DSC_TYPE_CONFIG,                    // Type
#ifndef CDC_IS_REPLACED_BY_FTDI
  {config_length_total(2, 3)          // + ((interface : 2) + (endpoint : 3))
    + cdc_config_length()},           // + cdc
  0x02,                               // NumInterfaces(C)
#else
  {config_length_total(1, 2)},        // + ((interface : 1) + (endpoint : 2))           
  0x01,                               // NumInterfaces(C(FTDI))
#endif
  DESC_CONFIG_bConfigurationValue,    // bConfigurationValue
  DESC_CONFIG_iConfiguration,         // iConfiguration
  DESC_CONFIG_bmAttributes,           // bmAttributes
  DESC_CONFIG_MaxPower                // MaxPower
}; //end of CONFIG

#ifndef CDC_IS_REPLACED_BY_FTDI
const __code interface_descriptor_t DESC_INTERFACE1 = { // Communication Class
  sizeof(interface_descriptor_t),// bLength
  DSC_TYPE_INTERFACE,             // bDescriptorType
  0x00,                           // bInterfaceNumber
  0x00,                           // bAlternateSetting
  0x01,                           // bNumEndpoints
  0x02,                           // bInterfaceClass (Communication Class)
  0x02,                           // bInterfaceSubClass (Abstract Control Model)
  0x01,                           // bInterfaceProcotol (V.25ter, Common AT commands)
  0x00                            // iInterface
}; // end of INTERFACE1

const __code cdc_descriptor_t DESC_CDC = {
  {   // Header Functional Descriptor
    sizeof(cdc_header_func_descriptor_t),
    DSC_TYPE_CS_INTERFACE,        // bDescriptorType (CS_INTERFACE)
    DSC_SUBTYPE_CS_HEADER_FUNC,   // bDescriptorSubtype (Header Functional)
    {0x0110}                      // bcdCDC (CDC spec release number, 1.1)
  },
  {   // Abstract Control Management Functional Descriptor
    sizeof(cdc_abst_control_mana_descriptor_t),
    DSC_TYPE_CS_INTERFACE,        // bDescriptorType (CS_INTERFACE)
    DSC_SUBTYPE_CS_ABST_CNTRL,    // bDescriptorSubtype (Abstract Control Management)
    0x06                          // bmCapabilities (Supports Send_Break, Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, and the notification Serial_State)
  },
  {   // Union Functional Descriptor
    sizeof(cdc_union_func_descriptor_t),
    DSC_TYPE_CS_INTERFACE,        // bDescriptorType (CS_INTERFACE)
    DSC_SUBTYPE_CS_UNION_FUNC,    // bDescriptorSubtype (Union Functional)
    0x00,                         // bMasterInterface (Interface number master interface in the union)
    0x01                          // bSlaveInterface0 (Interface number slave interface in the union)
  },
  {   // Call Management Functional Descriptor
    sizeof(cdc_call_man_func_descriptor_t),
    DSC_TYPE_CS_INTERFACE,        // bDescriptorType (CS_INTERFACE)
    DSC_SUBTYPE_CS_CALL_MAN,      // bDescriptorSubtype (Call Management)
    0x01,                         // bmCapabilities (only over Communication Class IF / handles itself)
    0x01                          // bDataInterface (Interface number of Data Class interface)
  }
};

const __code endpoint_descriptor_t DESC_ENDPOINT1 = {
  sizeof(endpoint_descriptor_t), // bLength
  DSC_TYPE_ENDPOINT,              // bDescriptorType
  IN_EP1,                         // bEndpointAddress
  DSC_EP_INTERRUPT,               // bmAttributes
  {PACKET_SIZE_EP1},              // MaxPacketSize
  0x10                            // bInterval
}; //end of ENDPOINT1
#endif

const interface_descriptor_t DESC_INTERFACE2 = { // Data Interface Class
  sizeof(interface_descriptor_t),// bLength
  DSC_TYPE_INTERFACE,             // bDescriptorType
#ifndef CDC_IS_REPLACED_BY_FTDI
  0x01,                           // bInterfaceNumber
#else
  0x00,                           // bInterfaceNumber
#endif
  0x00,                           // bAlternateSetting
  0x02,                           // bNumEndpoints
#ifndef CDC_IS_REPLACED_BY_FTDI
  0x0A,                           // bInterfaceClass (Data Interface Class)
  0x00,                           // bInterfaceSubClass
  0x00,                           // bInterfaceProcotol (No class specific protocol required)
#else
  0xFF,                           // bInterfaceClass (Data Interface Class)
  0xFF,                           // bInterfaceSubClass
  0xFF,                           // bInterfaceProcotol (No class specific protocol required)
#endif
  0x00                            // iInterface
}; //end of INTERFACE2

const __code endpoint_descriptor_t DESC_ENDPOINT2 = {
  sizeof(endpoint_descriptor_t), // bLength
  DSC_TYPE_ENDPOINT,              // bDescriptorType
  IN_EP2,                         // bEndpointAddress
  DSC_EP_BULK,                    // bmAttributes
  {PACKET_SIZE_EP2},              // MaxPacketSize
  0                               // bInterval
}; //end of ENDPOINT2

const __code endpoint_descriptor_t DESC_ENDPOINT3 = {
  sizeof(endpoint_descriptor_t), // bLength
  DSC_TYPE_ENDPOINT,              // bDescriptorType
  OUT_EP2,                        // bEndpointAddress
  DSC_EP_BULK,                    // bmAttributes
  {PACKET_SIZE_EP2},              // MaxPacketSize
  0                               // bInterval
}; //end of ENDPOINT3

const __code  BYTE DESC_STRING1[] = {
  sizeof(DESC_STRING1), DSC_TYPE_STRING,
  'n', 0,
  'a', 0,
  'r', 0,
  'u', 0,
  'o', 0,
  'k', 0,
  'a', 0,
  '.', 0,
  'o', 0,
  'r', 0,
  'g', 0,
}; //end of String1_Desc

const __code BYTE DESC_STRING2[] = {
  sizeof(DESC_STRING2), DSC_TYPE_STRING,
  'g', 0,
  'p', 0,
  'i', 0,
  'b', 0,
  '-', 0,
  'u', 0,
  's', 0,
  'b', 0,
}; //end of String2_Desc

const __code BYTE DESC_STRING3[] = {
  sizeof(DESC_STRING3), DSC_TYPE_STRING,
  'C', 0,
  'D', 0,
  'C', 0,
  '.', 0,
  'X', 0,
  'X', 0,
  'X', 0,
  'X', 0,
}; //end of String4_Desc

const __code BYTE * __code DESC_STRINGs[3] = {
  DESC_STRING1,
  DESC_STRING2,
  DESC_STRING3,
};
