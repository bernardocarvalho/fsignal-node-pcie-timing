/***************************************************************************
 *   Copyright (C) 2006 by Andre Neto   *
 *   andre.neto@cfn.ist.utl.pt   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FSIGNAL_EVENT_WRAPPER_H
#define FSIGNAL_EVENT_WRAPPER_H

#include "../orb/SCADUtils.hh"
#include "../orb/SCAD.hh"
#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include "TimeStampWrapper.h"
#include "../utils/FSignalUtils.h"

namespace fsignal
{
    class EventWrapper
    {
    public:
        EventWrapper(){}
        
        EventWrapper(const org::cfn::scad::core::corba::hardware::Event &evt){
            tevent      = new TimeStampWrapper(evt.tstamp);
            eventNameUID = Utils::narrowCorba(evt.nameUniqueID);
            eventID     = evt.event_unique_id;
        }
        
        TimeStampWrapper &getEventTime(){
            return tevent;
        }
        
        std::string &getEventNameID(){
            return eventNameUID;
        }
        
        long long getEventID(){
            return eventID;
        }
        
        org::cfn::scad::core::corba::hardware::Event getAsCorbaEvent(){
            org::cfn::scad::core::corba::hardware::Event_var evt = new org::cfn::scad::core::corba::hardware::Event();             
            evt->event_unique_id = eventID;
            evt->nameUniqueID    = Utils::widenCorba(eventNameUID);
            evt->tstamp          = tevent.getAsCorbaTimeStamp();
            return evt;
        }

        inline void operator=(org::cfn::scad::core::corba::hardware::Event &evt){
            tevent      = new TimeStampWrapper(evt.tstamp);
            eventNameUID = Utils::narrowCorba(evt.nameUniqueID);
            eventID     = evt.event_unique_id;
        }
        
        inline void operator=(EventWrapper evt){
            tevent       = evt.tevent;
            eventNameUID = evt.eventNameUID;
            eventID      = evt.eventID;
        }
        
        void setEventNameUID(std::string &eventNameUID){
            this->eventNameUID = eventNameUID;
            
        }
        
        void setEventID(long long eventID){
            this->eventID = eventID;
        }
        
        void setEventTime(TimeStampWrapper &evtTime){
            tevent = evtTime;
        }                
               
        void toString(std::string& str);
        
    private:
        TimeStampWrapper tevent;
        std::string      eventNameUID;
        long long        eventID;
    };

	
    inline void EventWrapper::toString(std::string& str)
    {
        std::stringstream buffer;
        buffer << "Event Name UID = " << eventNameUID << std::endl;
        buffer << "Event ID = " << eventID << std::endl;
        std::string evtTimeStr;
        tevent.toString(evtTimeStr);
        buffer << "Event time = " << evtTimeStr << std::endl;        
        str = buffer.str();
    }
}
#endif
