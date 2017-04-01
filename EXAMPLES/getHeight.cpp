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
     h=usb2lin06::getHeightInCM(report);
     return true;
  }

  return false;
}

#define LIBUSB_DEFAULT_TIMEOUT 1000
int main (int argc,char **argv)
{
  libusb_device_handle* udev = NULL;
  unsigned char buf[256];
  int ret=-1;

  //init libusb
  {
    if(libusb_init(0)!=0)
    {
      fprintf(stderr, "Error failed to init libusb");
      return 1;
    }
    libusb_set_debug(0,LIBUSB_LOG_LEVEL_WARNING);//and let usblib be verbose
  }

  //find and open device
  {
    udev = usb2lin06::openDevice();
    if(udev == NULL )
    {
      fprintf(stderr, "Error NO device");
      return 1;
    }
  }

  //getting height
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
  {
    libusb_close(udev);
    libusb_exit(0);
  }
  return 0;
}
