#include <iostream>
#include <vector>
#include <string.h>     // std::string, memcpy
#include <iomanip>      // std::setw
#include <unistd.h>  //usleep
#include <sstream>  //std::ostringstream
#include <math.h>
#include "usb2lin06Controler.h"

using namespace std;

/*
 * WARNING: dangerous
 * this will move towards goal
 * TODO: stop if cannot move
 */
bool moveTo(usb2lin06::usb2lin06Controler & controler, uint16_t target)
{
  const usb2lin06::StatusReportEx &r = controler.report;

  const unsigned int max_a = 3; unsigned int a = max_a;//stuck protection
  uint16_t oldH=0;  

  const int epsilon = 13;//this seems to be move prcision

  while(true)
  {
    DEBUGOUT("moveTo()",target)
    controler.move(target);

    usleep(200000);

    if( controler.getStatusReport() )
    {
      double distance = r.ref1cnt-r.ref1.pos;
      double delta    = oldH-r.ref1.pos;

      if(fabs(distance)<=epsilon | fabs(delta) <=epsilon | oldH==r.ref1.pos)
        a--;
      else
        a=max_a;

      cout
        <<"current height: "<<dec<<setw(5)<<setfill(' ')<<r.ref1.pos
        <<" target height: "<<dec<<setw(5)<<setfill(' ')<<target
        <<" distance:"<<dec<<setw(5)<<setfill(' ')<<distance
      <<endl;

      if(a==0) {break;}
      oldH=r.ref1.pos;
    }
  }//while

  return ( fabs(r.ref1.pos-target) <= epsilon);
}

void printHelp()
{
  cout<<"this will set height of your desk using usb2lin06"<<endl
      <<"WARNING: this might be dangerous please make sure that you dont hit something!"<<endl
      <<"please start program with: arg1 (height)"<<endl;
}

int main (int argc,char **argv)
{
  bool succes=false;

  int16_t targetHeight=-1;//target height to move, form arg1
  
  DEBUGOUT("main() - start");
  {
    long long int tmp= atoll(argv[1]);
    targetHeight=tmp;

    if(argc!=2 || targetHeight<0 || tmp > (long long int)targetHeight)
    {
      printHelp();
      return -1;
    }
  }

  DEBUGOUT("main() - init");
  usb2lin06::usb2lin06Controler controler;

  DEBUGOUT("main() - move to targetHeight",targetHeight);
  {
    if( moveTo(controler,targetHeight) )
    {
      cout<<"success"<<endl;
      succes=true;
    }
    else
      cout<<"failed"<<endl;
  }

  DEBUGOUT("main() - end");
  {
    cout<<"DONE"<<endl;
  }
  return (succes ? 0 : -1);
}
