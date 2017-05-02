#pragma once

#include <cstdint>
#include "usb2lin06.h"

namespace usb2lin06
{
using StatusReportBase = usb2lin06::StatusReport;

struct statusReport : StatusReportBase//NOTE: this is a workaround struct from usb2lin06.h cannot have functions due to usage in kernel
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

  void print() const;
};
static_assert(sizeof(statusReport)==StatusReportSize,"wrong size of StatusReport");
}//usb2lin06