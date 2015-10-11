#include <iostream>
#include <vector>
#include <string.h>     // std::string, memcpy
#include <iomanip>      // std::setw
#include <unistd.h>  //usleep
#include <sstream>  //std::ostringstream

#include "../usb2lin06.hpp"

using namespace std;
/*
 * just some universal device descripor print out
 */
void printDescriptor(struct libusb_device_handle *udev)
{
  struct libusb_device * dev = libusb_get_device(udev);

  struct libusb_device_descriptor des;
  libusb_get_device_descriptor(dev,&des);

  struct keyName { std::string name; unsigned int value; std::string desc; };
  std::vector<keyName> data =
  {
    {        "bLength",              des.bLength,            "Size of the Descriptor in Bytes (18 bytes)"},
    {        "bDescriptorType",      des.bDescriptorType,    "Constant        Device Descriptor (0x01)"},
    {        "bcdUSB",               des.bcdUSB,             "USB Specification Number which device complies too."},
    {        "bDeviceClass",         des.bDeviceClass,       "Class   Class Code (Assigned by USB Org)"},
    {        "bDeviceSubClass",      des.bDeviceSubClass,    "SubClass        Subclass Code (Assigned by USB Org)"},
    {        "bDeviceProtocol",      des.bDeviceProtocol,    "Protocol        Protocol Code (Assigned by USB Org)"},
    {        "bMaxPacketSize0",      des.bMaxPacketSize0,    "Number  Maximum Packet Size for Zero Endpoint."},
    {        "idVendor",             des.idVendor,           "ID      Vendor ID (Assigned by USB Org)"},
    {        "idProduct",            des.idProduct,          "ID      Product ID (Assigned by Manufacturer)"},
    {        "bcdDevice",            des.bcdDevice,          "BCD     Device Release Number"},
    {        "iManufacturer",        des.iManufacturer,      "Index   Index of Manufacturer String Descriptor"},
    {        "iProduct",             des.iProduct,           "Index   Index of Product String Descriptor"},
    {        "iSerialNumber",        des.iSerialNumber,      "Index   Index of Serial Number String Descriptor"},
    {        "bNumConfigurations: ", des.bNumConfigurations, "1       Integer Number of Possible Configurations"}
  };

  cout<<"Descriptor (values in HEX)"<<endl;
  for ( auto d:data)
  {
    cout<<std::setw(20)<<d.name<<":"
      <<setw(10)<<hex<<d.value<<" "
      <<d.desc<<endl;
  }
}

void printStatusReport(const usb2lin06::statusReport &report)
{
  bool  isMoving= (report.moveIndicator!=0);
  bool  isBottom= (report.height[0]==0 && report.height[1]==0);
  float height  = usb2lin06::getHeight(report);

  cout<<hex<<setfill('0')
    <<" header:"<<setw(4)<<(int)report.header
    <<" u1:"<<setw(4)<<(int)report.unknown1
    <<" H:"<<setw(2)<<(int)report.height[0]<<setw(2)<<(int)report.height[1]
    <<" moveDir:"<<setw(2)<<(int)report.moveDir
    <<" mi:"<<setw(2)<<(int)report.moveIndicator
    <<" u3:"<<setw(2)<<(int)report.unknown3[0]<<setw(2)<<(int)report.unknown3[1]
    <<" u5:"<<setw(2)<<(int)report.unknown5[0]<<setw(2)<<(int)report.unknown5[1]<<setw(2)<<(int)report.unknown5[2]
    <<" key:"<<setw(4)<<(int)report.key
    <<" u8:"<<setw(2)<<(int)report.unknown8
  <<"\t";

  switch(report.key)
  {
    case 0xffff: cout<<"--"; break;
    case 0x0047: cout<<"B1"; break;
    case 0x0046: cout<<"B2"; break;
    case 0x000e: cout<<"B3"; break;
    case 0x000f: cout<<"B4"; break;
    case 0x000c: cout<<"B5"; break;
    case 0x000d: cout<<"B6"; break;
    default:     cout<<"??"; break;
  }

/*
  switch(report.moveDir)
  {
    case 0xf0:
    case 0xe0: cout<<" DOWN "; break;
    case 0x10: cout<<"  UP  "; break;
    case 0x00: cout<<" STOP "; break;
  }
*/
  cout<<std::dec<<" height: "<<dec<<setprecision(2)<<fixed<<setw(5)<<height;

  cout<<endl;

  return;
}

