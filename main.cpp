#include <stdio.h>
#include <iostream>
//#include <usb.h> //apt-get install libusb-dev
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <vector>
#include <string>       // std::string
#include <iomanip>      // std::setw
#include <utility>      // std::pair, std::make_pair
#include <unistd.h>
#include <bitset>
#include <sys/types.h>
#include <string.h> //memcpy
#include <sstream>


#include <ctime>
#include <iostream>
#include <locale>

//some sources
//http://matthias.vallentin.net/blog/2007/04/writing-a-linux-kernel-driver-for-an-unknown-usb-device/
//from: http://www.beyondlogic.org/usbnutshell/usb5.shtml
//http://www.harshbutfair.org/software/example.c

//lubusb-1.0
// http://libusb.sourceforge.net/api-1.0/
//CONTROL URB https://www.safaribooksonline.com/library/view/linux-device-drivers/0596005903/ch13.html

using namespace std;


struct statusReport//size:64B
{
  uint16_t header;       //[ 0, 1] 0x04, 0x38, 0x11  [End of Transmission, 8, Device Control 1 (oft. XON)]
  uint16_t unknown1;     //[ 2, 3] 0x11,0x08 << after movment (few seconds), 0x00,0x00 afterwards, 0x01,0x08 << while moving
  uint8_t  height[2];    //[ 4, 5] low,high 0x00 0x00 <<bottom
  uint8_t  moveDir;      //[  6  ] 0xe0 <<going down,0x10<< going up, 0xf0 starting going down
  uint8_t  moveIndicator;//[  7  ] if != 0 them moving;
  uint8_t  unknown2[12]; //[ 8-19] zero ??
  uint8_t  unknown3[2];  //[20,21] 0x01 0x80 ??
  uint8_t  unknown4[10]; //[22-31] zero ??
  uint8_t  unknown5[3];  //[32,34] 0x01 0x00 0x36 ??
  uint8_t  unknown6[7];  //[35,41] zero ??
  uint16_t key;          //[42,43] button pressed down
  uint8_t  unknown7[14]; //[44-57]no work was done
  uint8_t  unknown8;     //[  58 ] 0x10/0x08 ??
  uint8_t  unknown9[4];  //[59-63] zero ??
};


/*
[ 6725.772231] usb 1-1.2: new full-speed USB device number 6 using ehci-pci
[ 6725.867477] usb 1-1.2: New USB device found, idVendor=12d3, idProduct=0002
[ 6725.867482] usb 1-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[ 6725.867484] usb 1-1.2: Product: USB Control Link
[ 6725.867486] usb 1-1.2: Manufacturer: Linak DeskLine A/S
[ 6725.867488] usb 1-1.2: SerialNumber: Љ
[ 6725.875451] hid-generic 0003:12D3:0002.0008: hiddev0,hidraw0: USB HID v1.11 Device [Linak DeskLine A/S USB Control Link] on usb-0000:00:1a.0-1.2/input0
*/
static struct libusb_device_handle *find_usb2lin06()
{
  const uint16_t
    vendor = 0x12d3,
    product = 0x0002;
  return libusb_open_device_with_vid_pid(0, vendor,product);
}


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
Get currnet status from device

URB_CONTROL IN
URB_CONTROL,URB_LEN=64,endpoint 0x80,IN

1. URB SUBMIT   ,DATA_LEN=0
host -> device

"Each request starts with a 8 byte long Setup Packet"
bmRequestType
  D7 Data Phase Transfer Direction
  1 = Device to Host
  D6..5 Type
  01 = Class
  D4..0 Recipient
  00001 = Interface
(bmRequestType)  (bRequest)  (wValue 16b)   (wIndex) (wLength)
(1010 0001)      (0000 0001) (0x0304    )   (0)      (64)

