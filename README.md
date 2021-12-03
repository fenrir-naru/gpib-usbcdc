GPIB-USBCDC
===============

_GPIB-USBCDC_ is an interface bridge between GPIB (HPIB) and USB communication device class. 
It is [Prologix GPIB-USB adapter](http://prologix.biz/gpib-usb-controller.html) clone with [EFM8 Universal Bee](https://www.silabs.com/products/mcu/8-bit/efm8-universal-bee/pages/efm8-universal-bee.aspx) or [C8051F38x](http://www.silabs.com/products/mcu/8-bit/c8051f38x/Pages/c8051f38x.aspx) microcontroller.

# Quick user guide
  1. Connect _GPIB-USBCDC_ to the PC. _GPIB-USBCDC_ is recognized as serial port, which will be installed with [CDC inf file](https://raw.githubusercontent.com/fenrir-naru/gpib-usbcdc/master/firmware/inf/gpib-usbcdc_C8051F38x.inf).
  1. Other usage is almost same as [Prologix GPIB-USB adapter](http://prologix.biz/gpib-usb-controller.html). Please see that product manual.
  
# Manual
[Nice user manual](https://github.com/fenrir-naru/gpib-usbcdc/blob/master/docs/manual_by_Alessandro.pdf) written by Alessandro Soraruf is available, Thanks!
  
# Board
[EagleCAD](http://www.cadsoftusa.com/) files are available (ver.1 [schematics](https://github.com/fenrir-naru/gpib-usbcdc/blob/master/board/gpib-usbcdc.sch) and [layout](https://github.com/fenrir-naru/gpib-usbcdc/blob/master/board/gpib-usbcdc.brd). Its components are listed in [BOM](https://github.com/fenrir-naru/gpib-usbcdc#bom-bill-of-material). The board design is published under [Creative Commons Attribution-ShareAlike 4.0 International](http://creativecommons.org/licenses/by-sa/4.0/).

## BOM (bill of material)

<a href='https://github.com/fenrir-naru/gpib-usbcdc/blob/master/board/gpib-usbcdc_layout.png'><img src='https://raw.githubusercontent.com/fenrir-naru/gpib-usbcdc/master/board/gpib-usbcdc_layout.png' width='400px' /></a>

| **Part** | **Value** | **Package** | **Multiple** |
|:---------|:----------|:------------|:-------------|
| C1, C4  | 1u / 6V3 | 1005 | 2 |
| C2, C3, C5, C6 | 0.1u / 10V | 1005 | 4 |
| CON1 | [HRS ZX62R-B-5P](http://www.digikey.jp/product-detail/ja/ZX62R-B-5P/H11574CT-ND/1787106) | | 1 |
| CON2 | [Norcomp 111-024-113L001](http://www.digikey.jp/product-detail/ja/0/1024PMA-ND) | | 1 |
| CON3 | [JST ZH B4B-ZR](http://www.jst-mfg.com/product/detail_e.php?series=287) | | 1 | 
| IC1 | [EFM8UB20F32G-A-QFN32](https://www.silabs.com/products/mcu/8-bit/efm8-universal-bee/pages/EFM8UB20F32G-A-QFN32.aspx) or [C8051F387-GM](http://www.silabs.com/products/mcu/8-bit/c8051f38x/pages/C8051F387-GM.aspx) | QFN32 | 1 |
| IC2 | [SN75160BDWR](http://www.ti.com/product/sn75160b) or [SN75ALS160DWR](http://www.ti.com/product/sn75als160) | SO20 | 1 |
| IC3 | [SN75162BDWR](http://www.ti.com/product/sn75162b) or [SN75ALS162DWR](http://www.ti.com/product/sn75als162) | SO24 | 1 |
| LED1 | Red | 1608 | 1 |
| LED2 | Orange | 1608 | 1 |
| LED3 | Green | 1608 | 1 |
| LED4 | Blue | 1608 | 1 |
| R1, R2, R3, R4 | 470 | 1005 | 4 |
| R5 | 1K | 1005 | 1 |

# Firmware
The official binary is published in [github release](https://github.com/fenrir-naru/gpib-usbcdc/releases). To build the firmware by yourself, install [sdcc](http://sdcc.sourceforge.net/) (testing with [ver 3.3.0 #8604](http://sourceforge.net/projects/sdcc/files/sdcc/3.3.0/)), and just "make" at "firmware" directory of the downloaded [code](https://github.com/fenrir-naru/gpib-usbcdc/tree/master/firmware). The generated firmware name is  `gpib-usbcdc.hex`. The firmware code is published under [New BSD License](http://opensource.org/licenses/BSD-3-Clause). 

[![Build Status](https://travis-ci.org/fenrir-naru/gpib-usbcdc.svg?branch=master)](https://travis-ci.org/fenrir-naru/gpib-usbcdc)

## How to write firmware to hardware
Connect a _GPIB-USBCDC_ board and a PC via [USB debug adapter (UDA)](http://www.silabs.com/products/mcu/pages/usbdebug.aspx) or compatible one. The minimum required programming connections are summarized in the following table. Then, use [Flash Programming Utilities](http://www.silabs.com/products/mcu/Pages/8-bit-microcontroller-software.aspx#flash). Note: the board may not be recognized by a PC when an UDA is connected via USB hubs. UDA is recommended to connect a PC directly.

| **Signal** | **UDA side** | **_GPIB-USBCDC_ board side** |
|:-----------|:-------------|:---------------------------------|
| C2D        | 4th pin      | CON3 3rd pin                     |
| C2CK       | 7th pin      | CON3 4th pin                     |
| GND        | 3rd pin      | CON3 1st pin                     |

# Additional information
* The project owner's website is [Fenrir's BLog](http://fenrir.naruoka.org/).
