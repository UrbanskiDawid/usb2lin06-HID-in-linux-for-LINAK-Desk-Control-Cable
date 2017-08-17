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

namespace usb2lin06 {
namespace controler {

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

const statusReport* usb2lin06Controler::getStatusReport()
{
  DEBUGOUT("getStatusReport()");

  unsigned char *buf = getReportBuffer(true);
  buf[0]=StatusReport_ID;

  int ret = sendUSBcontrolTransfer(URB_getStatus,buf);
  if(ret!=StatusReportSize)
  {
    std::ostringstream msg; msg << "Failed to get statusReport "<<ret<<"!= "<<StatusReportSize;
    throw exception(RETURN_CODES::MESSAGE_ERROR,msg);
    /* Broken pipe is EPIPE. That means the device sent a STALL to your control
    message. If you don't know what a STALL is, check the USB specs.

    While the meaning of STALL varies from device to device and the
    situation, it usually means what you sent was wrong.
    I'd double check the request, requesttype, value and index arguments.
    I'd also double check the title_request buffer.*/
  }

#ifdef DEBUG
  std::cout<<"DEBUG: received ";
  buffer2stdout(buf,StatusReportSize);
#endif

  DEBUGOUT("getStatusReport() - check received data");
  {
    if(report.featureRaportID!=StatusReport_ID)
    {
      std::ostringstream msg; msg << "wrong featureRaportID: '"<<report.featureRaportID<<"' expected: '"<<StatusReport_ID;
      throw exception(RETURN_CODES::MESSAGE_ERROR,msg);
    }

    if(report.numberOfBytes!=StatusReport_nrOfBytes)
    {
      const int experimental=0x34;
      if(report.numberOfBytes!=experimental)
      {
          std::ostringstream msg; msg << "Livo exception! numberOfBytes'"<<report.numberOfBytes<<"' expected: '"<<experimental;
          throw exception(RETURN_CODES::MESSAGE_ERROR,msg);    
      }

      std::ostringstream msg; msg << "wrong numberOfBytes: '"<<report.numberOfBytes<<"' expected: '"<<StatusReport_nrOfBytes;
      throw exception(RETURN_CODES::MESSAGE_ERROR,msg);
    }
  }
  return &report;
}

const unsigned char* usb2lin06Controler::getExperimentalStatusReport()
{
  DEBUGOUT("getStatusReport()");

  unsigned char *buf = reinterpret_cast<unsigned char*>(&reportExperimental);
  memset(buf, 0, StatusReportSize);

  int ret = sendUSBcontrolTransfer(URB_getEx,buf);
  if(ret!=StatusReportSize)
  {
    std::ostringstream msg; msg <<"Failed to get statusReport "<<ret<<" != "<<StatusReportSize;
    throw exception(RETURN_CODES::MESSAGE_ERROR,msg);
  }

#ifdef DEBUG
  std::cout<<"reportEx:"<<endl;
  buffer2stdout(buf,StatusReportSize);
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
    unsigned char *buf = getReportBuffer(true);

    buf[0]=USB2LIN_featureReportID_modeOfOperation;         //0x03 Feature report ID = 3
    buf[1]=USB2LIN_featureReportID_modeOfOperation_default; //0x04 mode of operation
    buf[2]=0x00;                                            //?
    buf[3]=0xfb;                                            //?

    int ret = sendUSBcontrolTransfer(URB_init,buf);
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

bool findFirstDevice( //alternatrive: libusb_open_device_with_vid_pid
  libusb_context *ctx,
  struct libusb_device **found
  )
{
  DEBUGOUT("findFirstDevice()");

  *found = NULL;

	libusb_device **devs;
	if (libusb_get_device_list(ctx, &devs) < 0)
		return false;

	libusb_device_handle *dev_handle = NULL;
	size_t i = 0;

  libusb_device *dev;
	while ((dev = devs[i++]) != NULL)
  {
		libusb_device_descriptor desc;
		if (libusb_get_device_descriptor(dev, &desc)<0) continue;

		if (desc.idVendor == VENDOR && desc.idProduct == PRODUCT)
    {
      *found=dev;
      break;
    }
	}

  if(*found==NULL) {
    DEBUGOUT("findFirstDevice() - failed");
    return false;
  }

  DEBUGOUT("findFirstDevice() - success");
  return true;
}

void usb2lin06Controler::openDevice()
{
  DEBUGOUT("openDevice() find & open");
  {
    libusb_device *device;
    if(!findFirstDevice(ctx,&device))
    {
      throw exception(RETURN_CODES::DEVICE_CANT_FIND);
    }

    if(libusb_open(device, &udev)!=LIBUSB_SUCCESS)
    {
      throw exception(RETURN_CODES::DEVICE_CANT_OPEN);
    }
  }

  DEBUGOUT("openDevice() claim interface");
  {
    //Check whether a kernel driver is attached to interface #0. If so, we'll need to detach it.
    if (libusb_kernel_driver_active(udev, 0))
    {
      int libusbError = libusb_detach_kernel_driver(udev, 0);
      if(libusbError!=LIBUSB_SUCCESS)
      {
        throw exception(libusbError,"can't detaching kernel driver");
      }
      DEBUGOUT("openDevice() kernel driver detached");
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
  DEBUGOUT("%s %d", "move()",targetHeight);

  unsigned char data[StatusReportSize];
  memset (data,0,sizeof(data));

  data[0]=USB2LIN_featureReportID_controlCBC;
  memcpy( data+1, &targetHeight, sizeof(targetHeight) );
  memcpy( data+3, &targetHeight, sizeof(targetHeight) );
  memcpy( data+5, &targetHeight, sizeof(targetHeight) );
  memcpy( data+7, &targetHeight, sizeof(targetHeight) );

  int ret=sendUSBcontrolTransfer(URB_move,data);
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

float usb2lin06Controler::getHeightInCM() const
{
  return (float)report.ref1.pos/98.0f;
}

inline void usb2lin06Controler::buffer2stdout(const unsigned char * buf,const unsigned int num) const
{
  std::cout<<std::hex;
  for(int i=0;i<num;i++)  std::cout<<std::setw(2)<<std::setfill('0')<<(int)buf[i]<<" ";
  std::cout<<std::dec<<endl;
}

inline unsigned char * usb2lin06Controler::getReportBuffer(bool clear)
{
  unsigned char * buf = reinterpret_cast<unsigned char*>(&report);
  if(clear) memset(buf, 0, StatusReportSize);
  return buf;
}

int usb2lin06Controler::sendUSBcontrolTransfer(const sCtrlURB & urb, unsigned char * data)
{
  DEBUGOUT("%s %s","sendUSBcontrolTransfer()",data);

  #ifdef DEBUG
  std::cout<<"DEBUG: sending ";
  buffer2stdout(data,StatusReportSize);
  #endif  

  return libusb_control_transfer(
    udev,
    urb.bmRequestType,
    urb.bRequest,
    urb.wValue,
    urb.wIndex,
    data,
    urb.wLength,
    DefaultUSBtimeoutMS
  );  
}

}//namespace controler
}
