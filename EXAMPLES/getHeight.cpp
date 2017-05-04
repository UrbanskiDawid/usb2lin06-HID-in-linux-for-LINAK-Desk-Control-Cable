#include <iostream>
#include <iomanip>      // std::setw
#include "usb2lin06Controler.h"

using namespace std;
using namespace usb2lin06::controler;
using usb2lin06::RefControlInput;

/*
 * get current height reported by device status
 */
bool getCurrentHeight(
  usb2lin06Controler &controler,
  RefControlInput &hRaw,
  float &hCM)
{
  hCM=-1.0f;
  hRaw=-1;

  if(!controler.getStatusReport()->isStatusReportNotReady())
  {
      hRaw=controler.getHeight();
      hCM=controler.getHeightInCM();
      return true;
  }

  return false;
}

int main (int argc,char **argv)
{
  try{
  DEBUGOUT("main() - init");
  usb2lin06Controler controler;

  DEBUGOUT("main() - getHeigh");
  {
    RefControlInput curHeight_Raw = 0;
    float curHeight_CM = 1.0f;
    if(!getCurrentHeight(controler,curHeight_Raw,curHeight_CM))
    {
      cerr<<"ERROR: getStatus"<<endl;
    }else{
      cout<<"current height: "
        <<curHeight_Raw
        <<" "<<setprecision(1)<<dec<<fixed<<curHeight_CM<<"cm"
        <<endl;
    }
  }

  DEBUGOUT("main() - end");
  {
    cout<<"DONE"<<endl;
  }

  }catch(usb2lin06::controler::exception e){
    cerr<<"Error: "<<" "<<e.what()<<endl;
    return e.getErrorCode();
  }
  return 0;
}
