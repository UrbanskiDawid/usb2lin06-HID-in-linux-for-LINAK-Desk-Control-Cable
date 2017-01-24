/* usb2lin06
 *
 * Autor: Dawid Urbanski
 * email: git@dawidurbanski.pl
 * Dawid: 2 october 2015
 *
 */

#define VENDOR  0x12d3
#define PRODUCT 0x0002

struct statusReport//size:64B
{
  uint16_t header;       //[ 0, 1] 0x04, 0x38 constant
  uint16_t unknown1;     //[ 2, 3] 0x1108 << after movment (few seconds), 0x0000 afterwards, 0x0108 << while moving
  int16_t  height;       //[ 4, 5] low,high 0x00 0x00 <<bottom
  uint8_t  moveDir;      //[  6  ] 0xe0 <<going down,0x10<< going up, 0xf0 starting/ending going down
  uint8_t  moveIndicator;//[  7  ] if != 0 them moving;
  uint8_t  unknown2[12]; //[ 8-19] zero ??
  int16_t  targetHeight; //[20,21] 0x01 0x80 < if stopped
  uint8_t  unknown4[10]; //[22-31] zero ??
  uint8_t  unknown5[3];  //[32,34] 0x01 0x00 0x36 ??
  uint8_t  unknown6[7];  //[35,41] zero ??
  uint16_t key;          //[42,43] button pressed down
  uint8_t  unknown7[14]; //[44-57]no work was done
  uint8_t  unknown8;     //[  58 ] 0x10/0x08 ??
  uint8_t  unknown9[4];  //[59-63] zero ??
};

struct sCtrlURB
{
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
};

#define URB_wValue_Init      0x0303
#define URB_wValue_GetStatus 0x0304
#define URB_wValue_Move      0x0305


const struct sCtrlURB URB_init      = { 0x21, 9/*HID_REPORT_SET*/, URB_wValue_Init,      0, 64};
const struct sCtrlURB URB_getStatus = { 0xA1, 1/*HID_REPORT_GET*/, URB_wValue_GetStatus, 0, 64};
const struct sCtrlURB URB_move      = { 0x21, 9/*HID_REPORT_SET*/, URB_wValue_Move,      0, 64};

