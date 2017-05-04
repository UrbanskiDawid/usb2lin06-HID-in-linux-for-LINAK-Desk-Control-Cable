#include "statusReport.h"
#include <iostream>
#include <iomanip>

namespace usb2lin06 {
namespace controler {

using namespace std;

bool statusReport::isStatusReportNotReady() const
{
  const unsigned char *report_bytes=reinterpret_cast<const unsigned char*>(this); 
  if(report_bytes[0] != StatusReport_ID
     ||
     report_bytes[1] != StatusReport_nrOfBytes) {
    return false; //THIS IS NOT A valid status report!
  }

  for(int i=2;i<StatusReportSize-6;i++)/*check bits: 2..58*/
  {
    if(report_bytes[i]!=0) return false;
  }

  return true;
}

float statusReport::getHeightCM(float offsetCM) const
{
  return offsetCM+ref1.pos*0.01f;
}

const char SeparatorFields=',';
const char SeparatorStart='{';
const char SeparatorEnd='}';

ostream& operator << (ostream &o, const usb2lin06::LINIDvalidFlag &a)
{
  o<<"flags:" <<SeparatorStart
  <<dec<<"0b"
  //note: struct reversed
  <<(int)a.unknown
  <<(int)a.ID07_Ref8_pos_stat_speed
  <<(int)a.ID06_Ref7_pos_stat_speed
  <<(int)a.ID38_Handset2command
  <<(int)a.ID37_Handset1command
  <<(int)a.ID05_Ref6_pos_stat_speed
  <<(int)a.ID28_Diagnostic
  <<(int)a.ID04_Ref5_pos_stat_speed
  <<(int)a.ID13_Ref4_controlInput
  <<(int)a.ID12_Ref3_controlInput
  <<(int)a.ID11_Ref2_controlInput
  <<(int)a.ID10_Ref1_controlInput
  <<(int)a.ID03_Ref4_pos_stat_speed
  <<(int)a.ID02_Ref3_pos_stat_speed
  <<(int)a.ID01_Ref2_pos_stat_speed
  <<(int)a.ID00_Ref1_pos_stat_speed
  <<SeparatorEnd;
}

ostream& operator << (ostream &o, const usb2lin06::Status &a)
{
  o<<"status:"<<SeparatorStart
  <<dec
  <<(bool)a.positionLost << (bool)a.antiColision << (bool)a.overloadDown << (bool)a.overloadUp
  <<SeparatorEnd;
}

ostream& operator << (ostream &o, const usb2lin06::RefPosStatSpeed &a)
{
  o<<"posStatSpeed:" <<SeparatorStart
  <<hex<<setfill('0')
  <<hex<<"0x"<<setw(4)<<(short)a.pos   <<SeparatorFields
  <<                           a.status<<SeparatorFields
  <<hex<<"0x"<<setw(4)<<(short)a.speed <<SeparatorEnd;
}

ostream& operator << (ostream &o, const usb2lin06::Diagnostic &a)
{
  o<<"diag:"   <<SeparatorStart
  <<hex<<"0x"<<setw(4)<<(short)a.type     <<SeparatorFields
  <<dec
  <<(int)a.event[0] <<SeparatorFields
  <<(int)a.event[1] <<SeparatorFields
  <<(int)a.event[2] <<SeparatorFields
  <<(int)a.event[3] <<SeparatorFields
  <<(int)a.event[4] <<SeparatorFields
  <<(int)a.event[5] <<SeparatorEnd;
}

ostream& operator << (ostream &o, const statusReport &a) {

  auto handsetToStr = [](const uint16_t &h)-> const char*
  {
    switch(h)
    {
    case 0xffff: return "--";
    case 0x0047: return "B1";
    case 0x0046: return "B2";
    case 0x000e: return "B3";
    case 0x000f: return "B4";
    case 0x000c: return "B5";
    case 0x000d: return "B6";
    }
    return "??";
  };

  o<<"StatusReport:"<<SeparatorStart
  <<hex<<setfill('0')
//  <<"ID:"  << SeparatorStart <<setw(2)<<(int)a.featureRaportID <<SeparatorEnd<<SeparatorFields
//  <<"bytes:"<<SeparatorStart <<setw(2)<<(int)a.numberOfBytes   <<SeparatorEnd <<SeparatorFields
  <<a.validFlag;
  if(a.validFlag.ID00_Ref1_pos_stat_speed) {o<<SeparatorFields<<"ref1"<< a.ref1<< " HEIGHT:"<<a.getHeightCM()<<"cm";}
  if(a.validFlag.ID01_Ref2_pos_stat_speed) o<<SeparatorFields<<"ref2"<< a.ref2;
  if(a.validFlag.ID02_Ref3_pos_stat_speed) o<<SeparatorFields<<"ref3"<< a.ref3;
  if(a.validFlag.ID03_Ref4_pos_stat_speed) o<<SeparatorFields<<"ref4"<< a.ref4;
  if(a.validFlag.ID04_Ref5_pos_stat_speed) o<<SeparatorFields<<"ref5"<< a.ref5;
  if(a.validFlag.ID05_Ref6_pos_stat_speed) o<<SeparatorFields<<"ref6"<< a.ref6;
  if(a.validFlag.ID06_Ref7_pos_stat_speed) o<<SeparatorFields<<"ref7"<< a.ref7;
  if(a.validFlag.ID07_Ref8_pos_stat_speed) o<<SeparatorFields<<"ref8"<< a.ref8;
  if(a.validFlag.ID10_Ref1_controlInput)   o<<SeparatorFields<<"ref1ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref1cnt)  <<SeparatorEnd;
  if(a.validFlag.ID11_Ref2_controlInput)   o<<SeparatorFields<<"ref2ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref2cnt)  <<SeparatorEnd;
  if(a.validFlag.ID12_Ref3_controlInput)   o<<SeparatorFields<<"ref3ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref3cnt)  <<SeparatorEnd;
  if(a.validFlag.ID13_Ref4_controlInput)   o<<SeparatorFields<<"ref4ctr:"<<SeparatorStart<<setw(4)<< (short)(a.ref4cnt)  <<SeparatorEnd;
  if(a.validFlag.ID28_Diagnostic)  o<<a.diagnostic <<SeparatorFields;
//  <<"undefined1:"   <<SeparatorStart <<(int)a.undefined1[0] <<(int)a.undefined1[1] <<SeparatorEnd  <<SeparatorFields
  if(a.validFlag.ID37_Handset1command) o<<SeparatorFields<<"handset1:"<< SeparatorStart << handsetToStr(a.handset1) <<SeparatorEnd;
  if(a.validFlag.ID38_Handset2command) o<<SeparatorFields<<"handset2:"<< SeparatorStart << handsetToStr(a.handset2) <<SeparatorEnd;
//  <<"undefined2:"   <<SeparatorStart <<(int)a.undefined2[0] <<(int)a.undefined2[1] <<(int)a.undefined2[2] <<(int)a.undefined2[3] <<(int)a.undefined2[4] <<(int)a.undefined2[5] <<SeparatorEnd

  o<<SeparatorEnd;
}

void statusReport::print() const
{
  if(isStatusReportNotReady())
    cerr<<"ERROR: statusReport -> device not ready"<<endl;
  else
    cout<<*this<<endl;
}
}//namespace controler
}//namespace