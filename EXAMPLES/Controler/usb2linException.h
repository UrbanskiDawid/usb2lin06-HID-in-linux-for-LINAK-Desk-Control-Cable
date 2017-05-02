#include <exception>
#include <string>
#include <libusb-1.0/libusb.h>

namespace usb2lin06
{
enum RETURN_CODES
{
    OK=0,
    //startup
    ARGS_MISSING=100,
    ARGS_WRONG,
    //device
    DEVICE_CANT_FIND=200,
    DEVICE_CANT_OPEN,
    DEVICE_CANT_INIT,
    //
    MESSAGE_ERROR=300
};

std::string errorToStr(int errID);

class exception: public std::exception {
    std::string m_msg;
    int m_error;
  public:
    exception(const int &p_errorCode,const std::string& p_msg="");
    const char *what() const throw();
    const int getErrorCode();
};
}