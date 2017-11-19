/*+**************************************************************************/
/***                                                                      ***/
/***   This file is distributed under a BSD license.                      ***/
/***   See LICENSE.txt for details.                                       ***/
/***                                                                      ***/
/**************************************************************************+*/

/****************************************************************************/
/* BUTTONS are mapped to [0..1]                                             */
/* AXES are mapped to [-1..1]                                               */
/* SCREENCOORDS are mapped to [-1..1]. values must be CLAMPED!!             */
/* ACCELEROMETERS are rawly mapped. gravity is 1                            */
/* KINECT coordinates are in userspace                                      */
/* MOVE coordinates are in cameraspace                                      */
/****************************************************************************/

/****************************************************************************/
/* Example:                                                                 */
/* #include "consoles/input2.hpp"                                           */
/* sInput2Scheme Scheme;                                                    */
/* enum { INPUT_TRIGGER };                                                  */
/* int main() {                                                             */
/*    Scheme.Init(1);                                                       */
/*    sInput2Device* Device = sFindInput2Device(sINPUT2_TYPE_JOYPADXBOX, 0);*/
/*    Scheme.Bind(INPUT_TRIGGER, sInput2Key(Device, sINPUT2_JOYPADXBOX_A)));*/
/*    while (1) {                                                           */
/*      sInput2Update(sGetTime());                                          */
/*      if (Scheme.Pressed(INPUT_TRIGGER))                                  */
/*        sDPrintF(L"triggered!\n");                                        */
/*    }                                                                     */
/* }                                                                        */
/****************************************************************************/

#ifndef FILE_BASE_INPUT2_HPP
#define FILE_BASE_INPUT2_HPP

#include "base/types.hpp"
#include "base/math.hpp"
#include "util/algorithms.hpp"

#define INPUT2_MESSAGE_QUEUE_SIZE 16

enum sInput2Joypad
{
  sINPUT2_JOYPAD_LEFT,
  sINPUT2_JOYPAD_LEFT_X = sINPUT2_JOYPAD_LEFT,
  sINPUT2_JOYPAD_LEFT_Y,
  sINPUT2_JOYPAD_RIGHT,
  sINPUT2_JOYPAD_RIGHT_X = sINPUT2_JOYPAD_RIGHT,
  sINPUT2_JOYPAD_RIGHT_Y,
  sINPUT2_JOYPAD_DIGITAL,
  sINPUT2_JOYPAD_DIGITAL_X = sINPUT2_JOYPAD_DIGITAL,
  sINPUT2_JOYPAD_DIGITAL_Y,
  sINPUT2_JOYPAD_A,
  sINPUT2_JOYPAD_B,
  sINPUT2_JOYPAD_C,
  sINPUT2_JOYPAD_D,
  sINPUT2_JOYPAD_DIGITAL_UP,
  sINPUT2_JOYPAD_DIGITAL_DOWN,
  sINPUT2_JOYPAD_DIGITAL_LEFT,
  sINPUT2_JOYPAD_DIGITAL_RIGHT,
  sINPUT2_JOYPAD_BACK,
  sINPUT2_JOYPAD_START,
  sINPUT2_JOYPAD_LT,
  sINPUT2_JOYPAD_LB,
  sINPUT2_JOYPAD_RT,
  sINPUT2_JOYPAD_RB,
  sINPUT2_JOYPAD_THUMB_L,
  sINPUT2_JOYPAD_THUMB_R,
  sINPUT2_JOYPAD_MAX
};

