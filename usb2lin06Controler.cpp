/* usb2lin06
 *
 * Autor: Dawid Urbanski
 * email: git@dawidurbanski.pl
 * Dawid: 9 apr 2017
 *
 */

#include "usb2lin06Controler.h"
#include <stdexcept>
#include <string.h>
#include <unistd.h> //usleep
#include <iostream>
#include <iomanip>  // std::setw

namespace usb2lin06
{

void printLibStrErr(int errID)
{
  switch(errID)
  {
    case LIBUSB_ERROR_TIMEOUT:   fprintf(stderr,"ERROR the transfer timed out (and populates transferred)"); break;
    case LIBUSB_ERROR_PIPE:      fprintf(stderr,"the endpoint halted"); break;
    case LIBUSB_ERROR_OVERFLOW:  fprintf(stderr,"the device offered more data, see Packets and overflows"); break;
    case LIBUSB_ERROR_NO_DEVICE: fprintf(stderr,"the device has been disconnected"); break;
    default:                     fprintf(stderr,"another LIBUSB_ERROR code on other failures %d",errID);
  }
}

bool isStatusReportNotReady(const StatusReport &report)
{
  DEBUGOUT("isStatusReportNotReady");

  char report_bytes[StatusReportSize];
  memcpy(&report_bytes, &report, sizeof(report));
  if(report_bytes[0] != StatusReport_ID
     ||
     report_bytes[1] != StatusReport_nrOfBytes) {
    return false; //THIS IS NOT A valid status report!
  }

  for(int i=2;i<StatusReportSize-6;i++)/*check bits: 2..58*/
  {
    if(report_bytes[i]!=0) return false;
  }

  return true;
}


usb2lin06Controler::usb2lin06Controler(bool initialization)
{
  DEBUGOUT("usb2lin06Controler()");
  {
    if(libusb_init(&ctx)!=0) throw std::runtime_error("failed to init libusb");
    libusb_set_debug(ctx,LIBUSB_LOG_LEVEL); 
  } 

  DEBUGOUT("usb2lin06Controler() - find and open device");
  {
    if(!openDevice())
    throw std::runtime_error("can't open device");
  }

  if(initialization) initDevice();  
}

usb2lin06Controler::~usb2lin06Controler()
{
    DEBUGOUT("~usb2lin06Controler()");
    if(udev) libusb_close(udev);
    if(ctx)  libusb_exit(ctx);
}

bool usb2lin06Controler::getStatusReport()
{
  DEBUGOUT("getStatusReport()");

  unsigned char *buf = reinterpret_cast<unsigned char*>(&report);
  memset(buf, 0, StatusReportSize);

  int ret = libusb_control_transfer(
     udev,
     URB_getStatus.bmRequestType,
     URB_getStatus.bRequest,
     URB_getStatus.wValue,
     URB_getStatus.wIndex,
     buf,
     URB_getStatus.wLength,
     DefaultUSBtimeoutMS
     );
  if(ret!=StatusReportSize)
  {
    fprintf(stderr,"fail to get status request err %d!=%d\n",ret,StatusReportSize);
    printLibStrErr(ret);
    /* Broken pipe is EPIPE. That means the device sent a STALL to your control
    message. If you don't know what a STALL is, check the USB specs.

    While the meaning of STALL varies from device to device and the
    situation, it usually means what you sent was wrong.
    I'd double check the request, requesttype, value and index arguments.
    I'd also double check the title_request buffer.*/
    return false;
  }

#ifdef DEBUG
  std::cout<<"DEBUG: received ";
  for(int i=0;i<StatusReportSize;i++) {
    std::cout<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)(unsigned char)buf[i]<< " ";
  }
  std::cout<<std::endl;
#endif

  bool success=true;
  DEBUGOUT("getStatusReport() - check received data");
  {
    if(report.featureRaportID!=StatusReport_ID)
    {
      fprintf(stderr,"ERROR: wrong featureRaportID: '%d' expected: '%d'\n",report.featureRaportID,StatusReport_ID);
      success=false;
    }

    const int experimental=0x34;
    if(report.numberOfBytes!=StatusReport_nrOfBytes && report.numberOfBytes!=experimental)
    {
      fprintf(stderr,"ERROR: wrong numberOfBytes: '%d' expected: '%d' or '%d'\n",report.numberOfBytes,StatusReport_nrOfBytes,experimental);
      success=false;
    }
  }

