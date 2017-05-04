#pragma once

#include <cstdint>
#include "usb2lin06.h"

namespace usb2lin06 {
using StatusReportBase = usb2lin06::StatusReport;
namespace controler {

struct statusReport : StatusReportBase
{
 /*
  * height is a 16 signed integer with the height in 1/10 mm with 0 as lowest height of actuators
  * offsetCM - bottom position of you desk
  */
  float getHeightCM(float offsetCM=0.0f) const;

 /*
  * if status report is:
  * 0x04380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
  * the device is not ready!
  */
  bool isStatusReportNotReady() const;

 /*
  * put this object to std out
  */
  void print() const;
};
static_assert(sizeof(statusReport)==StatusReportSize,"wrong size of StatusReport");

}//controler

}//usb2lin06