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

struct statusReport//size:64B
{
  uint16_t header;       //[ 0, 1] 0x04, 0x38, 0x11  [End of Transmission, 8, Device Control 1 (oft. XON)]
  uint16_t unknown1;     //[ 2, 3] 0x11,0x08 << after movment (few seconds), 0x00,0x00 afterwards, 0x01,0x08 << while moving
  uint8_t  height[2];    //[ 4, 5] low,high 0x00 0x00 <<bottom
  uint8_t  moveDir;      //[  6  ] 0xe0 <<going down,0x10<< going up, 0xf0 starting going down
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
Get currnet status from device

URB_CONTROL IN
URB_CONTROL,URB_LEN=64,endpoint 0x80,IN

1. URB SUBMIT   ,DATA_LEN=0
host -> device

"Each request starts with a 8 byte long Setup Packet"
bmRequestType
  D7 Data Phase Transfer Direction
  1 = Device to Host
  D6..5 Type
  01 = Class
  D4..0 Recipient
  00001 = Interface
(bmRequestType)  (bRequest)  (wValue 16b)   (wIndex) (wLength)
(1010 0001)      (0000 0001) (0x0304    )   (0)      (64)

2. URB COMPLEATE,DATA_LEN=64
device -> host
control responce data
 00000000 00000100 00111000 00000000 00000000 00000000 00000000 00000000 00000000 .8...... ???  <<error heiht ??
 00000000 00000100 00111000 00010001 00001000 00000000 00000000 00000000 00000000 .8...... _min
 00000000 00000100 00111000 00010001 00001000 00011010 00000000 00000000 00000000 .8...... _min2
 00000000 00000100 00111000 00010001 00001000 00101111 00000000 00000000 00000000 .8../... _min3
 00000000 00000100 00111000 00010001 01000001 00101111 00000000 00000000 00000000 (simulated++)
                                    (-----------------) < wysokosc
                                     00000000
                                     00000001
                                     00000002
                                     00000000 00000003 etc
                                       10*0    10*1      10*3    (jade!)    10*5
 (-------------------------)const
 00000000 00000100 00111000 00010001 00001000 01101000 00000000 00000000 00000000 .8..h... min0
 00000000 00000100 00111000 00010001 00001000 01000001 00000010 00000000 00000000 .8..A...
 00000000 00000100 00111000 00010001 00001000 00000101 00000100 00000000 00000000 .8......
 00000000 00000100 00111000 00010001 00001000 10101001 00001000 00000000 00000000 .8......
 00000000 00000100 00111000 00010001 00001000 01010010 00011001 00000000 00000000 .8..R...
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



bool moveDown(libusb_device_handle * udev, int timeout=1000)
{
  int ret=-1;
  unsigned char buf[64];
  const unsigned char down[] = { 0x05, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f };

  memset (buf,0,sizeof(buf));
  memcpy (buf,down,sizeof(down));

  ret=libusb_control_transfer(
    udev,
    0x21,   //bmRequestType
    0x09,   //bRequest
    0x0305, //wValue
    0,      //wIndex,
    buf,64, //data, wLength
    timeout
  );
  return (64==ret);
}

bool moveUp(libusb_device_handle * udev, int timeout=1000)
{
  int ret=-1;
  unsigned char buf[64];
  const unsigned char   up[] = { 0x05, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80 };

  memset (buf,0,sizeof(buf));
  memcpy (buf,up,sizeof(up));

  ret=libusb_control_transfer(
    udev,
    0x21,   //bmRequestType
    0x09,   //bRequest
    0x0305, //wValue
    0,      //wIndex,
    buf,64, //data, wLength
    timeout
  );
  return (64==ret);
}


/*
 * this will return height form statusReport
 */
float getHeight(const statusReport &report)
{
  return ((float)(int)report.height[0])/257 + (unsigned int)report.height[1];
}

}
