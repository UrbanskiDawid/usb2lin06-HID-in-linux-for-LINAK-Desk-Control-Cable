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
 * WARNING: dangerous
 * this will move towards goal
 * TODO: stop if cannot move
 */
bool moveTo(  libusb_device_handle* udev, uint16_t target)
{
  usb2lin06::statusReport r;

  const unsigned int max_a = 3; unsigned int a = max_a;//stuck protection
  uint16_t oldH=0;

  const int epsilon = 13;//this seems to be move prcision

  while(true)
  {
    usb2lin06::move(udev,target);

    usleep(200000);

    if( usb2lin06::getStatusReport(udev,r) )
    {
      double distance = r.ref1cnt-r.ref1.pos;
      double delta    = oldH-r.ref1.pos;

      if(fabs(distance)<=epsilon | fabs(delta) <=epsilon | oldH==r.ref1.pos)
        a--;
      else
        a=max_a;

      cout
        <<"current height: "<<dec<<setw(5)<<setfill(' ')<<r.ref1.pos
        <<" target height: "<<dec<<setw(5)<<setfill(' ')<<target
        <<" distance:"<<dec<<setw(5)<<setfill(' ')<<distance
      <<endl;

      if(a==0) {break;}
      oldH=r.ref1.pos;
    }
  }//while

  return ( fabs(r.ref1.pos-target) <= epsilon);
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

  int16_t targetHeight=-1;//target height to move, form arg1

  //get target heigh & print help
  {
    long long int tmp= atoll(argv[1]);
    targetHeight=tmp;

    if(argc!=2 || targetHeight<0 || tmp > (long long int)targetHeight)
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
    udev = usb2lin06::openDevice();
    if(udev == NULL )
    {
      fprintf(stderr, "Error NO device");
      return 1;
    }
  }

  //move to targetHeight
  bool succes=false;
  {
    if( moveTo(udev,targetHeight) )
    {
      cout<<"success"<<endl;
      succes=true;
    }
    else
      cout<<"failed"<<endl;
  }

  //cleanup
  {
    libusb_close(udev);
    libusb_exit(0);
  }

  return (succes ? 0 : -1);
}
