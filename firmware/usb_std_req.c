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
#include "f38x_usb.h"
#include "f38x_usb_register.h"

#include "usb_descriptor.h"
#include "usb_std_req.h"

#include "usb_cdc.h"

// Standard Request Codes
enum {
  GET_STATUS = 0x00,        // Get Status
  CLEAR_FEATURE = 0x01,     // Clear Feature
  SET_FEATURE = 0x03,       // Set Feature
  SET_ADDRESS = 0x05,       // Set Address
  GET_DESCRIPTOR = 0x06,    // Get Descriptor
  SET_DESCRIPTOR = 0x07,    // Set Descriptor (not used)
  GET_CONFIGURATION = 0x08, // Get Configuration
  SET_CONFIGURATION = 0x09, // Set Configuration
  GET_INTERFACE = 0x0A,     // Get Interface
  SET_INTERFACE = 0x0B,     // Set Interface
  SYNCH_FRAME = 0x0C,       // Synch Frame (not used)
};

// These are response packets used for
static const __code BYTE ONES_PACKET[2] = {0x01, 0x00};
// communication with host
static const __code BYTE ZERO_PACKET[2] = {0x00, 0x00};

static __code device_descriptor_t * __xdata desc_device;
static __code configuration_descriptor_t * __xdata desc_config;


/**
 * This routine returns a two byte status packet to the host
 */
static void get_status(){
  
  // If non-zero return length or data length not
  // equal to 2 then send a stall indicating invalid request
  if(ep0_setup.wValue.c[MSB] 
        || ep0_setup.wValue.c[LSB] 
        || ep0_setup.wLength.c[MSB]
        || (ep0_setup.wLength.c[LSB] != 2)){
    return;
  }

  // Determine if recipient was device, interface, or EP
  switch(ep0_setup.bmRequestType){
    case IN_DEVICE:
      if(ep0_setup.wIndex.i == 0){
        // send 0x00, indicating bus power and no remote wake-up supported
        ep0_register_data((BYTE*)&ZERO_PACKET, 2);
        break;
      }
      // else Send stall if request is invalid
      return;
    
    case IN_INTERFACE:
      if((usb_state == DEV_CONFIGURED)
          && (ep0_setup.wIndex.i < desc_config->bNumInterfaces)){
        // Only valid if device is configured and non-zero index
        // Status packet always returns 0x00
        ep0_register_data((BYTE*)&ZERO_PACKET, 2);
        break;
      }
      // Otherwise send stall to host
      return;
      
    case IN_ENDPOINT:
      if((usb_state == DEV_CONFIGURED) 
          && (ep0_setup.wIndex.c[MSB] == 0)){
        // Make sure device is configured and index msb = 0x00
        
        // Handle case if request is directed to some EPs
        __bit is_valid_req = FALSE;
        switch(ep0_setup.wIndex.c[LSB]){
          case IN_EP1:
          case IN_EP2:
          /*case IN_EP3:*/
          /*case OUT_EP1:*/
          case OUT_EP2:
          /*case OUT_EP3:*/ {
            
            BYTE ep_dir = (is_EP_IN(ep0_setup.wIndex.c[LSB]) ? DIR_IN : DIR_OUT);
            BYTE ep_number = EP_NUMBER(ep0_setup.wIndex.c[LSB]);
            
            if(ep_strip_owner(usb_ep_status(ep_dir, ep_number)) == EP_HALT){
              // If endpoint is halted, return 0x01,0x00
              ep0_register_data((BYTE*)&(ONES_PACKET), 2);
            }else{
              // Otherwise return 0x00,0x00 to indicate endpoint active
              ep0_register_data((BYTE*)&(ZERO_PACKET), 2);
            }
            
            is_valid_req = TRUE;
            break;
          }
        }
        if(is_valid_req){break;} 
      }
      // else Send stall if unexpected data encountered  
      return;
      
    default:
      return;
  }
  
  usb_ep0_status = EP_TX;
  ep0_request_completed = TRUE;
}

/**
 * This routine can clear Halt Endpoint features on EPs.
 * 
 */
