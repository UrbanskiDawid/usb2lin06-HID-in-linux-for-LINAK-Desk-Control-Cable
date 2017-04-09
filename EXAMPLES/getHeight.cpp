#include <iostream>
#include <iomanip>      // std::setw
#include "usb2lin06Controler.h"

using namespace std;

using usb2lin06::usb2lin06Controler;
using usb2lin06::isStatusReportNotReady;

/*
 * get current height reported by device status
 */
bool getCurrentHeight(usb2lin06Controler &controler,float &h)
{
  h=-1.0f;

  if(controler.getStatusReport())
  {
     if(!isStatusReportNotReady(controler.report))
     {
        h=controler.getHeightInCM();
        return true;
     }
  }

  return false;
}

int main (int argc,char **argv)
{
  DEBUGOUT("main() - init");
  usb2lin06Controler controler;

  DEBUGOUT("main() - getHeigh");
  {
    float curHeight = 1.0f;
    if(!getCurrentHeight(controler,curHeight))
    {
      cerr<<"ERROR: getStatus"<<endl;
    }else{
      cout<<"current height: "<<setprecision(1)<<dec<<fixed<<curHeight<<"cm"<<endl;
    }
  }

  DEBUGOUT("main() - end");
  {
    cout<<"DONE"<<endl;
  }
  return 0;
}
