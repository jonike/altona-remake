/*+**************************************************************************/
/***                                                                      ***/
/***   This file is distributed under a BSD license.                      ***/
/***   See LICENSE.txt for details.                                       ***/
/***                                                                      ***/
/**************************************************************************+*/

#include "base/types.hpp"

#if sCONFIG_SYSTEM_WINDOWS

#include "base/devices.hpp"
#include "base/system.hpp"
#include <windows.h>

static sBool sMidiLog, sMidiOnlyPhys;

/****************************************************************************/
/****************************************************************************/

// there may be only one cosumer thread and one producer thread

template <class T,int max> class sLocklessQueue2
{
  T Data[max];
  volatile int Write;
  volatile int Read;
public:
  sLocklessQueue2()   { Write=0;Read=0;sVERIFY(sIsPower2(max)); }
  ~sLocklessQueue2()  {}

  // may be called by producer

  sBool IsFull()      { return Write >= Read+max; }
  void AddTail(const T &e)  { int i=Write; sVERIFY(i < Read+max); Data[i&(max-1)] = e; sWriteBarrier(); Write=i+1; }

  // may be called by consumer

  sBool IsEmpty()     { return Read >= Write; }
  sBool RemHead(T &e)        { int i=Read; if(i>=Write) return 0; e=Data[i&(max-1)]; sWriteBarrier(); Read=i+1; return 1; }

  // this is an approximation!

  int GetUsed()      { return Write-Read; }
};

/****************************************************************************/
/****************************************************************************/

struct sMidiIn
{
  HMIDIIN Handle;
  sString<128> Name;
};

struct sMidiOut
{
  HMIDIOUT Handle;
  sString<128> Name;
};

void CALLBACK sMidiInProc (HMIDIIN  in ,UINT msg,DWORD inst,DWORD p0,DWORD p1);
void CALLBACK sMidiOutProc(HMIDIOUT out,UINT msg,DWORD inst,DWORD p0,DWORD p1);

class sMidiHandlerWin : public sMidiHandler_
{
  friend void CALLBACK sMidiInProc (HMIDIIN  in ,UINT msg,DWORD inst,DWORD p0,DWORD p1);
  sArray<sMidiIn> In;
  sArray<sMidiOut> Out;
  sLocklessQueue2<sMidiEvent,1024> InQueue;
public:
  sMidiHandlerWin();
  ~sMidiHandlerWin();
  const sChar *GetDeviceName(sBool out,int dev);

  sBool HasInput();
  sBool GetInput(sMidiEvent &e);
  void Output(sMidiEvent &e);
  void Output(uint8_t dev,uint8_t chan,uint8_t msg,uint8_t val);
};

sMidiHandler_ *sMidiHandler;

/****************************************************************************/

void CALLBACK sMidiInProc(HMIDIIN in,UINT msg,DWORD inst,DWORD p0,DWORD p1)
{
  if(sMidiLog)
    sLogF(L"midi",L"in  %08x %08x %08x %08x %08x\n",sPtr(in),uint32_t(msg),uint32_t(inst),uint32_t(p0),uint32_t(p1));
  if(sMidiHandler)
  {
    sMidiHandlerWin *mh = (sMidiHandlerWin *) sMidiHandler;
    sMidiEvent ev;
    ev.TimeStamp = timeGetTime();
    ev.Device = inst;
    ev.Status = (p0>>0) & 0xff;
    ev.Value1 = (p0>>8) & 0xff;
    ev.Value2 = (p0>>16) & 0xff;
    if(!mh->InQueue.IsFull())
      mh->InQueue.AddTail(ev);
    if(mh->InputMsg.Target)
      mh->InputMsg.PostASync();
  }
}

void CALLBACK sMidiOutProc(HMIDIOUT out,UINT msg,DWORD inst,DWORD p0,DWORD p1)
{
  if(sMidiLog)
    sLogF(L"midi",L"out %08x %08x %08x %08x %08x\n",sPtr(out),uint32_t(msg),uint32_t(inst),uint32_t(p0),uint32_t(p1));
}


sMidiHandlerWin::sMidiHandlerWin()
{
  int max;
  sMidiIn *in;
  sMidiOut *out;
  sString<128> str;

  max = midiInGetNumDevs();
  In.HintSize(max);
  for(int i=0;i<max;i++)
  {
    HMIDIIN hnd;
    MIDIINCAPSW caps;
    int n = In.GetCount();
    MMRESULT r = midiInOpen(&hnd,i,(DWORD_PTR)sMidiInProc,n,CALLBACK_FUNCTION);
    if(r==MMSYSERR_NOERROR)
    {
      if(midiInGetDevCapsW(i,&caps,sizeof(caps))==MMSYSERR_NOERROR)
        str = caps.szPname;

      in = In.AddMany(1);
      str.PrintF(L"(%d)",n);
      in->Name = str;
      in->Handle = hnd;

      sLogF(L"midi",L"midi in %d = %q\n",n,str);

      midiInStart(hnd);
    }
  }

  max = midiOutGetNumDevs();
  Out.HintSize(max);
  for(int i=0;i<max;i++)
  {
    HMIDIOUT hnd;
    MIDIOUTCAPSW caps;
    int n = Out.GetCount();
    MMRESULT r = midiOutOpen(&hnd,i,(DWORD_PTR)sMidiOutProc,n,CALLBACK_FUNCTION);
    if(r==MMSYSERR_NOERROR)
    {
      if(midiOutGetDevCapsW(i,&caps,sizeof(caps))==MMSYSERR_NOERROR)
        str = caps.szPname;

      if (sMidiOnlyPhys && caps.wTechnology != MOD_MIDIPORT)
        continue;

      out = Out.AddMany(1);
      str.PrintF(L"(%d)",n);
      out->Name = str;
      out->Handle = hnd;

      sLogF(L"midi",L"midi out %d = %q\n",n,str);
    }
  }
}

const sChar *sMidiHandlerWin::GetDeviceName(sBool out,int dev)
{
  if(out)
  {
    if(dev>=0 && dev<Out.GetCount())
      return Out[dev].Name;
    else 
      return 0;
  }
  else
  {
    if(dev>=0 && dev<In.GetCount())
      return In[dev].Name;
    else 
      return 0;
  }
}

sMidiHandlerWin::~sMidiHandlerWin()
{
  sMidiIn *in;
  sMidiOut *out;

  sFORALL(In,in)
    midiInClose(in->Handle);
  sFORALL(Out,out)
    midiOutClose(out->Handle);

}

/****************************************************************************/

sBool sMidiHandlerWin::HasInput()
{
  return !InQueue.IsEmpty();
}

sBool sMidiHandlerWin::GetInput(sMidiEvent &e)
{
  return InQueue.RemHead(e);
}

void sMidiHandlerWin::Output(uint8_t dev,uint8_t stat,uint8_t val1,uint8_t val2)
{
  midiOutShortMsg(Out[dev].Handle,stat|(val1<<8)|(val2<<16));
}

/****************************************************************************/

void sInitMidi()
{
  sMidiHandler = new sMidiHandlerWin;
}

void sExitMidi()
{
  delete sMidiHandler;
}


void sAddMidi(sBool logging, sBool onlyPhysical)
{
  sMidiLog = logging;
  sMidiOnlyPhys = onlyPhysical;
  sAddSubsystem(L"midi",0x81,sInitMidi,sExitMidi);
}

/****************************************************************************/
/****************************************************************************/

#endif

/****************************************************************************/
