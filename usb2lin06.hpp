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
#include "usb2lin06.h"

void printLibStrErr(int errID)
{
  switch(errID)
  {
    case LIBUSB_ERROR_TIMEOUT: fprintf(stderr,"ERROR the transfer timed out (and populates transferred)"); break;
    case LIBUSB_ERROR_PIPE:    fprintf(stderr,"the endpoint halted"); break;
    case LIBUSB_ERROR_OVERFLOW: fprintf(stderr,"the device offered more data, see Packets and overflows"); break;
    case LIBUSB_ERROR_NO_DEVICE: fprintf(stderr,"the device has been disconnected"); break;
    default: fprintf(stderr,"another LIBUSB_ERROR code on other failures %d",errID);
  }
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
bool getStatusReport(libusb_device_handle* udev, statusReport &report, int timeout=1000)
{
  unsigned char buf[64]; //CONTROL responce data
  int ret = libusb_control_transfer(
     udev,
     URB_getStatus.bmRequestType,//0b10100001
     URB_getStatus.bRequest,
     URB_getStatus.wValue,
     URB_getStatus.wIndex,
     buf,
     URB_getStatus.wLength,
     timeout
     );
  if(ret!=64)
  {
    fprintf(stderr,"fail to get status request err %d!=64\n",ret);
    printLibStrErr(ret);
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
 * if status report is:
 * 0x04380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
 * the device is not ready!
*/
bool isStatusReportNotReady(const statusReport &report) {

  char report_bytes[64];
  memcpy(&report_bytes, &report, sizeof(report));
  if(report_bytes[0] != 0x04
     ||
     report_bytes[1] != 0x38) {
    return false; //THIS IS NOT A valid status report!
  }

  for(int i=2;i<64;i++)
  {
    if(report_bytes[i]!=0) return false;
  }

  return true;
}

bool moveEnd(libusb_device_handle * udev, int timeout=1000);


/*
 * init device, after poweron the device is not ready to work @see isStatusReportNotReady
 * this is the initial procedure. It has to be run once after device is powered on
 */
void initDevice(libusb_device_handle* udev)
{
  if(udev == NULL ) return;

  int timeout = 1000;

  //  USBHID 128b SET_REPORT Request bmRequestType 0x21 bRequest 0x09 wValue 0x0303 wIndex 0 wLength 64
  //  03 04 00 fb 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  {
    unsigned char buf[64];
    memset(buf, 0, 64);
    buf[0]=0x03;
    buf[1]=0x04;
    buf[2]=0x00;
    buf[3]=0xfb;

    int ret = libusb_control_transfer(
      udev,
      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,//      0x21,//host to device, Type: class, Recipient: Interface
      LIBUSB_REQUEST_SET_CONFIGURATION, //0x09 //HID_REPORT_SET
      0x0303,//Feature3, ReportID: 3
      0,
      buf,
      64,
      timeout
      );
    if(ret!=64)
    {
      fprintf(stderr,"device not ready - initializing failed on step1 ret=%d",ret);
      printLibStrErr(ret);
    }
  }

  usleep(1000);//must give device some time to think;)

  // USBHID 128b SET_REPORT Request bmRequestType 0x21 bRequest 0x09 wValue 0x0305 wIndex 0 wLength 64
  //  05 0180 0180 0180 0180 0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  {
    if(!moveEnd(udev)) fprintf(stderr,"device not ready - initializing failed on step2 (moveEnd)");
  }

  usleep(100000);//0.1sec //must give device some time to think;)

}


/*
[ 6725.772231] usb 1-1.2: new full-speed USB device number 6 using ehci-pci
[ 6725.867477] usb 1-1.2: New USB device found, idVendor=12d3, idProduct=0002
[ 6725.867482] usb 1-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[ 6725.867484] usb 1-1.2: Product: USB Control Link
[ 6725.867486] usb 1-1.2: Manufacturer: Linak DeskLine A/S
[ 6725.867488] usb 1-1.2: SerialNumber: Љ
[ 6725.875451] hid-generic 0003:12D3:0002.0008: hiddev0,hidraw0: USB HID v1.11 Device [Linak DeskLine A/S USB Control Link] on usb-0000:00:1a.0-1.2/input0
*/
static struct libusb_device_handle *openDevice(bool initialization=true)
{
  libusb_device_handle* udev = libusb_open_device_with_vid_pid(0,VENDOR,PRODUCT);
  if(udev==NULL) return NULL;

  //claim device
  {
    //Check whether a kernel driver is attached to interface #0. If so, we'll need to detach it.
    if (libusb_kernel_driver_active(udev, 0))
    {
      if (libusb_detach_kernel_driver(udev, 0) != 0)
      {
        fprintf(stderr, "Error detaching kernel driver.\n");
        return NULL;
      }
    }

    // Claim interface #0
    if (libusb_claim_interface(udev, 0) != 0)
    {
      fprintf(stderr, "Error claiming interface.\n");
      return NULL;
    }
  }

  //init device
  if(initialization)
  {
    statusReport report;
    if(!getStatusReport(udev, report))
    {
      fprintf(stderr, "Error geting init status report!\n");
    }else{
      if(isStatusReportNotReady(report))
      {
        initDevice(udev);
      }
    }
  }

  return udev;
}

/*
 * please dont use this function
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
 * question: target height is repeated 4times..can we move legs separately?
 */
inline bool move(libusb_device_handle * udev, int16_t targetHeight, int timeout=1000)
{
  unsigned char data[64];
  memset (data,0,sizeof(data));

  data[0]=0x05;
  memcpy( data+1, &targetHeight, sizeof(targetHeight) );
  memcpy( data+3, &targetHeight, sizeof(targetHeight) );
  memcpy( data+5, &targetHeight, sizeof(targetHeight) );
  memcpy( data+7, &targetHeight, sizeof(targetHeight) );

  int ret=libusb_control_transfer(
    udev,
    URB_move.bmRequestType,
    URB_move.bRequest,
    URB_move.wValue,
    URB_move.wIndex,
    data,
    URB_move.wLength,
    timeout
  );
  return (64==ret);
}

/*
 * Move one step down
 * this will send:
 * 05 ff 7f ff 7f ff 7f ff 7f 00
*/
bool moveDown(libusb_device_handle * udev, int timeout=1000)
{
  return move(udev,0x7fff,timeout);
}

/*
 * Move one step up
 * this will send:
 * 05 00 80 00 80 00 80 00 80 00
*/
bool moveUp(libusb_device_handle * udev, int timeout=1000)
{
  return move(udev,0x8000,timeout);
}

/*
 * End Movment sequence
 * this will send:
 * 05 01 80 01 80 01 80 01 80 00
*/
bool moveEnd(libusb_device_handle * udev, int timeout)
{
  return move(udev,0x8001,timeout);
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