enum sInput2JoypadXBox
{
  sINPUT2_JOYPADXBOX_LEFT,
  sINPUT2_JOYPADXBOX_LEFT_X = sINPUT2_JOYPADXBOX_LEFT,
  sINPUT2_JOYPADXBOX_LEFT_Y,
  sINPUT2_JOYPADXBOX_RIGHT,
  sINPUT2_JOYPADXBOX_RIGHT_X = sINPUT2_JOYPADXBOX_RIGHT,
  sINPUT2_JOYPADXBOX_RIGHT_Y,
  sINPUT2_JOYPADXBOX_DIGITAL,
  sINPUT2_JOYPADXBOX_DIGITAL_X = sINPUT2_JOYPADXBOX_DIGITAL,
  sINPUT2_JOYPADXBOX_DIGITAL_Y,
  sINPUT2_JOYPADXBOX_A,
  sINPUT2_JOYPADXBOX_B,
  sINPUT2_JOYPADXBOX_X,
  sINPUT2_JOYPADXBOX_Y,
  sINPUT2_JOYPADXBOX_DIGITAL_UP,
  sINPUT2_JOYPADXBOX_DIGITAL_DOWN,
  sINPUT2_JOYPADXBOX_DIGITAL_LEFT,
  sINPUT2_JOYPADXBOX_DIGITAL_RIGHT,
  sINPUT2_JOYPADXBOX_BACK,
  sINPUT2_JOYPADXBOX_START,
  sINPUT2_JOYPADXBOX_LT,
  sINPUT2_JOYPADXBOX_LB,
  sINPUT2_JOYPADXBOX_RT,
  sINPUT2_JOYPADXBOX_RB,
  sINPUT2_JOYPADXBOX_THUMB_L,
  sINPUT2_JOYPADXBOX_THUMB_R,
  sINPUT2_JOYPADXBOX_MAX
};

enum sInput2Mouse
{
  sINPUT2_MOUSE_POS, // screen space position
  sINPUT2_MOUSE_X = sINPUT2_MOUSE_POS,
  sINPUT2_MOUSE_Y,  
  sINPUT2_MOUSE_RAW, // raw position (unbounded)
  sINPUT2_MOUSE_RAW_X = sINPUT2_MOUSE_RAW,
  sINPUT2_MOUSE_RAW_Y,
  sINPUT2_MOUSE_WHEEL,
  sINPUT2_MOUSE_LMB,
  sINPUT2_MOUSE_RMB,
  sINPUT2_MOUSE_MMB,
  sINPUT2_MOUSE_X1,
  sINPUT2_MOUSE_X2,
  sINPUT2_MOUSE_X3,
  sINPUT2_MOUSE_X4,
  sINPUT2_MOUSE_X5,
  sINPUT2_MOUSE_VALID,
  sINPUT2_MOUSE_MAX
};

