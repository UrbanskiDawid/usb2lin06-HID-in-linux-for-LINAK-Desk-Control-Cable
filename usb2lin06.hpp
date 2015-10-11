/* usb2lin06
 *
 * Autor: Dawid Urbanski
 * email: git@dawidurbanski.pl
 * Dawid: 2 october 2015
 *
 */

#include <libusb-1.0/libusb.h>
#include <bitset>
#include <sys/types.h>

//some sources
//http://matthias.vallentin.net/blog/2007/04/writing-a-linux-kernel-driver-for-an-unknown-usb-device/
//from: http://www.beyondlogic.org/usbnutshell/usb5.shtml
//http://www.harshbutfair.org/software/example.c

//lubusb-1.0
// http://libusb.sourceforge.net/api-1.0/
//CONTROL URB https://www.safaribooksonline.com/library/view/linux-device-drivers/0596005903/ch13.html

namespace usb2lin06
{

/*
 u1: 0x0000,0x1008, 0x1108,0x0100,0x1000
  0x??08 < while starting to move, and afrer movment
  0x??00 < when moving or when stoped

  0x01??
  0x10??
  0x11??
 */
struct statusReport//size:64B
{
  uint16_t header;       //[ 0, 1] 0x04, 0x38 constant
  uint16_t unknown1;     //[ 2, 3] 0x1108 << after movment (few seconds), 0x0000 afterwards, 0x0108 << while moving
  uint16_t height;       //[ 4, 5] low,high 0x00 0x00 <<bottom
  uint8_t  moveDir;      //[  6  ] 0xe0 <<going down,0x10<< going up, 0xf0 starting/ending going down
  uint8_t  moveIndicator;//[  7  ] if != 0 them moving;
  uint8_t  unknown2[12]; //[ 8-19] zero ??
  uint8_t  unknown3[2];  //[20,21] 0x01 0x80 ??
  uint8_t  unknown4[10]; //[22-31] zero ??
  uint8_t  unknown5[3];  //[32,34] 0x01 0x00 0x36 ??
  uint8_t  unknown6[7];  //[35,41] zero ??
  uint16_t key;          //[42,43] button pressed down
  uint8_t  unknown7[14]; //[44-57]no work was done
  uint8_t  unknown8;     //[  58 ] 0x10/0x08 ??
  uint8_t  unknown9[4];  //[59-63] zero ??
};

/*
[ 6725.772231] usb 1-1.2: new full-speed USB device number 6 using ehci-pci
[ 6725.867477] usb 1-1.2: New USB device found, idVendor=12d3, idProduct=0002
[ 6725.867482] usb 1-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[ 6725.867484] usb 1-1.2: Product: USB Control Link
[ 6725.867486] usb 1-1.2: Manufacturer: Linak DeskLine A/S
[ 6725.867488] usb 1-1.2: SerialNumber: Љ
[ 6725.875451] hid-generic 0003:12D3:0002.0008: hiddev0,hidraw0: USB HID v1.11 Device [Linak DeskLine A/S USB Control Link] on usb-0000:00:1a.0-1.2/input0
*/
static struct libusb_device_handle *openDevice()
{
  const uint16_t
    vendor = 0x12d3,
    product = 0x0002;
  return libusb_open_device_with_vid_pid(0, vendor,product);
}

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
bool getStatus(libusb_device_handle* udev, statusReport &report)
{
  unsigned char buf[64]; //CONTROL responce data
  int ret = libusb_control_transfer(
     udev,
     0xa1,  //0b10100001 (mbRequest)
     1,     //request (bRequest)
     0x0304,//value (wValue)
     0,     //(wIndex)
     buf,   //bytes
     64,    //size (wLenght)
     3000   //timeout
     );
  if(ret!=64)
  {
    fprintf(stderr,"fail to get status request err%d\n",ret);
    /* Broken pipe is EPIPE. That means the device sent a STALL to your control
    message. If you don't know what a STALL is, check the USB specs.

    While the meaning of STALL varies from device to device and the
    situation, it usually means what you sent was wrong.
    I'd double check the request, requesttype, value and index arguments.
    I'd also double check the title_request buffer.*/
    return false;
  }

  //Debug Print Binary
  //for(int i=0;i<64;i++) {    cout<<std::bitset<8>(buf[i])<<" "; }

  //Debug Print Hex
  //for(int i=0;i<64;i++) {    cout<<setw(2)<<setfill('0')<<std::hex<<(int)(unsigned char)buf[i]<< " ";} cout<<endl;

  memcpy(&report, buf, sizeof(report));
  return (report.header==0x3804);
}

/*
 * please dont use this function
 * use: moveUp, moveDown, moveStop
 *
 * only 3 combinations are know:
 * DOWN:  05 ff  7f ff  7f ff  7f ff  7f	00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 * UP  :  05 00  80 00  80 00  80 00  80	00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 * END :  05 01  80 01  80 01  80 01  80	00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 */
inline bool _move(libusb_device_handle * udev, unsigned char a, unsigned char b, int timeout=1000)
{
  unsigned char data[64];
  memset (data,0,sizeof(data));

  const unsigned char header[] = {
    0x05, b,
    a,    b,
    a,    b,
    a,    b,
    a };
  memcpy (data,header,sizeof(header));

  int ret=libusb_control_transfer(
    udev,
    0x21,   //bmRequestType
    0x09,   //bRequest
    0x0305, //wValue-move
    0,      //wIndex,
    data,64, //data, wLength
    timeout
  );
  return (64==ret);
}

/*
 * Move one step down
 * this will send:
 * 05 ff
 * 7f ff
 * 7f ff
 * 7f ff
 * 7f 00
*/
bool moveDown(libusb_device_handle * udev, int timeout=1000)
{
  return _move(udev,0x7f,0xff,timeout);
}

/*
 * Move one step up
 * this will send:
 * 05 00
 * 80 00
 * 80 00
 * 80 00
 * 80 00
*/
bool moveUp(libusb_device_handle * udev, int timeout=1000)
{
  return _move(udev,0x80,0x00,timeout);
}

/*
 * End Movment sequence
 * this will send:
 * 05 01
 * 80 01
 * 80 01
 * 80 01
 * 80 00
*/
bool moveEnd(libusb_device_handle * udev, int timeout=1000)
{
  return _move(udev,0x80,0x01,timeout);
}

/*
 * this will calculate height form statusReport
 */
unsigned int getHeight(const statusReport &report)
{
  return (unsigned int)report.height;
}

/*
 * get height in centimeters, rough - precision: 1decimal points
 */
float getHeightInCM(const statusReport &report)
{
  return (float)report.height/98.0f;
}

}
