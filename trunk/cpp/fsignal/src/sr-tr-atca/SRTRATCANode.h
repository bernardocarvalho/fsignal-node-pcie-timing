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
#ifndef FSIGNAL_SRTRATCA_NODE_H
#define FSIGNAL_SRTRATCA_NODE_H
#include "FSNode.h"
#include "EventSem.h"
#include "SRTRATCAParameter.h"
#include <vector>

namespace fsignal {
    class SRTRATCANode : public FSNode {
    public:
        SRTRATCANode(const char* propsLoc, const char *logFileLoc=NULL);
        ~SRTRATCANode(){
            evtSem.Close();
        }
        void setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList);
        void newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend);
        virtual void abort();
        static void* dataProducer(void* args);
    private:
        EventSem                                     evtSem;
        org::cfn::scad::core::corba::hardware::Event mainEvent;
        std::vector<std::wstring>                    eventTriggerIDs;
        std::wstring                                 eventEndOfPulseID;
        int                                          nparameters;
        int                                          nhardware;
        std::stringstream                            logStream;
        bool                                         aborted;
        bool                                         pulseTerminated;
        static const std::string                     DEV_FILE_LOCATIONS_ID;
        //Arm the acquisition as a reply to this event        
        static const std::string                     TRIGGER_EVT_IDs; 
	//Terminate the acquisition as a reply to this event
	static const std::string                     END_OF_PULSE_EVT_ID;
        //when this number of read bytes is read. New data is sent
        static const std::string                     FS_BUFFER_SEND_SIZE;
        unsigned int                                 bufferSendSize;
        //Store all the device ids 
        std::vector<std::string>                     devIds;
    };
}

#endif