enum sInput2Keyboard { sINPUT2_KEYBOARD_ESCAPE = 1, sINPUT2_KEYBOARD_1 = 2, sINPUT2_KEYBOARD_2, sINPUT2_KEYBOARD_3, sINPUT2_KEYBOARD_4, sINPUT2_KEYBOARD_5, sINPUT2_KEYBOARD_6, sINPUT2_KEYBOARD_7, sINPUT2_KEYBOARD_8, sINPUT2_KEYBOARD_9, sINPUT2_KEYBOARD_0, 
  sINPUT2_KEYBOARD_A = 30, sINPUT2_KEYBOARD_B = 48, sINPUT2_KEYBOARD_C = 46, sINPUT2_KEYBOARD_D = 32, sINPUT2_KEYBOARD_E = 18, sINPUT2_KEYBOARD_F = 33, sINPUT2_KEYBOARD_G = 34, sINPUT2_KEYBOARD_H = 35, sINPUT2_KEYBOARD_I = 23, sINPUT2_KEYBOARD_J = 36, sINPUT2_KEYBOARD_K = 37, sINPUT2_KEYBOARD_L = 38, sINPUT2_KEYBOARD_M = 50, sINPUT2_KEYBOARD_N = 49, sINPUT2_KEYBOARD_O = 24, sINPUT2_KEYBOARD_P = 25, sINPUT2_KEYBOARD_Q = 16, sINPUT2_KEYBOARD_R = 19 , sINPUT2_KEYBOARD_S = 31, sINPUT2_KEYBOARD_T = 20 , sINPUT2_KEYBOARD_U = 22, sINPUT2_KEYBOARD_V = 47, sINPUT2_KEYBOARD_W = 17 , sINPUT2_KEYBOARD_X = 45, sINPUT2_KEYBOARD_Y = 44, sINPUT2_KEYBOARD_Z = 21, 
  sINPUT2_KEYBOARD_F1 = 59, sINPUT2_KEYBOARD_F2, sINPUT2_KEYBOARD_F3, sINPUT2_KEYBOARD_F4, sINPUT2_KEYBOARD_F5, sINPUT2_KEYBOARD_F6, sINPUT2_KEYBOARD_F7, sINPUT2_KEYBOARD_F8, sINPUT2_KEYBOARD_F9, sINPUT2_KEYBOARD_F10, sINPUT2_KEYBOARD_F11 = 87, sINPUT2_KEYBOARD_F12 = 88, 
  sINPUT2_KEYBOARD_COMMA = 51, sINPUT2_KEYBOARD_FULLSTOP = 52, 
  sINPUT2_KEYBOARD_TAB = 15, sINPUT2_KEYBOARD_LSHIFT = 42, sINPUT2_KEYBOARD_RSHIFT = 54, sINPUT2_KEYBOARD_LCTRL = 29, sINPUT2_KEYBOARD_RCTRL, sINPUT2_KEYBOARD_LALT, sINPUT2_KEYBOARD_RALT, sINPUT2_KEYBOARD_LWIN, sINPUT2_KEYBOARD_RWIN, sINPUT2_KEYBOARD_MENU, sINPUT2_KEYBOARD_CAPSLOCK, sINPUT2_KEYBOARD_SPACE = 57, sINPUT2_KEYBOARD_BACKSPACE = 14, sINPUT2_KEYBOARD_ENTER = 28, sINPUT2_KEYBOARD_PRINT, sINPUT2_KEYBOARD_SCROLLLOCK, sINPUT2_KEYBOARD_PAUSE, sINPUT2_KEYBOARD_UP = 72, sINPUT2_KEYBOARD_DOWN = 80, sINPUT2_KEYBOARD_LEFT = 75, sINPUT2_KEYBOARD_RIGHT = 77, sINPUT2_KEYBOARD_INSERT = 82, sINPUT2_KEYBOARD_DELETE = 83, sINPUT2_KEYBOARD_POS1 = 71, sINPUT2_KEYBOARD_END = 79, sINPUT2_KEYBOARD_PAGEUP = 73, sINPUT2_KEYBOARD_PAGEDOWN = 81, 
  sINPUT2_KEYBOARD_NUMLOCK, sINPUT2_KEYBOARD_NUM_DIVIDE, sINPUT2_KEYBOARD_NUM_MULTIPLY, sINPUT2_KEYBOARD_NUM_MINUS = 74, sINPUT2_KEYBOARD_NUM_PLUS = 78, sINPUT2_KEYBOARD_NUM_ENTER, sINPUT2_KEYBOARD_NUM_0, sINPUT2_KEYBOARD_NUM_1, sINPUT2_KEYBOARD_NUM_2, sINPUT2_KEYBOARD_NUM_3, sINPUT2_KEYBOARD_NUM_4, sINPUT2_KEYBOARD_NUM_5, sINPUT2_KEYBOARD_NUM_6, sINPUT2_KEYBOARD_NUM_7, sINPUT2_KEYBOARD_NUM_8, sINPUT2_KEYBOARD_NUM_9, sINPUT2_KEYBOARD_NUM_COMMA, 
  sINPUT2_KEYBOARD_MAX = 256
};

enum sInput2Type 
{
  sINPUT2_TYPE_UNKNOWN = 0,
  sINPUT2_TYPE_JOYPAD           = sMAKE4('J', 'P', 'A', 'D'),
  sINPUT2_TYPE_JOYPADXBOX       = sMAKE4('J', 'X', 'B', 'X'),
  sINPUT2_TYPE_MOUSE            = sMAKE4('M', 'O', 'U', 'S'),
  sINPUT2_TYPE_KEYBOARD         = sMAKE4('K', 'Y', 'B', 'D'),
  sINPUT2_TYPE_CUSTOM           = sMAKE4('C', 'U', 'S', 'T'),
};

enum sInput2Status {
  sINPUT2_STATUS_NOT_CONNECTED              = 0x00000001,
  sINPUT2_STATUS_CAMERA_NOT_CONNECTED       = 0x00000002,
  sINPUT2_STATUS_ERROR                      = 0x80000000, // general error, only null-data is send from device
};


/****************************************************************************/

//! \ingroup altona_base_system_input2
//! Input2Device class
//!
//! State of specific device. Mostly used to pass to Scheme.Bind() and
//! to ask for the current state of the device (eg. if it's disconnected)

