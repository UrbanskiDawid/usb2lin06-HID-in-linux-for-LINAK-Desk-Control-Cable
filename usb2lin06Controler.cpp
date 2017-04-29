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
#include <bitset>

namespace usb2lin06
{

using std::endl;


usb2lin06Controler::usb2lin06Controler(bool initialization)
{
  DEBUGOUT("usb2lin06Controler()");
  {
    int libusbError = libusb_init(&ctx);
    if(libusbError!=LIBUSB_SUCCESS) { throw exception(libusbError,"libusb_init failed"); }
    libusb_set_debug(ctx,LIBUSB_LOG_LEVEL); 
  } 

  DEBUGOUT("usb2lin06Controler() - find and open device");
  {
    try{
      openDevice();
      if(initialization) initDevice();
    }catch(exception e){
      libusb_close(udev); udev=NULL;
      libusb_exit(ctx);   ctx=NULL;
      throw e;
    }
  }
}

usb2lin06Controler::~usb2lin06Controler()
{
    DEBUGOUT("~usb2lin06Controler()");
    if(udev) libusb_close(udev);
    if(ctx)  libusb_exit(ctx);
}

StatusReportEx* usb2lin06Controler::getStatusReport()
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
    std::ostringstream msg; msg << "Failed to get statusReport "<<ret<<"!= "<<StatusReportSize;
    throw exception(RETURN_CODES::MESSAGE_ERROR,msg.str());
    /* Broken pipe is EPIPE. That means the device sent a STALL to your control
    message. If you don't know what a STALL is, check the USB specs.

    While the meaning of STALL varies from device to device and the
    situation, it usually means what you sent was wrong.
    I'd double check the request, requesttype, value and index arguments.
    I'd also double check the title_request buffer.*/
  }

#ifdef DEBUG
  std::cout<<"DEBUG: received ";
  for(int i=0;i<StatusReportSize;i++) std::cout<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)(unsigned char)buf[i]<< " ";
  std::cout<<std::endl;
#endif

  DEBUGOUT("getStatusReport() - check received data");
  {
    if(report.featureRaportID!=StatusReport_ID)
    {
      std::ostringstream msg; msg << "wrong featureRaportID: '"<<report.featureRaportID<<"' expected: '"<<StatusReport_ID;
      throw exception(RETURN_CODES::MESSAGE_ERROR,msg.str());
    }

    const int experimental=0x34;
    if(report.numberOfBytes!=StatusReport_nrOfBytes && report.numberOfBytes!=experimental)
    {
      std::ostringstream msg; msg << "wrong numberOfBytes: '"<<report.numberOfBytes<<"' expected: '"<<StatusReport_nrOfBytes<<"' or '"<<experimental<<"'",
      throw exception(RETURN_CODES::MESSAGE_ERROR,msg.str());
    }
  }
  return &report;
}

unsigned char* usb2lin06Controler::getStatusReportEx()
{
  DEBUGOUT("getStatusReport()");

  unsigned char *buf = reinterpret_cast<unsigned char*>(&reportExperimental);
  memset(buf, 0, StatusReportSize);

  int ret = libusb_control_transfer(
     udev,
     URB_getEx.bmRequestType,
     URB_getEx.bRequest,
     URB_getEx.wValue,
     URB_getEx.wIndex,
     buf,
     URB_getEx.wLength,
     DefaultUSBtimeoutMS
     );
  if(ret!=StatusReportSize)
  {
    std::ostringstream msg; msg <<"Failed to get statusReportEx "<<ret<<" != "<<StatusReportSize;
    throw exception(RETURN_CODES::MESSAGE_ERROR,msg.str());
  }

#ifdef DEBUG
  std::cout<<"reportEx:"<<endl;
  for(int i=0;i<StatusReportSize;i++) std::cout<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)buf[i]<< " ";
  std::cout<<endl;
#endif
  return buf;
}