2. URB COMPLEATE,DATA_LEN=64
device -> host
control responce data
 00000000 00000100 00111000 00000000 00000000 00000000 00000000 00000000 00000000 .8...... ???  <<error heiht ??
 00000000 00000100 00111000 00010001 00001000 00000000 00000000 00000000 00000000 .8...... _min
 00000000 00000100 00111000 00010001 00001000 00011010 00000000 00000000 00000000 .8...... _min2
 00000000 00000100 00111000 00010001 00001000 00101111 00000000 00000000 00000000 .8../... _min3
 00000000 00000100 00111000 00010001 01000001 00101111 00000000 00000000 00000000 (simulated++)
                                    (-----------------) < wysokosc
                                     00000000
                                     00000001
                                     00000002
                                     00000000 00000003 etc
                                       10*0    10*1      10*3    (jade!)    10*5
 (-------------------------)const
 00000000 00000100 00111000 00010001 00001000 01101000 00000000 00000000 00000000 .8..h... min0
 00000000 00000100 00111000 00010001 00001000 01000001 00000010 00000000 00000000 .8..A...
 00000000 00000100 00111000 00010001 00001000 00000101 00000100 00000000 00000000 .8......
 00000000 00000100 00111000 00010001 00001000 10101001 00001000 00000000 00000000 .8......
 00000000 00000100 00111000 00010001 00001000 01010010 00011001 00000000 00000000 .8..R...
*/
bool linakGetStatus(libusb_device_handle* udev, statusReport &report)
{
  unsigned char buf[64]; //CONTROL responce data
  int ret = libusb_control_transfer(
     udev,
     0xa1,  //0b10100001 (mbRequest),
     1,     //request (bRequest)
     0x0304,//value (wValue)
     0,     //(wIndex)
     buf,   //bytes
     64,    //size (wLenght)
     3000   //timeout
     );
  if(ret<0)
  {
    fprintf(stderr,"fail to get status request err%d\n",ret);
    /* Broken pipe is EPIPE. That means the device sent a STALL to your control
    message. If you don't know what a STALL is, check the USB specs.

    While the meaning of STALL varies from device to device and the
    situation, it usually means what you sent was wrong.
    I'd double check the request, requesttype, value and index arguments.
    I'd also double check the title_request buffer.*/
    return false;
  }

  //Debug Print Binary
  //for(int i=0;i<64;i++) {    cout<<std::bitset<8>(buf[i])<<" "; }

  //Debug Print Hex
  //for(int i=0;i<64;i++) {    cout<<setw(2)<<setfill('0')<<std::hex<<(int)(unsigned char)buf[i]<< " ";} cout<<endl;

  memcpy(&report, buf, sizeof(report));
  return (report.header==0x3804);
}

void printStatusReport(const statusReport &report)
{
  bool  isMoving= (report.moveIndicator!=0);
  bool  isBottom= (report.height[0]==0 && report.height[1]==0);
  float height  = ((float)(int)report.height[0])/257 + (unsigned int)report.height[1];

  cout<<hex<<setfill('0')
    <<" header:"<<setw(4)<<(int)report.header
    <<" u1:"<<setw(4)<<(int)report.unknown1
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

    udev = find_usb2lin06();
    if(udev == NULL )
    {
      fprintf(stderr, "Error NO device");
      return 1;
    }
    printf("INFO: found device\n");
  }

  //print some device info
  {
    printDescriptor(udev);
  }

  //read some data
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

  //not 100% sure if this is needed
  {
    ret=libusb_control_transfer(udev, 0b00100001, 0x0a,0x0000,0,buf,0,LIBUSB_DEFAULT_TIMEOUT);
    if(ret<0)
    {
      fprintf(stderr,"Error to send control msg %d\n",ret);
    }else{
      buf[ret]='\0';
      cout<<"OK '"<<buf<<"'"<<ret<<endl;
    }
  }

  //not 100% sure if this is needed
  {
    int transferred = 0;
    ret= libusb_interrupt_transfer(udev, 0b10000001, buf, 127,&transferred,LIBUSB_DEFAULT_TIMEOUT);
    if(ret<0)
    {
      fprintf(stderr,"Error to send interrupt err%d\n",ret);
    }else{
      buf[ret]='\0';
      cout<<"OK '"<<buf<<"'"<<ret<<endl;
    }
  }

  //getting status 1000times
  {
    statusReport report;

    for(unsigned int i=0;i<1000;i++)
    {
      cout<<getPreciseTime()<<" ["<<setw(4)<<i<<"/"<<1000<<"] ";

      if(linakGetStatus(udev,report))
      { printStatusReport(report); }
      else
      { cout<<" Error "<<endl; }

      usleep(100000);
    }
  }

  //cleanup
  {
    libusb_close(udev);
    libusb_exit(0);
  }
  return 0;
}