class sInput2Device {
friend class sInput2Scheme;
public:
  virtual              ~sInput2Device() {}
                       sInput2Device() { MotorSlow = MotorFast = 0; }
  virtual void         OnGameStep(int time, sBool ignoreTimestamp = sFALSE, sBool mute = sFALSE) = 0;// call once per gamestep        
  virtual float         GetRel(int id) const = 0;                       //! get relative value of id and return zero if below threshold
  virtual float         GetAbs(int id) const = 0;                       //! get absolute value of id and return zero if below threshold
  virtual uint32_t         GetStatusWord() const = 0;                       //! return administrative status of device (connected, identified, ....)
  sBool                IsStatus(sInput2Status condition) const { return (GetStatusWord()&condition)!=0; }
  virtual int         GetTimestamp() const = 0;                        //! return timestamp prior to current gamestep time
  virtual uint32_t         GetType() const = 0;                             //! return device type
  virtual int         GetId() const = 0;                               //! return device id
  virtual int         Pressed(int id, float threshold) const = 0;
  virtual int         Released(int id, float threshold) const = 0;
  
  virtual sArrayRange<float> GetStatusVector() = 0;          // returns full statusvector, don't use it if you don't write for gameframes

  void                 SetMotor(int slow, int fast) { MotorSlow = slow; MotorFast = fast; } //! set motor of the device. this should REALLY be replaced by something more intelligent and is not really part of an input system.
  volatile int MotorSlow;
  volatile int MotorFast;

  sDNode DNode;

protected:
  virtual int KeyCount() const = 0;
  
};



/****************************************************************************/


template<int NUM>
class sInput2DeviceImpl : public sInput2Device 
{
public:
  sInput2DeviceImpl(uint32_t type, int id) 
  { 
    Type = type;
    Id = id;
    Clear();
  }

  int KeyCount() const        { return NUM; }
  uint32_t GetType() const         { return Type; }
  int GetId() const           { return Id; }

  void Clear()
  {
    Status = sINPUT2_STATUS_NOT_CONNECTED | sINPUT2_STATUS_ERROR; 
    sClear(ValuesAbs);
    sClear(ValuesRel);
  }

  void OnGameStep(int time, sBool ignoreTimestamp = sFALSE, sBool mute = sFALSE) 
  {
    // calculate from all applicable values
    sClear(ValuesRel);
    while(!Values.IsEmpty() && (time >= Values.Front().Timestamp || ignoreTimestamp)) {
      // remove from queue
      Value_ v;
      Values.RemHead(v);

      if (!mute) {
        for (int i=0; i<NUM; i++) { 
          ValuesRel[i] +=v.Value[i] - ValuesAbs[i];    // calc all deltas and add them to Rel Value
          ValuesAbs[i] = v.Value[i];                   // set new Abs Value
        }
      }

      // get timestamp and error
      Timestamp = v.Timestamp;
      Status    = v.Status;
    }
  }

  sINLINE float GetRel(int id) const
  {
    sVERIFY2(id >= 0 && id < NUM, L"input2: invalid key id specified in sInput2Device::getRel() or sInput2Scheme::Bind()")
    return ValuesRel[id];
  }

  sINLINE float GetAbs(int id) const
  {
    sVERIFY2(id >= 0 && id < NUM, L"input2: invalid key id specified in sInput2Device::getAbs() or sInput2Scheme::Bind()")
    return ValuesAbs[id];
  }

  sINLINE int Pressed(int id, float threshold) const
  {
    sVERIFY2(id >= 0 && id < NUM, L"input2: invalid key id specified in sInput2Device::getPressed() or sInput2Scheme::Bind()")
#if 1
    float abs = ValuesAbs[id];
    float old = abs-ValuesRel[id];
#else
    float abs = GetAbs(id);
    float old = abs - GetRel(id);
#endif
    if (threshold > 0.0f)
      return abs >= threshold && old < threshold ? 1 : 0;
    if (threshold < 0.0f)
      return abs <= threshold && old > threshold ? -1 : 0;

    return 0;
  }

  sINLINE int Released(int id, float threshold) const
  {
    sVERIFY2(id >= 0 && id < NUM, L"input2: invalid key id specified in sInput2Device::getReleased() or sInput2Scheme::Bind()")

#if 1
    float abs = ValuesAbs[id];
    float old = abs-ValuesRel[id];
#else
    float abs = GetAbs(id);
    float old = abs - GetRel(id);
#endif
    if (threshold > 0.0f)
      return abs < threshold && old >= threshold ? 1 : 0;
    if (threshold < 0.0f)
      return abs > threshold && old <= threshold ? -1 : 0;
     
    return 0;
  }

  uint32_t GetStatusWord() const { return Status; }