static void clear_feature(){

  // Send procedural stall if device isn't configured
  if((usb_state != DEV_CONFIGURED)
        // or request is made to host(remote wakeup not supported)
        || (ep0_setup.bmRequestType == OUT_DEVICE)
        // or request is made to interface
        || (ep0_setup.bmRequestType == OUT_INTERFACE)
        // or msbs of value or index set to non-zero value
        || ep0_setup.wValue.c[MSB] || ep0_setup.wIndex.c[MSB]
        // or data length set to non-zero.
        || ep0_setup.wLength.c[MSB] || ep0_setup.wLength.c[LSB]){
    return;
  }
    
  // Verify that packet was directed at an endpoint
  if((ep0_setup.bmRequestType != OUT_ENDPOINT)
        // the feature selected was HALT_ENDPOINT
        || (ep0_setup.wValue.c[LSB] != ENDPOINT_HALT)){
    return;
  }
  
  {
    BYTE ep_number = EP_NUMBER(ep0_setup.wIndex.c[LSB]);
    
    // the request was directed at IN_EP1, IN_EP2 or OUT_EP2
    
    // Clear feature endpoint halt
    // and Set endpoint status back to idle
    switch(ep0_setup.wIndex.c[LSB]){
      case IN_EP1:
      case IN_EP2:
      /*case IN_EP3:*/ {
        if(!(usb_ep_status(DIR_IN, ep_number) & EP_OWNED_BY_USER)){
          POLL_WRITE_BYTE (INDEX, ep_number);
          POLL_WRITE_BYTE (EINCSR1, rbInCLRDT  | rbInFLUSH);
          usb_ep_status(DIR_IN, ep_number) = EP_IDLE;
        }
        break;
      }
      
      /*case OUT_EP1:*/
      case OUT_EP2:
      /*case OUT_EP3:*/ {
        if(!(usb_ep_status(DIR_OUT, ep_number) & EP_OWNED_BY_USER)){
          POLL_WRITE_BYTE (INDEX, ep_number);
          POLL_WRITE_BYTE (EOUTCSR1, rbOutCLRDT  | rbOutFLUSH);
          usb_ep_status(DIR_OUT, ep_number) = EP_IDLE;
        }
        break;
      }
      
      default:
        // Send procedural stall
        return;
    }
  }
  
  ep0_request_completed = TRUE;
}

/**
 * This routine will set the EP Halt feature for EPs.
 * 
 */
static void set_feature(){

  // Make sure device is configured, setup data
  // is all valid and that request is directed at an endpoint
  if((usb_state != DEV_CONFIGURED)
          || (ep0_setup.bmRequestType == OUT_DEVICE)
          || (ep0_setup.bmRequestType == OUT_INTERFACE)
          || ep0_setup.wValue.c[MSB]  || ep0_setup.wIndex.c[MSB]
          || ep0_setup.wLength.c[MSB] || ep0_setup.wLength.c[LSB]){
    
    // Otherwise send stall to host
    return;
  }             

  // Make sure endpoint exists and that halt
  // endpoint feature is selected
  if((ep0_setup.bmRequestType != OUT_ENDPOINT)
        || (ep0_setup.wValue.c[LSB] != ENDPOINT_HALT)){
    // Send procedural stall
    return;
  }
  
  {
    BYTE ep_number = EP_NUMBER(ep0_setup.wIndex.c[LSB]);
    
    // Set feature endpoint halt
    switch(ep0_setup.wIndex.c[LSB]){
      case IN_EP1:
      case IN_EP2:
      /*case IN_EP3:*/ {
        if(!(usb_ep_status(DIR_IN, ep_number) & EP_OWNED_BY_USER)){
          POLL_WRITE_BYTE (INDEX, ep_number);
          POLL_WRITE_BYTE (EINCSR1, rbInSDSTL);       
          usb_ep_status(DIR_IN, ep_number) = EP_HALT;
        }
        break;
      }
      /*case OUT_EP1:*/
      case OUT_EP2:
      /*case OUT_EP3:*/ {
        if(!(usb_ep_status(DIR_OUT, ep_number) & EP_OWNED_BY_USER)){
          POLL_WRITE_BYTE (INDEX, ep_number);
          POLL_WRITE_BYTE (EOUTCSR1, rbOutSDSTL);         
          usb_ep_status(DIR_OUT, ep_number) = EP_HALT;
        }
        break;
      }
      
      default:
        // Send procedural stall
        return;
    }
  }
  
  ep0_request_completed = TRUE;
}

/**
 * Set new function address
 * 
 */
static void set_address(){
  
  // Request must be directed to device
  // with index and length set to zero.
  if((ep0_setup.bmRequestType != OUT_DEVICE)
          || ep0_setup.wIndex.c[MSB] || ep0_setup.wIndex.c[LSB]
          || ep0_setup.wLength.c[MSB] || ep0_setup.wLength.c[LSB]
          || ep0_setup.wValue.c[MSB]  || (ep0_setup.wValue.c[LSB] & 0x80)){
            
    // Send stall if setup data invalid
    return;
  }

  // Set endpoint zero to update address next status phase
  usb_ep0_status = EP_ADDRESS;
  if(ep0_setup.wValue.c[LSB] != 0){
    // Indicate that device state is now address
    usb_state = DEV_ADDRESS;
  }else{
    // If new address was 0x00, return device to default state
    usb_state = DEV_DEFAULT;                  
  }
  
  // Indicate setup packet has been serviced
  ep0_request_completed = TRUE;
}

