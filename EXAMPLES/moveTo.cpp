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
bool getCurrentHeight(libusb_device_handle* udev,unsigned int &h)
{
  h=0;
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
float moveTo(libusb_device_handle* udev,double target,unsigned int epsilon = 20)
{
  double d=0.0f,           oldD=0.0f;
  unsigned int curHeight=0,   oldHeight=666;

  const unsigned int delay = 200000;

  const unsigned int leftMax=5;
  unsigned int left=leftMax;

  bool success=false;

  std::cout.precision(2);
  std::cout.setf( std::ios::fixed, std::ios::floatfield ); // float field set to fixed
  while(true)
  {
    if(!getCurrentHeight(udev,curHeight)) { fprintf(stderr,"Error get status\n"); break; }

    oldD = d;
    d = (target-curHeight);//distance go target

    cout<<endl<<"current height: "<<curHeight<<" distance: "<<d<<"+/-"<<epsilon<<" traveled: "<<(d-oldD)<<" ";

    if( fabs(d) < epsilon )      { success=true; break; }//close enough

    if(fabs(d-oldD)<epsilon) { left--; cout<<" bump (reason: "<<fabs(d-oldD)<<")"; if(left==0) { fprintf(stderr,"WARNING: stuck"); break; } }
    else                     left=leftMax;

    if(d * oldD<0)           { success=true; break; }//to far

    usleep(delay);

    if(d>0.0f)    { cout<<" moving up "; usb2lin06::moveUp  (udev);}
    else
    if(d<0.0f)    { cout<<" move down "; usb2lin06::moveDown(udev);}

    if(fabs(d) < 500)
    {
      cout<<"woooo";
      usleep(delay*1.6);
    }
    else
    usleep(delay);
  }//while

  if(success)
  {
    cout<<" success destination reached "<<endl;
  }//if

  usb2lin06::moveEnd(udev);
  return curHeight;
}

void printHelp()
{
  cout<<"this will set height of your desk using usb2lin06"<<endl
      <<"WARNING: this might be dangerous please make sure that you dont hit something!"<<endl
      <<"please start program with: arg1 (height)"<<endl;
}

#define LIBUSB_DEFAULT_TIMEOUT 1000
int main (int argc,char **argv)
{
  libusb_device_handle* udev = NULL;
  int ret=-1;

  unsigned int target=0.0f;

  //get target heigh & print help
  {
      if(argc==2)
      {
        target=::atoll(argv[1]);//long long
      }

      if(argc!=2 || target<0 || target > 9999)
      {
          printHelp();
          return -1;
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

  //get current height and move
  {
    unsigned int curHeight=0.0f;

    if(!getCurrentHeight(udev,curHeight))
    {
      fprintf(stderr,"Error getStatus\n");
    }else{
      cout<<"current height: "<<curHeight<<" target height: "<<target<<endl;

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
