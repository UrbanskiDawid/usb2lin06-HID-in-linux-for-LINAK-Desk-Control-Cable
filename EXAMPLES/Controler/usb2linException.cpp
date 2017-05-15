#include <iostream>
#include "usb2linException.h"

namespace usb2lin06 {
namespace controler {
std::string errorToStr(int errID)
{
  switch(errID)//libusb errors
  {
    case LIBUSB_SUCCESS:         return "OK";
    case LIBUSB_ERROR_TIMEOUT:   return "the transfer timed out (and populates transferred)";
    case LIBUSB_ERROR_PIPE:      return "the endpoint halted";
    case LIBUSB_ERROR_OVERFLOW:  return "the device offered more data, see Packets and overflows";
    case LIBUSB_ERROR_NO_DEVICE: return "the device has been disconnected";
  }
  if(errID<=0){
    return std::string("another LIBUSB_ERROR code on other failures")+std::to_string(errID);
  }

  switch(errID)//usb2lin errors
  {
    case ARGS_MISSING: return "missing arguments for program";
    case ARGS_WRONG:   return "program arguments are wrong";
    
    case DEVICE_CANT_FIND: return "can't find device";
    case DEVICE_CANT_OPEN: return "can't open device";
    case DEVICE_CANT_INIT: return "can't init device";

    case MESSAGE_ERROR:    return "message from device has errors";
  }
 
  return std::string("unknown error: '")+std::to_string(errID)+"'";
}

exception::exception(const int &p_errorCode,const std::string& p_msg):
  m_msg(errorToStr(p_errorCode)),
  m_error(p_errorCode),
  std::exception()
{
  if(p_msg!="") m_msg+=" "+p_msg;
}

exception::exception(const int &p_errorCode,const std::ostringstream & p_msg):
  m_msg(errorToStr(p_errorCode)),
  m_error(p_errorCode),
  std::exception()
{
  const std::string s = p_msg.str();
  if(s!="") m_msg+=" "+s;
}


const char *exception::what() const throw() { return this->m_msg.c_str(); };

const int exception::getErrorCode() { return m_error; }
}//namespace controler
}