/**
 * This routine sets the data pointer and size to correct 
 * descriptor and sets the endpoint status to transmit
 * 
 */
static void get_descriptor(){

  /* 
   * Determine which type of descriptor was requested, 
   * and set data ptr and size accordingly
   * @see USB spec. 9.4.3 get Descriptor
   */
  ep0_data.callback = NULL;
  switch(ep0_setup.wValue.c[MSB]){
    case DSC_TYPE_DEVICE:
      desc_device = &DESC_DEVICE;
      desc_config = &DESC_CONFIG;
      usb_mode = USB_CDC_READY;
      ep0_data.buf = (BYTE *)desc_device;
      ep0_data.size = desc_device->bLength;
      break;  
    case DSC_TYPE_CONFIG:  // config + interfaces + endpoints‘S•”•Ô‚·‚±‚Æ!!
      ep0_data.buf = (BYTE *)desc_config;
      ep0_data.size = desc_config->wTotalLength.i;
      break;
    case DSC_TYPE_STRING:
      if(ep0_setup.wValue.c[LSB] == 0){return;}
      ep0_data.buf = DESC_STRINGs[ep0_setup.wValue.c[LSB]-1];
      // Can have a maximum of 255 strings
      ep0_data.size = *ep0_data.buf;
      break;
    default:
      // Send Stall if unsupported request
      return;
  }
  
  // activate only when the requested descriptor is valid
  if ((ep0_setup.wLength.c[LSB] < ep0_data.size) && (ep0_setup.wLength.c[MSB] == 0)){
    ep0_data.size = ep0_setup.wLength.i;  // Send only requested amount of data
  }
  
  // Put endpoint in transmit mode
  usb_ep0_status = EP_TX;
  ep0_request_completed = TRUE;
}

/**
 * This routine returns current configuration value
 * 
 */
static void get_configuration(){

  // This request must be directed to the device
  if ((ep0_setup.bmRequestType != IN_DEVICE)
          // with value word set to zero
          || ep0_setup.wValue.c[MSB] || ep0_setup.wValue.c[LSB]
          // and index set to zero
          || ep0_setup.wIndex.c[MSB] || ep0_setup.wIndex.c[LSB]
          // and setup length set to one
          || ep0_setup.wLength.c[MSB] || (ep0_setup.wLength.c[LSB] != 1)){
    // Otherwise send a stall to host
    return;
  }

  if(usb_state == DEV_CONFIGURED){
    // If the device is configured, then return value 0x01  
    // since this software only supports one configuration
    ep0_register_data((BYTE*)&ONES_PACKET, 1);
  }else if(usb_state == DEV_ADDRESS){
    // If the device is in address state, it is not
    // configured, so return 0x00
    ep0_register_data((BYTE*)&ZERO_PACKET, 1);
  }

  // Put endpoint into transmit mode
  usb_ep0_status = EP_TX;
  ep0_request_completed = TRUE;
}

static void set_endpoints_configuration(){
  
  // Change index to endpoint 1
  POLL_WRITE_BYTE(INDEX, 1);
  // Set DIRSEL to indicate endpoint 1 is IN
  // and double-buffered
  POLL_WRITE_BYTE(EINCSR2, (rbInDIRSEL | rbInDBIEN));
  POLL_WRITE_BYTE(EINCSR1, (rbInCLRDT | rbInFLUSH));
  
  // Change index to endpoint 2
  POLL_WRITE_BYTE(INDEX, 2);
  // Set DIRSEL to indicate endpoint 2 is IN and OUT(Split mode) 
  // and double-buffered
  POLL_WRITE_BYTE(EINCSR2, (rbInDIRSEL | rbInSPLIT | rbInDBIEN));
  POLL_WRITE_BYTE(EINCSR1, (rbInCLRDT | rbInFLUSH));
  POLL_WRITE_BYTE(EOUTCSR2, rbOutDBOEN);
  POLL_WRITE_BYTE(EOUTCSR1, (rbOutCLRDT | rbOutFLUSH));
  
#if 0
  // Change index to endpoint 3
  POLL_WRITE_BYTE(INDEX, 3);
  
  // Set DIRSEL to indicate endpoint 3 is IN and OUT(Split mode) 
  // and double-buffered
  POLL_WRITE_BYTE(EINCSR2, (rbInDIRSEL | rbInSPLIT | rbInDBIEN));
  POLL_WRITE_BYTE(EINCSR1, (rbInCLRDT | rbInFLUSH));
  POLL_WRITE_BYTE(EOUTCSR2, rbOutDBOEN);
  POLL_WRITE_BYTE(EOUTCSR1, (rbOutCLRDT | rbOutFLUSH));
#endif
}

