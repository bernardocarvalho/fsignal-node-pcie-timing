/**
 *
 * @file PcieTimingNode.h
 * @author Bernardo Carvalho
 * @date 2017-10-17
 * @brief .
 *
 *
 * Copyright 2006-2017 IPFN-Instituto Superior Tecnico, Portugal
 *
 * Licensed under the EUPL, Version 1.2 or - as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 *   writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

#ifndef FSIGNAL_PCIE_TIMING_NODE_H
#define FSIGNAL_PCIE_TIMING_NODE_H
#include "FSNode.h"
#include "EventSem.h"
#include "PcieTimingParameter.h"
#include <vector>
#include "cadef.h"

namespace fsignal {
    class PcieTimingNode : public FSNode {
    public:
        PcieTimingNode(const char* propsLoc, std::wstring mainEventID, const char *logFileLoc=NULL);
        ~PcieTimingNode(){
            evtSem.Close();
        }
        void setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList);
        void newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend);
        virtual void abort();
        static void* dataProducer(void* args);
        virtual void changeHardwareStatus(const CORBA::WChar* hardwareUniqueID, fscns::hardware::HardwareStatus newStatus);

    private:
        EventSem                                     evtSem;
        org::cfn::scad::core::corba::hardware::Event mainEvent;
        std::wstring                                 mainEventID;
        //int                                          nparameters;
        int                                          nhardware;
        std::stringstream                            logStream;
        bool                                         aborted;
        static const std::string                     DEV_FILE_LOCATIONS_ID;
        //when this number of read bytes is read. New data is sent
        static const std::string                     FS_BUFFER_SEND_SIZE;
        //static const std::string                     PV_VVESSEL_TEMP;
        //static const char *                      PV_VVESSEL_TEMP;
        static const char *                      PV_COUNTDOWN;
        //unsigned int                                 bufferSendSize;
        //The location of the char devices for each hardware
        std::vector<std::string>                     devIds;
        //mktime seem to be influenced by CORBA.
        //This gives the difference between PC time and CORBA time
        int                                          timeCorr;
        chid                        pv_countdwn_id; // epics temperature PC */
    };
}

#endif