  return success;
}

bool usb2lin06Controler::initDevice()
{
  DEBUGOUT("initDevice()");

  if(udev == NULL ) return false;

  if(!getStatusReport())
  {
    fprintf(stderr, "Error geting init status report!\n");
    return false;
  }

  if(!isStatusReportNotReady(report)) return false;

  //  USBHID 128b SET_REPORT Request bmRequestType 0x21 bRequest 0x09 wValue 0x0303 wIndex 0 wLength 64
  //  03 04 00 fb 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  {
    unsigned char buf[StatusReportSize];
    memset(buf, 0, StatusReportSize);
    buf[0]=USB2LIN_modeOfOperation_featureReportID; //0x03 Feature report ID = 3
    buf[1]=USB2LIN_ModeOfOperation_default;         //0x04 mode of operation
    buf[2]=0x00;                                    //?
    buf[3]=0xfb;                                    //?

    int ret = libusb_control_transfer(
      udev,
      LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,//      0x21,//host to device, Type: class, Recipient: Interface
      LIBUSB_REQUEST_SET_CONFIGURATION, //0x09 //HID_REPORT_SET
      0x0303,//Feature3, ReportID: 3
      0,
      buf,
      StatusReportSize,
      DefaultUSBtimeoutMS
      );
    if(ret!=StatusReportSize)
    {
      fprintf(stderr,"device not ready - initializing failed on step1 ret=%d",ret);
      printLibStrErr(ret);
      return false;
    }
  }

  usleep(1000);//must give device some time to think;)

  // USBHID 128b SET_REPORT Request bmRequestType 0x21 bRequest 0x09 wValue 0x0305 wIndex 0 wLength 64
  //  05 0180 0180 0180 0180 0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  {
    if(!moveEnd())
    {
      fprintf(stderr,"device not ready - initializing failed on step2 (moveEnd)");
      return false;
    }
  }

  usleep(100000);//0.1sec //must give device some time to think;)

  return true;
}


bool usb2lin06Controler::openDevice()
{
  DEBUGOUT("openDevice()");

  udev = libusb_open_device_with_vid_pid(0,VENDOR,PRODUCT);
  if(udev==NULL)
  {
    fprintf(stderr, "Error cant find usb device %d %d\n",VENDOR,PRODUCT);
    return false;
  }

  DEBUGOUT("openDevice() claim device");
  {
    //Check whether a kernel driver is attached to interface #0. If so, we'll need to detach it.
    if (libusb_kernel_driver_active(udev, 0))
    {
      if (libusb_detach_kernel_driver(udev, 0) != 0)
      {
        fprintf(stderr, "Error detaching kernel driver.\n");
        return false;
      }
    }

    // Claim interface #0
    if (libusb_claim_interface(udev, 0) != 0)
    {
      fprintf(stderr, "Error claiming interface.\n");
      return false;
    }
  }

  return true;
}


bool usb2lin06Controler::move(int16_t targetHeight)
{
  DEBUGOUT("move()",targetHeight);

  unsigned char data[StatusReportSize];
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
    DefaultUSBtimeoutMS
  );
  return (ret==StatusReportSize);
}

/*
 * Move one step down
 * this will send:
 * 05 ff 7f ff 7f ff 7f ff 7f 00
*/
bool usb2lin06Controler::moveDown()
{
  return move(HEIGHT_moveDownwards);
}

/*
 * Move one step up
 * this will send:
 * 05 00 80 00 80 00 80 00 80 00
*/
bool usb2lin06Controler::moveUp()
{
  return move(HEIGHT_moveUpwards);
}

/*
 * End Movment sequence
 * this will send:
 * 05 01 80 01 80 01 80 01 80 00
*/
bool usb2lin06Controler::moveEnd()
{
  return move(HEIGHT_moveEnd);
}

/*
 * this will calculate height form StatusReport
 */
int usb2lin06Controler::getHeight()
{
  return (int)report.ref1.pos;
}

/*
 * get height in centimeters, rough - precision: 1decimal points
 */
float usb2lin06Controler::getHeightInCM()
{
  return (float)report.ref1.pos/98.0f;
}

}
