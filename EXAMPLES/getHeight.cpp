#include <iostream>
#include <vector>
#include <string.h>     // std::string, memcpy
#include <iomanip>      // std::setw
#include <unistd.h>  //usleep
#include <sstream>  //std::ostringstream
#include <math.h>
#include "../usb2lin06.hpp"

using namespace std;


/*
 * get current height reported by device status
 */
bool getCurrentHeight(libusb_device_handle* udev,float &h)
{
  h=-1.0f;
  usb2lin06::StatusReport report;

  if(usb2lin06::getStatusReport(udev,report))
  {
     if(isStatusReportNotReady(report)) return false;

     h=usb2lin06::getHeightInCM(report);
     return true;
  }

  return false;
}

#define LIBUSB_DEFAULT_TIMEOUT 1000
int main (int argc,char **argv)
{
  libusb_context *ctx = NULL;
  libusb_device_handle* udev = NULL;
  unsigned char buf[256];

  DEBUGOUT("main() - init libusb");
  {
    if(libusb_init(&ctx)!=0)
    {
      fprintf(stderr, "Error failed to init libusb");
      return 1;
    }

    #ifdef DEBUG
    libusb_set_debug(ctx,LIBUSB_LOG_LEVEL_DEBUG);
    #else
    libusb_set_debug(ctx,LIBUSB_LOG_LEVEL_WARNING);
    #endif
  }

  DEBUGOUT("main() - find and openDevice");
  {
    udev = usb2lin06::openDevice();
    if(udev == NULL )
    {
      fprintf(stderr, "Error NO device");
      return 1;
    }
  }

  DEBUGOUT("main() - getHeigh");
  {

    float curHeight = 1.0f;
    if(!getCurrentHeight(udev,curHeight))
    {
      fprintf(stderr,"Error getStatus\n");
    }else{
      cout<<"current height: "<<setprecision(1)<<dec<<fixed<<curHeight<<"cm"<<endl;
    }
  }

  //cleanup
  DEBUGOUT("main() - cleanup");
  {
    libusb_close(udev);
    libusb_exit(ctx);
  }
  return 0;
}
