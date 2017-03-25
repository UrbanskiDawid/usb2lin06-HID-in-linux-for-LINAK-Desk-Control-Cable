/* usb2lin06
 *
 * Autor: Dawid Urbanski
 * email: git@dawidurbanski.pl
 * Dawid: 25 marzec 2017
 *
 */
#ifndef usb2lin06_h
#define usb2lin06_h 1

#define VENDOR  0x12d3
#define PRODUCT 0x0002

struct LINIDvalidFlag
{
  bool ID00_Ref1_pos_stat_speed:1;
  bool ID01_Ref2_pos_stat_speed:1;
  bool ID02_Ref3_pos_stat_speed:1;
  bool ID03_Ref4_pos_stat_speed:1;

  bool ID10_Ref1_controlInput:1;
  bool ID11_Ref2_controlInput:1;
  bool ID12_Ref3_controlInput:1;
  bool ID13_Ref4_controlInput:1;

  bool ID04_Ref5_pos_stat_speed:1;

  bool ID28_Diagnostic:1;

  bool ID05_Ref6_pos_stat_speed:1;

  bool ID37_Handset1command:1;
  bool ID38_Handset2command:1;

  bool ID06_Ref7_pos_stat_speed:1;
  bool ID07_Ref8_pos_stat_speed:1;

  bool unknown1:1;
};
static_assert(sizeof(LINIDvalidFlag)==sizeof(uint16_t),"wrong size of LINIDvalidFlag (must be 16bit)");

typedef int16_t RefControlInput;

struct RefPosStatSpeed
{
  RefControlInput pos;   //HEIGHT low,high 0x00 0x00 <<bottom
  uint8_t         status;//0x00 <<stop,0xe0 <<going down,0x10<< going up, 0xf0 starting/ending going down
  uint8_t         speed; //if != 0 them moving;
};

#define StatusReport_ID 0x4
#define StatusReport_nrOfBytes 0x38
#define StatusReportSize 64
struct statusReport
{
  uint8_t header;           //[ 0 ] 0x04(CurrentStatusReport),
  uint8_t numberOfBytes;    //[ 1 ] 0x38(CurrentStatusReport)
  LINIDvalidFlag validFlag; //[ 2, 3] 0x1108 << after movment (few seconds), 0x0000 afterwards, 0x0108 << while moving
  RefPosStatSpeed ref1;     //[ 4, 5, 6, 7] pos,status,speed
  RefPosStatSpeed ref2;     //[ 8, 9,10,11] pos,status,speed
  RefPosStatSpeed ref3;     //[12,13,14,15] pos,status,speed
  RefPosStatSpeed ref4;     //[16,17,18,19] pos,status,speed
  RefControlInput ref1cnt;  //[20,21] target height
  RefControlInput ref2cnt;  //[22,23] target height
  RefControlInput ref3cnt;  //[24,25] target height
  RefControlInput ref4cnt;  //[26,27] target height
  RefPosStatSpeed ref5;     //[28,29,30,31] pos,status,speed
  uint8_t diagnostic[8];    //[32,33,34,35,36,37,38,39]
  uint8_t undefined1[2];    //[40,41]
  uint16_t handset1;        //[42,43] buttons
  uint16_t handset2;        //[44,45] buttons
  RefPosStatSpeed ref6;     //[46-49] pos,status,speed
  RefPosStatSpeed ref7;     //[50-53] pos,status,speed
  RefPosStatSpeed ref8;     //[54-57] pos,status,speed
  uint8_t undefined2[6];    //[58-63]
};
static_assert(sizeof(statusReport)==StatusReportSize,"wrong size of statusReport");

struct sCtrlURB
{
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
};

#define USB2LIN_modeOfOperation_featureReportID 3
#define USB2LIN_ModeOfOperation_default 4

#define URB_wValue_Init      0x0303
#define URB_wValue_GetStatus 0x0304
#define URB_wValue_Move      0x0305


const struct sCtrlURB URB_init      = { 0x21, 9/*HID_REPORT_SET*/, URB_wValue_Init,      0, StatusReportSize};
const struct sCtrlURB URB_getStatus = { 0xA1, 1/*HID_REPORT_GET*/, URB_wValue_GetStatus, 0, StatusReportSize};
const struct sCtrlURB URB_move      = { 0x21, 9/*HID_REPORT_SET*/, URB_wValue_Move,      0, StatusReportSize};

#define DefaultUSBtimeoutMS 1000

void printLibStrErr(int errID);

//height - is a 16 signed integer with the height in 1/10 mm with 0 as lowest height of actuators together with 8 bit status information
//TODO: veryfy type: signed/unsigned
//TODO: ?8 bit status information?
#define HEIGHT_moveDownwards 0x7fff
#define HEIGHT_moveUpwards   0x8000
#define HEIGHT_moveEnd       0x8001
//------

// PUBLIC INTERFACE
//================================================================================
struct libusb_device_handle *openDevice(bool initialization=true);
bool initDevice(libusb_device_handle* udev);

bool getStatusReport(libusb_device_handle* udev, statusReport &report, int timeout=DefaultUSBtimeoutMS);
bool isStatusReportNotReady(const statusReport &report);
int   getHeight(const statusReport &report);
float getHeightInCM(const statusReport &report);

bool move    (libusb_device_handle * udev, int16_t targetHeight, int timeout=DefaultUSBtimeoutMS);
bool moveDown(libusb_device_handle * udev, int timeout=DefaultUSBtimeoutMS);
bool moveUp  (libusb_device_handle * udev, int timeout=DefaultUSBtimeoutMS);
bool moveEnd (libusb_device_handle * udev, int timeout=DefaultUSBtimeoutMS);
//================================================================================


#endif
