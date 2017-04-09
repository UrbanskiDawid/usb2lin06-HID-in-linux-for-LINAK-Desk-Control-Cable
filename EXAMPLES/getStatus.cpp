#include <iostream>
#include <vector>
#include <string.h>     // std::string, memcpy
#include <iomanip>      // std::setw
#include <unistd.h>  //usleep
#include <sstream>  //std::ostringstream

#include "usb2lin06Controler.h"

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

int main (int argc,char **argv)
{
  int SETTINGS_COUNT = 1000;//how may times should i get status
  unsigned int SETTINGS_CommadDelay = 1000000;//delay between sending commands [in micro Sec]

  DEBUGOUT("main() - get args");
  {
    if(argc>1)
    {
      SETTINGS_COUNT=::atoi(argv[1]);

      if(SETTINGS_COUNT<0 || SETTINGS_COUNT > 99999)
      {
        cerr<<"ERROR: arg1 - remetitions must be an integer in range <0,99999>"<<endl;
        return -1;
      }
    }

    if(argc==3)
    {
      float delaySec = -9.9f;
      delaySec=::atof(argv[2]);

      if(delaySec<0.1f and delaySec>10.0f)
      {
        cerr<<"ERROR: arg2 - time [sec] must get greater than 0.1 and less than 10 sec seconds"<<endl;
        return -1;
      }

      SETTINGS_CommadDelay = 1000000*delaySec;
    }
  }

  DEBUGOUT("main() - init");
  usb2lin06::usb2lin06Controler controler(false);

  DEBUGOUT("main() - print some device info");
  {
    printDescriptor(controler.udev);

    unsigned char buf[50];
    for(int i : {1,2})
    {
      int ret = libusb_get_string_descriptor_ascii(controler.udev, i, buf, sizeof(buf));
      if(ret<0)  { cerr<<"ERROR: to read device decriptor string"<<i<<" err"<<ret<<endl;}
      else       { cout<<"line "<<i<<": '"<<buf<<"'"<<endl; }
    }
  }

  DEBUGOUT("main() - check if device it ready");
  {
    if(!controler.getStatusReport())
    {
      cerr<<"ERROR: cant get initial status"<<endl;
      return 1;
    }else{
      if(controler.report.isStatusReportNotReady()){
        cout<<" device is not ready"<<endl;
        if(!controler.initDevice())
        {
          cerr<<"ERROR: can't init device!"<<endl;
          return 1;
        }
      }else{
        cout<<" device is ready"<<endl;
      }
    }
  }

  DEBUGOUT("main() - getting status SETTINGS_COUNT times");
  {
    std::string sCOUNT = std::to_string(SETTINGS_COUNT);

    unsigned int i=1;
    while(true)
    {
      if(controler.getStatusReport())
      {      
          cout<<getPreciseTime()<<dec<<" ["<<setfill('0')<<setw(sCOUNT.length())<<i<<"/"<<sCOUNT<<"] ";
          controler.report.print();
      }

      if(i++==SETTINGS_COUNT) break;
      usleep(SETTINGS_CommadDelay);
    }
  }

  DEBUGOUT("main() - end");
  {
    cout<<"DONE"<<endl;
  }
  return 0;
}
