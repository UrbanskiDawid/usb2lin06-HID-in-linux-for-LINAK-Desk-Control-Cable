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
  usb2lin06::statusReport report;

  if(usb2lin06::getStatus(udev,report))
  {
     h=usb2lin06::getHeight(report);
     return true;
  }

  return false;
}

/*
 * WARNING: dangerous
 * this will move towards goal
 * TODO: stop if cannot move
 */
float moveTo(libusb_device_handle* udev,float target)
{
  if(target<0.1f || target>100.0f) return -1;

  float d=0.0f,           oldD=0.0f,
        curHeight=0.0f,   oldHeight=666.6f;

  const float epsilon = 0.2f;

  const unsigned int delay = 150000;

  const int leftMax=5;
  int left=leftMax;

  std::cout.precision(1);
  while(true)
  {
    if(!getCurrentHeight(udev,curHeight)) { fprintf(stderr,"Error get status\n"); return -1; }

    cout<<"current height "<<std::fixed<<curHeight;

    oldD = d;
    d = (target-curHeight);

    if(fabs(d)<epsilon)      { break; }//close enough

    if(fabs(d-oldD)<epsilon) { left--; if(left==0) { fprintf(stderr,"WARNING: stuck"); return curHeight; } }
    else                       left=leftMax;

    if(d * oldD<0)           { break; }//to far

    usleep(delay);

    cout<<std::fixed<<"\tdelta "<<d<<"+/-"<<epsilon;

    if(d>0.0f)    { cout<<" moving up"<<endl; usb2lin06::moveUp  (udev);}
    else
    if(d<0.0f)    { cout<<" move down"<<endl; usb2lin06::moveDown(udev);}

    usleep(delay);
  }

  cout<<" destination reached "<<endl;
  return curHeight;
}

void printHelp()
{
  cout<<"this will set height of your desk using usb2lin06"<<endl
      <<"please start program with: arg1 (height)"<<endl;
}

#define LIBUSB_DEFAULT_TIMEOUT 1000
int main (int argc,char **argv)
{
  libusb_device_handle* udev = NULL;
  unsigned char buf[256];
  int ret=-1;

  float target=0.0f;

  //get target heigh & print help
  {
      if(argc<2)
      {
        printHelp();
      }else{
        target=::atof(argv[1]);
        if(target<0.0 || target > 100.0f)
        {
          printHelp();
        }
     }
  }

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
    printf("Lets look for the Linak device...\n");

    udev = usb2lin06::openDevice();
    if(udev == NULL )
    {
      fprintf(stderr, "Error NO device");
      return 1;
    }
    printf("INFO: found device\n");
  }

  //claim device
  {
    //Check whether a kernel driver is attached to interface #0. If so, we'll need to detach it.
    if (libusb_kernel_driver_active(udev, 0))
    {
      ret = libusb_detach_kernel_driver(udev, 0);
      if (ret != 0)
      {
        fprintf(stderr, "Error detaching kernel driver. %d\n",ret);
        return 1;
      }
    }

    // Claim interface #0
    ret = libusb_claim_interface(udev, 0);
    if (ret != 0)
    {
      fprintf(stderr, "Error claiming interface. %d\n",ret);
      return 1;
    }
  }

  //not 100% sure if this is needed
  {
    ret=libusb_control_transfer(udev, 0b00100001, 0x0a,0x0000,0,buf,0,LIBUSB_DEFAULT_TIMEOUT);
    if(ret<0)
    {
      fprintf(stderr,"Error to send control msg %d\n",ret);
    }
  }

  //getting status 1000times
  {
    float curHeight=0.0f;

    if(!getCurrentHeight(udev,curHeight))
    {
      fprintf(stderr,"Error getStatus\n");
    }else{

      cout<<"current height "<<curHeight<<endl;
      moveTo(udev,target);
    }
  }

  //cleanup
  {
    libusb_close(udev);
    libusb_exit(0);
  }
  return 0;
}