  int GetTimestamp() const { return Timestamp; }

  // everything below is FOR INTERNAL USE ONLY

  struct Value_ {
    float          Value[NUM];
    int          Timestamp;
    uint32_t          Status;
  };
  void addValues(const Value_& v)
  {
    Values.AddTail(v);
  }

  sArrayRange<float> GetStatusVector()
  {
    return sAll(ValuesAbs);
  }

private:

  sLockQueue<Value_, INPUT2_MESSAGE_QUEUE_SIZE> Values; // written by low-level input thread
  float        ValuesAbs[NUM];                           // current values for Get*() functions. this is set by OnGameStep
  float        ValuesRel[NUM];                           // relative values since last OnGameStep
  uint32_t        Type;                                     // device type
  int        Id;                                       // device ordinal
  uint32_t        Status;
  int        Timestamp;
};


/****************************************************************************/
/***                                                                      ***/
/***   input2 device management                                           ***/
/***                                                                      ***/
/****************************************************************************/

sInput2Device* sFindInput2Device(uint32_t type, int id);         //! find a specific sINPUT2_TYPE_??? device
int sInput2NumDevices(uint32_t type);                            //! find number of devices of a specific sINPUT2_TYPE_??? type
int sInput2NumDevicesConnected(int type);                   //! find number of connected devices of a specific sINPUT2_TYPE_??? type
void sInput2Update(int time, sBool ignoreTimstamp = sFALSE); //! call this once per gamestep
void sInput2RegisterDevice(sInput2Device* device);            // only use in system implementations 
void sInput2RemoveDevice(sInput2Device* device);              // only use in system implementations
void sInput2SetMute(sBool pause);                             //! set mute state for the WHOLE system. use for system menues.

/****************************************************************************/

template <int NUM> class sInput2CustomEvaluator : public sInput2DeviceImpl<NUM>
{
public:

  sInput2CustomEvaluator() : sInput2DeviceImpl<NUM>(sINPUT2_TYPE_UNKNOWN, 0), LastTime(0)
  {
    extern int sInput2CECounter;
    Id = sInput2CECounter++;
    sInput2RegisterDevice(this);
  }

  ~sInput2CustomEvaluator()
  {
    sInput2RemoveDevice(this);
  }

protected:

  virtual sBool Tick(int delta, float *out) = 0;
  virtual void Reset() {};

private:

  static int IdCounter;
  int Id;
  int LastTime;

  int GetId() { return Id; }

  void OnGameStep(int time, sBool ignoreTimestamp = sFALSE, sBool mute = sFALSE)
  {
    if (LastTime == 0 || ignoreTimestamp)
    {
      Reset();
      LastTime = time;
    }

    if (!mute)
    {
      typename sInput2DeviceImpl<NUM>::Value_ values;
      values.Status=0;
      values.Timestamp=time;
      if (Tick(time-LastTime,values.Value))
        addValues(values);
      else
        values.Status=1;
    }
    LastTime=time;

    sInput2DeviceImpl<NUM>::OnGameStep(time, ignoreTimestamp, mute);
  }

};

/****************************************************************************/

class sInput2Key 
{
friend class sInput2Scheme;
friend class sInput2Mapping;
public:
  sInput2Key() { Device = sNULL; KeyId = 0; ThresholdAnalog = 0.0f; ThresholdDigitalLo = 0.5f; ThresholdDigitalHi = 0.8f; Next = sNULL; }
  sInput2Key(const sInput2Key& other) { Device = other.Device; KeyId = other.KeyId; ThresholdAnalog = other.ThresholdAnalog; ThresholdDigitalHi = other.ThresholdDigitalHi; ThresholdDigitalLo = other.ThresholdDigitalLo; Next = 0; }
  sInput2Key(const sInput2Device* device, int keyId, float thresholdAnalog = 0.0f, float thresholdDigitalLo = 0.2f, float thresholdDigitalHi = 0.8f)
  { 
    Device = device;
    KeyId = keyId; 
    ThresholdAnalog = thresholdAnalog;
    ThresholdDigitalLo = thresholdDigitalLo;
    ThresholdDigitalHi = thresholdDigitalHi;
    Next = sNULL;
  }
  sBool operator == (const sInput2Key& other) const { return Device == other.Device && KeyId == other.KeyId && ThresholdAnalog == other.ThresholdAnalog && ThresholdDigitalLo == other.ThresholdDigitalLo && ThresholdDigitalHi == other.ThresholdDigitalHi;}

private:
  const sInput2Device* Device;
  int KeyId;
  float ThresholdAnalog;
  float ThresholdDigitalLo;
  float ThresholdDigitalHi;
  sInput2Key* Next;
};

