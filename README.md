GPIB-USBCDC
===============

_GPIB-USBCDC_ is an interface bridge between GPIB (HPIB) and USB communication device class. 
It is [Prologix GPIB-USB adapter](http://prologix.biz/gpib-usb-controller.html) clone with [EFM8 Universal Bee](https://www.silabs.com/products/mcu/8-bit/efm8-universal-bee/pages/efm8-universal-bee.aspx) or [C8051F38x](http://www.silabs.com/products/mcu/8-bit/c8051f38x/Pages/c8051f38x.aspx) microcontroller.

# Quick user guide
  1. Connect _GPIB-USBCDC_ to the PC. _GPIB-USBCDC_ is recognized as serial port, which will be installed with [CDC inf file](https://raw.githubusercontent.com/fenrir-naru/gpib-usbcdc/master/firmware/inf/gpib-usbcdc_C8051F38x.inf).

# BOM (bill of material)

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

## Additional information
* The project owner's website is [Fenrir's BLog](http://fenrir.naruoka.org/).
