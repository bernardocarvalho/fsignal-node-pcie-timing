// This file is generated by omniidl (C++ backend)- omniORB_4_1. Do not edit.

#include "SCADUtils.hh"
#include <omniORB4/IOP_S.h>
#include <omniORB4/IOP_C.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/callHandle.h>
#include <omniORB4/objTracker.h>


OMNI_USING_NAMESPACE(omni)

static const char* _0RL_library_version = omniORB_4_2;



void
org::cfn::scad::core::corba::util::Property::operator>>= (cdrStream &_n) const
{
  _n.marshalWString(name,0);
  (const CORBA::Any&) value >>= _n;

}

void
org::cfn::scad::core::corba::util::Property::operator<<= (cdrStream &_n)
{
  name = _n.unmarshalWString(0);
  (CORBA::Any&)value <<= _n;

}

void
org::cfn::scad::core::corba::util::Time::operator>>= (cdrStream &_n) const
{
  picos >>= _n;
  nanos >>= _n;
  micros >>= _n;
  millis >>= _n;
  _n.marshalOctet(seconds);
  _n.marshalOctet(minutes);
  _n.marshalOctet(hours);

}

void
org::cfn::scad::core::corba::util::Time::operator<<= (cdrStream &_n)
{
  (CORBA::Short&)picos <<= _n;
  (CORBA::Short&)nanos <<= _n;
  (CORBA::Short&)micros <<= _n;
  (CORBA::Short&)millis <<= _n;
  seconds = _n.unmarshalOctet();
  minutes = _n.unmarshalOctet();
  hours = _n.unmarshalOctet();

}

void
org::cfn::scad::core::corba::util::Date::operator>>= (cdrStream &_n) const
{
  _n.marshalOctet(day);
  _n.marshalOctet(month);
  year >>= _n;

}

void
org::cfn::scad::core::corba::util::Date::operator<<= (cdrStream &_n)
{
  day = _n.unmarshalOctet();
  month = _n.unmarshalOctet();
  (CORBA::Short&)year <<= _n;

}

void
org::cfn::scad::core::corba::util::TimeStamp::operator>>= (cdrStream &_n) const
{
  (const Date&) dt >>= _n;
  (const Time&) tm >>= _n;

}

void
org::cfn::scad::core::corba::util::TimeStamp::operator<<= (cdrStream &_n)
{
  (Date&)dt <<= _n;
  (Time&)tm <<= _n;

}

void
org::cfn::scad::core::corba::util::Comment::operator>>= (cdrStream &_n) const
{
  _n.marshalWString(username,0);
  (const TimeStamp&) time >>= _n;
  _n.marshalWString(com,0);

}

void
org::cfn::scad::core::corba::util::Comment::operator<<= (cdrStream &_n)
{
  username = _n.unmarshalWString(0);
  (TimeStamp&)time <<= _n;
  com = _n.unmarshalWString(0);

}