/****************************************************************************/

//! \ingroup altona_base_system_input2
//! input2-Scheme
//!
//! Bindingscheme that manages all bindings of one or more input-devices.<br>
//! You can specify more than one binding to one binding-id. The bindings do not have to be of the same device.<br>
//! The access-functions define the semantic of the binding. The same binding-id can be used by different access-functions at any time.

class sInput2Scheme 
{
public:
  static const int DEFAULT_RETRIGGER_RATE =100;
  static const int DEFAULT_RETRIGGER_DELAY=500;

  static const int DEFAULT_RETRIGGER_RATE_KINECT = 200;
  static const int DEFAULT_RETRIGGER_DELAY_KINECT = 1400;

              ~sInput2Scheme();
  void        Init(int capacity);                    //! init capacity. specify maximum binding-id + 1. <pre>enum { INPUT_LEFT, INPUT_RIGHT, INPUT_MAX }; Scheme.Init(INPUT_MAX); </pre> is a good idiom.
  void        OnGameStep(int delta);                 //! tick once per frame. only needed when using retriggers
  void        Bind(int id, const sInput2Key& key);   //! bind a key to an id
  void        Bind(int id, sInput2Key* key);         //! bind a list of keys to an id
  void        Bind(int id, const sInput2Device* device, int keyId, float thresholdAnalog = 0.0f, float thresholdDigitalLo = 0.2f, float thresholdDigitalHi = 0.8f); //! convenience function
  void        SetRetrigger(int id, int rate=DEFAULT_RETRIGGER_RATE, int delay=DEFAULT_RETRIGGER_DELAY); //! set key retrigger values for id. \param rate retrigger rate after delay \param delay retrigger delay before first retrigger
  void        SetRetriggerPermanent(int id, int rate, int delay); //! for ageing-test will always retrigger 
  void        Unbind(int id);                        //! unbind all keys with id

  int        Pressed(int id) const;                 //! number of presses since last gamestep
  int        Released(int id) const;                //! number of releases since last gamestep
  sBool       Hold(int id) const;                    //! current status mapped to [0..1]
  float        Relative(int id) const;                //! accumulated relative movement since last gamestep
  float        Analog(int id) const;                  //! current status
  sVector2    Coords(int id) const;                  //! 2d vector. specify only axes as ids.
  sVector31   Position(int id) const;                //! 3d vector. use for kinect & move positions
  sVector30   Normal(int id) const;                  //! 3d vector. use for kinect & move normals
  sQuaternion Quaternion(int id) const;              //! quaternion. use for move rotation
  sVector4    Vector4(int id) const;                 //! 4d vector. use for kinect floor equation

  int        GetTimestamp() const;                   //! get timestamp of all bindings of this scheme for current tick

  const sInput2Device* GetDevice(int id) const;      //! get device of the binding if Hold() is true. if more than one binding returns true, the first one is returned

private:
  sStaticArray<sInput2Key*>   Bindings;
  mutable sStaticArray<int>  RetriggerTime;
  sStaticArray<int>          RetriggerRate;
  sStaticArray<int>          RetriggerDelay;
  mutable sStaticArray<sBool> RetriggerPressed;
};

/****************************************************************************/

class sInput2Mapping {
public:
  sInput2Mapping();
  ~sInput2Mapping();
  void Init(int maxPlayers, int maxKeys);                  // init
  void Clear();                                              // clear all bindings for all players
  void Rem(int player, int logicalKey);                    // clear a specific binding for a specific player
  void Add(int player, int logicalKey, sInput2Key key);    // add a binding for a specific player
  void Add(int player, int logicalKey, const sInput2Device* device, int key, float ThresholdAnalog = 0.0f, float ThresholdDigital = 0.2f); // convenience function
  sInput2Key* Get(int player, int logicalKey);             // get a list of keybindings that can be passed to Scheme.Bind()

private:
  sStaticArray<sInput2Key*> Mapping;
  int MaxPlayers;
  int MaxKeys;
};


#endif // FILE_BASE_INPUT2_HPP
