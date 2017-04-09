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

int main (int argc,char **argv)
{
  libusb_context *ctx = NULL;
  libusb_device_handle* udev = NULL;

  DEBUGOUT("main() - init libusb");
  {
    if(libusb_init(&ctx)!=0)
    {
      cerr<<"ERROR: failed to init libusb"<<endl;
      return 1;
    }
    libusb_set_debug(ctx,LIBUSB_LOG_LEVEL);
  }

  DEBUGOUT("main() - find and openDevice");
  {
    udev = usb2lin06::openDevice();
    if(udev == NULL )
    {
      cerr<<"ERROR: NO device"<<endl;
      return 1;
    }
  }

  DEBUGOUT("main() - getHeigh");
  {
    float curHeight = 1.0f;
    if(!getCurrentHeight(udev,curHeight))
    {
      cerr<<"ERROR: getStatus"<<endl;
    }else{
      cout<<"current height: "<<setprecision(1)<<dec<<fixed<<curHeight<<"cm"<<endl;
    }
  }

  DEBUGOUT("main() - cleanup");
  {
    libusb_close(udev);
    libusb_exit(ctx);
  }
  return 0;
}