/**
 * This routine allows host to change current device configuration value
 * 
 */
static void set_configuration(){

  // Device must be addressed before configured
  if((usb_state == DEV_DEFAULT)
          // and request recipient must be the device
          || (ep0_setup.bmRequestType != OUT_DEVICE)
          // the index and length words must be zero
          || ep0_setup.wIndex.c[MSB] || ep0_setup.wIndex.c[LSB]
          || ep0_setup.wLength.c[MSB] || ep0_setup.wLength.c[LSB] 
          // This software only supports config = 0,1
          || ep0_setup.wValue.c[MSB] || (ep0_setup.wValue.c[LSB] > 1)){
    // Send stall if setup data is invalid
    return;
  }
  
  // Any positive configuration request
  if(ep0_setup.wValue.c[LSB] > 0){
    
    set_endpoints_configuration();
    
    // results in configuration being set to 1                                         
    usb_state = DEV_CONFIGURED;
    
    // Set endpoint status to idle (enabled)
    usb_ep_status(DIR_IN, 1) = EP_IDLE;
    //usb_ep_status(DIR_OUT, 1) = EP_IDLE;
    usb_ep_status(DIR_IN, 2) = EP_IDLE;
    usb_ep_status(DIR_OUT, 2) = EP_IDLE;
    //usb_ep_status(DIR_IN, 3) = EP_IDLE;
    //usb_ep_status(DIR_OUT, 3) = EP_IDLE;
  }else{
    
    // Unconfigures device by setting state to address, 
    // and changing endpoint 1 and 2 status to halt
    usb_state = DEV_ADDRESS;
    
    usb_ep_status(DIR_IN, 1) = EP_HALT;
    //usb_ep_status(DIR_OUT, 1) = EP_HALT;
    usb_ep_status(DIR_IN, 2) = EP_HALT;
    usb_ep_status(DIR_OUT, 2) = EP_HALT;
    //usb_ep_status(DIR_IN, 3) = EP_HALT;
    //usb_ep_status(DIR_OUT, 3) = EP_HALT;
  }     
  
  ep0_request_completed = TRUE;
}

/**
 * This routine returns 0x00, since only one interface
 * is supported by this firmware
 * 
 */
static void get_interface(){

  // If device is not configured
  if((usb_state != DEV_CONFIGURED)
        // or recipient is not an interface
        || (ep0_setup.bmRequestType != IN_INTERFACE)
        // or non-zero value or index fields
        || ep0_setup.wValue.c[MSB] || ep0_setup.wValue.c[LSB] 
        || (ep0_setup.wIndex.i >= desc_config->bNumInterfaces)
        // or data length not equal to one
        || ep0_setup.wLength.c[MSB] ||(ep0_setup.wLength.c[LSB] != 1)){
    // Then return stall due to invalid request
    return;
  }
  
  // Otherwise, return 0x00 to host
  ep0_register_data((BYTE*)&ZERO_PACKET, 1);
  
  // Set Serviced Setup packet, put endpoint in transmit
  // mode and reset Data sent counter         
  usb_ep0_status = EP_TX;
  ep0_request_completed = TRUE;
}

/**
 * This function sets interface if it's supported
 * 
 */
static void set_interface(){
  
  // Make sure request is directed at interface
  // and all other packet values are set to zero
  if((ep0_setup.bmRequestType != OUT_INTERFACE)
        || ep0_setup.wLength.c[MSB] || ep0_setup.wLength.c[LSB]
        || ep0_setup.wValue.c[MSB] || ep0_setup.wValue.c[LSB]
        || (ep0_setup.wIndex.i >= desc_config->bNumInterfaces)){
    // Othewise send a stall to host
    return;
  }
  
  set_endpoints_configuration();
  
  ep0_request_completed = TRUE;
}

void usb_standard_request(){
  switch(ep0_setup.bRequest){
    case GET_STATUS:        get_status();         break;
    case CLEAR_FEATURE:     clear_feature();      break;
    case SET_FEATURE:       set_feature();        break;
    case SET_ADDRESS:       set_address();        break;
    case GET_DESCRIPTOR:    get_descriptor();     break;
    case GET_CONFIGURATION: get_configuration();  break;
    case SET_CONFIGURATION: set_configuration();  break;
    case GET_INTERFACE:     get_interface();      break;
    case SET_INTERFACE:     set_interface();      break;
  }
}