/*
 * just an ugly way to get time in format HH:MM:SS:MS
 */
std::string getPreciseTime()
{
  std::ostringstream stm ;

  std::time_t t = std::time(NULL);
  timeval curTime;
  gettimeofday(&curTime, NULL);
  int milli = curTime.tv_usec / 1000;

  char mbstr[100]; mbstr[0]='\0';
  if (std::strftime(mbstr, sizeof(mbstr), "%0H:%0M:%0S", std::localtime(&t)))
  {
    stm << mbstr <<":"<<setfill('0')<<setw(3)<<milli;
  }

  return stm.str() ;
}

#define LIBUSB_DEFAULT_TIMEOUT 1000
int main (int argc,char **argv)
{
  libusb_device_handle* udev = NULL;
  unsigned char buf[256];
  int ret=-1;

  int SETTINGS_COUNT = 1000;//how may times should i get status
  unsigned int SETTINGS_CommadDelay = 1000000;//delay between sending commands [in mikro Sec]

  bool showInfo=true;

  //get args
  {
    showInfo=(argc==1);

    if(argc>1)
    {
      SETTINGS_COUNT=::atoi(argv[1]);

      if(SETTINGS_COUNT<0 || SETTINGS_COUNT > 99999)
      {
        fprintf(stderr, "Error arg1 - remetitions must be an integer in range <0,99999>\n");
        return -1;
      }
    }

    if(argc==3)
    {
      float delaySec = -9.9f;
      delaySec=::atof(argv[2]);

      if(delaySec<0.1f and delaySec>10.0f)
      {
        fprintf(stderr, "Error arg2 - time [sec] must get greater than 0.1 and less than 10 sec seconds\n");
        return -1;
      }

      SETTINGS_CommadDelay = 1000000*delaySec;
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
    if(showInfo)    printf("Lets look for the Linak device...\n");

    udev = usb2lin06::openDevice();
    if(udev == NULL )
    {
      fprintf(stderr, "Error NO device");
      return 1;
    }
    if(showInfo)    printf("INFO: found device\n");
  }

  //print some device info
  {
    if(showInfo)    printDescriptor(udev);
  }

  //read some data
  if(showInfo)
  {
    ret = libusb_get_string_descriptor_ascii(udev, 1, buf, sizeof(buf));
    if(ret<0)  { fprintf(stderr,"Error to read string1 err%d \n",ret);}
    else       { cout<<"line 1: '"<<buf<<"'"<<endl; }

    ret = libusb_get_string_descriptor_ascii(udev, 2, buf, sizeof(buf));
    if(ret<0)  { fprintf(stderr,"Error to read string2  err%d \n",ret);}
    else       { cout<<"line 2: '"<<buf<<"'"<<endl; }
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

  //getting status SETTINGS_COUNT times
  {
    usb2lin06::statusReport report;
    std::string sCOUNT =  std::to_string(SETTINGS_COUNT);

    unsigned int i=1;   while(true)
    {
      cout<<getPreciseTime()<<" ["<<setfill('0')<<setw(sCOUNT.length())<<i<<"/"<<sCOUNT<<"] ";

      if(usb2lin06::getStatus(udev,report))
      { printStatusReport(report); }
      else
      { cout<<" Error "<<endl; }

      if(i++==SETTINGS_COUNT) break;
      usleep(SETTINGS_CommadDelay);
    }
  }

  //cleanup
  {
    libusb_close(udev);
    libusb_exit(0);
  }
  return 0;
}
