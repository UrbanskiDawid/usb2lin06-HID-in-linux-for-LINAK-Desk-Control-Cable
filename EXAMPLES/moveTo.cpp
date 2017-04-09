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
  usb2lin06::StatusReport r;

  const unsigned int max_a = 3; unsigned int a = max_a;//stuck protection
  uint16_t oldH=0;

  const int epsilon = 13;//this seems to be move prcision

  while(true)
  {
    DEBUGOUT("moveTo()",target)
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

int main (int argc,char **argv)
{
  libusb_context *ctx = NULL;
  libusb_device_handle* udev = NULL;
  bool succes=false;

  int16_t targetHeight=-1;//target height to move, form arg1
  
  DEBUGOUT("main() - start");
  {
    long long int tmp= atoll(argv[1]);
    targetHeight=tmp;

    if(argc!=2 || targetHeight<0 || tmp > (long long int)targetHeight)
    {
      printHelp();
      return -1;
    }
  }

  DEBUGOUT("main() - initlibusb");
  {
    if(libusb_init(&ctx)!=0)
    {
      cerr<<"Error failed to init libusb";
      return 1;
    }
    libusb_set_debug(ctx,LIBUSB_LOG_LEVEL);
  }

  DEBUGOUT("main() - find and open device");
  {
    udev = usb2lin06::openDevice();
    if(udev == NULL )
    {
      cerr<<"Error NO device";
      return 1;
    }
  }

  DEBUGOUT("main() - move to targetHeight",targetHeight);
  {
    if( moveTo(udev,targetHeight) )
    {
      cout<<"success"<<endl;
      succes=true;
    }
    else
      cout<<"failed"<<endl;
  }

  DEBUGOUT("main() - close");
  {
    libusb_close(udev);
    libusb_exit(ctx);
  }

  return (succes ? 0 : -1);
}