void usb2lin06Controler::initDevice()
{
  DEBUGOUT("initDevice()");

  if(udev == NULL ) return;

  getStatusReport();

  if(!report.isStatusReportNotReady()) return;

  //  USBHID 128b SET_REPORT Request bmRequestType 0x21 bRequest 0x09 wValue 0x0303 wIndex 0 wLength 64
  //  03 04 00 fb 000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  {
    unsigned char buf[StatusReportSize];
    memset(buf, 0, StatusReportSize);
    buf[0]=USB2LIN_featureReportID_modeOfOperation;         //0x03 Feature report ID = 3
    buf[1]=USB2LIN_featureReportID_modeOfOperation_default; //0x04 mode of operation
    buf[2]=0x00;                                            //?
    buf[3]=0xfb;                                            //?

    int ret = libusb_control_transfer(
      udev,
      URB_init.bmRequestType,
      URB_init.bRequest,
      URB_init.wValue,
      URB_init.wIndex,
      buf,
      URB_init.wLength,
      DefaultUSBtimeoutMS
      );
    if(ret!=StatusReportSize)
    {
      throw exception(RETURN_CODES::DEVICE_CANT_INIT,"device not ready - initializing failed on step1 ret="+ret);
    }
  }

  usleep(1000);//must give device some time to think;)

  // USBHID 128b SET_REPORT Request bmRequestType 0x21 bRequest 0x09 wValue 0x0305 wIndex 0 wLength 64
  //  05 0180 0180 0180 0180 0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  {
    if(!moveEnd())
    {
      throw exception(RETURN_CODES::DEVICE_CANT_INIT,"device not ready - initializing failed on step2 (moveEnd)");
    }
  }

  usleep(100000);//0.1sec //must give device some time to think;)

  return;
}

libusb_device *findFirstDevice(libusb_context *context)
{
    libusb_device **list = NULL;
    int countOrErrorCode = libusb_get_device_list(context, &list);
    if(countOrErrorCode<0) throw exception(countOrErrorCode,"libusb_get_device_list failed");

    int ret=-1;
    for (size_t idx = 0; idx < countOrErrorCode; ++idx)
    {
        libusb_device *device = list[idx];
        if(ret==-1)
        {
          libusb_device_descriptor desc;
          if(libusb_get_device_descriptor(device, &desc) == LIBUSB_SUCCESS)
          {
            if(desc.idVendor == VENDOR && desc.idProduct == PRODUCT){
                ret=idx;
                continue; //note no libusb_unref_device
            }
          }
        }
        libusb_unref_device(device);
    }

    libusb_free_device_list(list,0);

    return (ret==-1 ? NULL : list[ret]);
}

void usb2lin06Controler::openDevice()
{
  DEBUGOUT("openDevice() find & open");
  {
    libusb_device *device = findFirstDevice(ctx);
    if(device==NULL)
    {
      throw exception(RETURN_CODES::DEVICE_CANT_FIND);
    }

    int errorCode = libusb_open(device, &udev);   //udev = libusb_open_device_with_vid_pid(0,VENDOR,PRODUCT);
    if(errorCode!=LIBUSB_SUCCESS)
    {
      throw exception(RETURN_CODES::DEVICE_CANT_OPEN);
    }
  }

  DEBUGOUT("openDevice() claim device");
  {
    //Check whether a kernel driver is attached to interface #0. If so, we'll need to detach it.
    if (libusb_kernel_driver_active(udev, 0))
    {
      int libusbError = libusb_detach_kernel_driver(udev, 0);
      if(libusbError!=LIBUSB_SUCCESS)
      {
        throw exception(libusbError,"can't detaching kernel driver");
      }
    }

    // Claim interface #0
    int libusbError =libusb_claim_interface(udev, 0);
    if(libusbError!=LIBUSB_SUCCESS)
    {
      throw exception(libusbError,"can't detachingclaiming interface.");
    }
  }
}


bool usb2lin06Controler::move(int16_t targetHeight)
{
  DEBUGOUT("move()",targetHeight);

  unsigned char data[StatusReportSize];
  memset (data,0,sizeof(data));

  data[0]=USB2LIN_featureReportID_controlCBC;
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


bool usb2lin06Controler::moveDown()
{
  return move(HEIGHT_moveDownwards);
}


bool usb2lin06Controler::moveUp()
{
  return move(HEIGHT_moveUpwards);
}


bool usb2lin06Controler::moveEnd()
{
  return move(HEIGHT_moveEnd);
}


int usb2lin06Controler::getHeight()
{
  return (int)report.ref1.pos;
}

float usb2lin06Controler::getHeightInCM()
{
  return (float)report.ref1.pos/98.0f;
}

}
