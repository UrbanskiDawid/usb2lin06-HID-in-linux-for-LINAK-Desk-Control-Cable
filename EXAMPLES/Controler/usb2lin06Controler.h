/* usb2lin06
 *
 * Autor: Dawid Urbanski
 * email: git@dawidurbanski.pl
 * Dawid: 2 october 2015
 *
 */
#pragma once

#include <libusb.h>
#include "usb2lin06.h"
#include "statusReport.h"
#include "usb2linException.h"
#include <iostream>

//some sources
//http://matthias.vallentin.net/blog/2007/04/writing-a-linux-kernel-driver-for-an-unknown-usb-device/
//from: http://www.beyondlogic.org/usbnutshell/usb5.shtml
//http://www.harshbutfair.org/software/example.c

//lubusb-1.0
// http://libusb.sourceforge.net/api-1.0/
//CONTROL URB https://www.safaribooksonline.com/library/view/linux-device-drivers/0596005903/ch13.html
#ifdef DEBUG
  #define DEBUGOUT(...) fprintf(stderr, "DEBUG: %s:%d", __FILE__, __LINE__); fprintf (stderr, __VA_ARGS__)
  #define LIBUSB_LOG_LEVEL LIBUSB_LOG_LEVEL_DEBUG
#else
  #define DEBUGOUT(msg,...) ;
  #define LIBUSB_LOG_LEVEL LIBUSB_LOG_LEVEL_WARNING
#endif

namespace usb2lin06 {
namespace controler {

struct usb2lin06Controler
{
usb2lin06Controler(bool initialization=true);
~usb2lin06Controler();

libusb_context *ctx = NULL;
libusb_device_handle* udev = NULL;
statusReport report;

/*Experimental - unknown structure:( */
unsigned char reportExperimental[64];

/*
 * Get currnet status from device
 * to get status we have to send a request.
 * request is send using USB control transfer:

	(bmRequestType)  (bRequest)  (wValue 16b)   (wIndex) (wLength)
	(1010 0001)      (0000 0001) (0x0304    )   (0)      (64)

        bmRequestType 1B "determine the direction of the request, type of request and designated recipien"
          0b10100001
	    D7 Data Phase Transfer Direction - 1 = Device to Host
	    D6..5 Type                       - 01 = Class
	    D4..0 Recipient                  - 00001 = Interface
	bRequest 1B "determines the request being made"
	  0b00000001
	wValue 2B "parameter to be passed with the request"
	  0x0304
	wIndex 2B "parameter to be passed with the request"
	  0x00
	wLength 2B "number of bytes to be transferred"
	  64
 * anwswer to this contains 64B of data example:
  04380000ee07000000000000000000000000000001800000000000000000000001001000000000000000ffff0000000000000000000000000000100000000000
*/
const statusReport* getStatusReport();

/*Experimental*/
const unsigned char* getExperimentalStatusReport();

/*
[ 6725.772231] usb 1-1.2: new full-speed USB device number 6 using ehci-pci
[ 6725.867477] usb 1-1.2: New USB device found, idVendor=12d3, idProduct=0002
[ 6725.867482] usb 1-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[ 6725.867484] usb 1-1.2: Product: USB Control Link
[ 6725.867486] usb 1-1.2: Manufacturer: Linak DeskLine A/S
[ 6725.867488] usb 1-1.2: SerialNumber: Љ
[ 6725.875451] hid-generic 0003:12D3:0002.0008: hiddev0,hidraw0: USB HID v1.11 Device [Linak DeskLine A/S USB Control Link] on usb-0000:00:1a.0-1.2/input0
*/
void openDevice();

/*
 * init device, after poweron the device is not ready to work @see isStatusReportNotReady
 * this is the initial procedure. It has to be run once after device is powered on
 */
void initDevice();

/*
 * use: moveUp, moveDown, moveStop
 *
 * only 3 combinations are know:
 * DOWN:  05 ff 7f ff 7f ff 7f ff 7f	00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 * UP  :  05 00 80 00 80 00 80 00 80	00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 * END :  05 01 80 01 80 01 80 01 80	00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 *
 *
 * [0x05] -const value (8-bit)
 * [H]    - target height (16-bit)
 * [H]    - target height (16-bit)
 * [H]    - target height (16-bit)
 * [H]    - target height (16-bit)
 *
 */
bool move(int16_t targetHeight);

/*
 * Move one step down
 * this will send:
 * 05 ff 7f ff 7f ff 7f ff 7f 00
*/
bool moveDown();

/*
 * Move one step up
 * this will send:
 * 05 00 80 00 80 00 80 00 80 00
*/
bool moveUp();

/*
 * End Movment sequence
 * this will send:
 * 05 01 80 01 80 01 80 01 80 00
*/
bool moveEnd();

/*
 * this will calculate height form StatusReport
 */
int getHeight();

/*
 * get height in centimeters, rough - precision: 1decimal points
 */
float getHeightInCM() const;

private:
	/*
	* get report as uchar buffer
	*/
	inline unsigned char * getReportBuffer(bool clear=false);

	/*
	 * 'print' buffer to stdout
	 */
	inline void buffer2stdout(const unsigned char * buffer,const unsigned int num) const;

	/*
	 * wrapper for libusb_control_transfer
	 */
	int sendUSBcontrolTransfer(const sCtrlURB & urb, unsigned char * data);
};//usb2lin06Controler

}//namespace controler
}//namespace
