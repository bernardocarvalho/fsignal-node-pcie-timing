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
#ifndef FSIGNAL_TEST_NODE_H
#define FSIGNAL_TEST_NODE_H
#include "../FSNode.h"
#include "../utils/EventSem.h"
#include "ReflectParameter.hpp"
#include <vector>

namespace fsignal {
    class ReflectNode : public FSNode {
    public:
        ReflectNode(const char* propsLoc, std::wstring mainEventID, const char *logFileLoc=NULL);
        ~ReflectNode(){
            evtSem.Close();
        }
        void setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList);
        void newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend);
        static void* dataProducer(void* args);
        bool getDataCluster(ReflectParameter &par, TimeStampWrapper &tstart, TimeStampWrapper &tend, std::vector<EventWrapper> &evtList, float *buffer);
    private:
        EventSem evtSem;
        org::cfn::scad::core::corba::hardware::Event mainEvent;
        std::wstring mainEventID;
        unsigned int nhardware;
        std::stringstream logStream;
        char telnetHost[32];
        char telnetPort[8];
        static std::string TELNET_HOST;
        static std::string TELNET_PORT;
        static int MODE_FIXED;
        static int MODE_SWEEP;
    };
}

#endif
