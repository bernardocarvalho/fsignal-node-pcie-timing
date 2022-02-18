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

#include "EventSem.h"


#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cmath>
#include <pthread.h>
#include "TestNode.h"
#include "../FSHardware.h"
#include "../FSParameter.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"
#include "../utils/FSignalUtils.h"
#include "TestParameter.h"

using namespace std;
using namespace fsignal;
using namespace org::cfn::scad::core::corba;

struct DataProducerInit{
    TestNode      *node;
    FSHardware    *hardware;
};

TestNode::TestNode(const char* propsLoc, wstring mainEventID, const char *logFileLoc) : FSNode(propsLoc, logFileLoc), mainEventID(mainEventID) {

    EventWrapper ew;

    int nparameters = 0;
    struct DataProducerInit *dpis = (struct DataProducerInit *)malloc(sizeof(struct DataProducerInit) * 256);

    nhardware = 0;
    vector<HardwareXml> hwXmls = getParsedHardwareXmls();
    for(unsigned int i=0; i<hwXmls.size(); i++) {
        FSHardware* hardware = new FSHardware(hwXmls[i].getUniqueID(), hwXmls[i].getFields());
        vector<string> parXmls = hwXmls[i].getParametersUniqueIDS();
        for(unsigned int j=0; j<parXmls.size(); j++) {
            TestParameter *parameter = new TestParameter(parXmls[j]);
            nparameters++;
            hardware->addParameter(parameter);
        }
        dpis[i].node      = this;
        dpis[i].hardware  = hardware;
        nhardware++;
        addHardware(hardware);
    }

    connectToServer();
    loadSavedValues();

    time_t nows=time(NULL);
    TimeStampWrapper tsw=TimeStampWrapper(&nows);
    //string str;
    //tsw.toString(str);
    int tis = getTimeInSeconds(tsw);
    cout << "Time now: " << tis << " or " << nows << endl;
    cout << "Difference: " << tis - nows << endl;

    //Create all the threads and synch mechanism
    evtSem.Create();

    //Create all the producers
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(&threads[0], NULL, dataProducer, (void*) dpis);
}

void TestNode::newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend) {
    /*cout << "New data available for: " << Utils::narrowCorba(nodeUniqueID)
     * << "-" << Utils::narrowCorba(hardwareUniqueID) << "-" <<Utils::narrowCorba(parameterUniqueID) << endl;
     *
     * string time;
     * TimeStampWrapper ts(tstart);
     * ts.toString(time);
     * cout << "TStart : " << time << endl;
     * TimeStampWrapper te(tend);
     * te.toString(time);
     * cout << "TEnd: " << time << endl;
     * cout << "Event ID: " << Utils::narrowCorba(eList[0].nameUniqueID) << endl;
     * TimeStampWrapper tevt(eList[0].tstamp);
     * tevt.toString(time);
     * cout << "TEvent: " << time << endl;*/
}

void TestNode::setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList) {
    wchar_t* evtList_uid = NULL;
    for(unsigned int i=0; i<evtList.length(); i++) {
        evtList_uid = CORBA::wstring_dup(evtList[i].nameUniqueID);
        if(evtList_uid == mainEventID) {
            mainEvent = evtList[i];
            setNodeStatus(hardware::OPERATIONAL);
            string evtID = "0x0000";
            EventWrapper eventToSend = EventWrapper(mainEvent);
            eventToSend.setEventNameUID(evtID);
            org::cfn::scad::core::corba::hardware::EventList evtListToSend = org::cfn::scad::core::corba::hardware::EventList(1);
            evtListToSend[0]=eventToSend.getAsCorbaEvent();
            getCentralServer()->fireEventTable(Utils::widenCorba(getUniqueID()), evtListToSend);
            changeAllHardwareStatus(hardware::RUNNING);
            setNodeStatus(hardware::NRUNNING);
            evtSem.Post();
        }
        CORBA::wstring_free(evtList_uid);
    }
}

void *TestNode::dataProducer(void* args) {
    struct DataProducerInit *dpi = (DataProducerInit *)args;
    TestNode             *node      = dpi->node;
    FSHardware           *hardware  = NULL;
    TestParameter        *parameter = NULL;
    TimeStampWrapper     tstart;
    TimeStampWrapper     tend;
    EventWrapper         mainEvt;
    vector<EventWrapper> eventList;
    unsigned char       *buffer     = NULL;
    stringstream         logStream;

    while(true){
        //FIX ME. A node manager thread will do this...
        node->setNodeStatus(hardware::STAND_BY);
        node->changeAllHardwareStatus(hardware::STOPPED);

        node->evtSem.ResetWait();

        eventList.clear();
        mainEvt = node->mainEvent;
        eventList.push_back(mainEvt);

        for(unsigned int i=0; i<node->nhardware; i++){
            int numberOfConnectedChannels = 0;
            hardware  = dpi[i].hardware;
            if(!hardware->isConnected()){
                continue;
            }

            //Check how many channels are connected for this hardware
            std::vector<FSParameter*> hwParameters = hardware->getAllParameters();
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (TestParameter *)hwParameters[j];
                if(parameter->isConnected()){
                    numberOfConnectedChannels++;
                }
            }

            if(numberOfConnectedChannels == 0){
                logStream.str("");
                logStream << "The hardware " << hardware->getUniqueID() << " is connected but the number of connected channels is 0";
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                continue;
            }

            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (TestParameter*)hwParameters[j];
                if(!parameter->isConnected()){
                    continue;
                }
                unsigned int bufferToSendSize = parameter->getNPoints() * sizeof(float);
                buffer = (unsigned char *)malloc(bufferToSendSize);
                if(buffer == NULL){
                    LOG4CXX_INFO(Utils::getLogger(), "Failed to allocated buffer memory for: " + parameter->getUniqueID());
                    continue;
                }
                memset(buffer, bufferToSendSize, 0);


                LOG4CXX_INFO(Utils::getLogger(), "Starting acquisition for " + parameter->getUniqueID());

                if(!node->getDataCluster(*parameter, tstart, tend, eventList, (float *)buffer)){
                    //ERROR
                    continue;
                }

                node->sendNewDataAvailable(
                    hardware->getUniqueID(),
                    parameter->getUniqueID(),
                    tstart,
                    tend,
                    eventList,
                    parameter->getXMLConfiguration(),
                    buffer,
                    bufferToSendSize
                );

                free(buffer);
            }
        }
    }
}

bool TestNode::getDataCluster(TestParameter &par, TimeStampWrapper &tstart, TimeStampWrapper &tend, vector<EventWrapper> &evtList, float *buffer){
    float amp  = par.getAmplitude();
    float freq = par.getFrequency();
    int   npts   = par.getNPoints();
    int   tbs    = par.getTBS();
    float *writeBuffer = buffer;

    if(evtList.size() == 0){
        return false;
    }

    //For now I will assume one event and that it is the correct one. FIX ME
    tstart = evtList[0].getEventTime();
    tend  = tstart;
    tend.addMilliSeconds(npts * tbs);
    evtList[0].setEventID(-1);
    for(int i=0; i<npts; i++){
        *writeBuffer = (float)(amp * sin(2 * M_PI * freq * i * (tbs / 1000.f)));
        writeBuffer++;
        //buffer.push_back((float)(amp * sin(2 * M_PI * freq * i * (tbs / 1000.f))));
    }

    //Utils::copyVecToVecArray(&buffer, bufferRaw);

    return true;
}

int TestNode::getTimeInSeconds(TimeStampWrapper tsw) {
    tm* now = (tm*)malloc(sizeof(struct tm));
    TimeWrapper tw = tsw.getTimeWrapper();
    DateWrapper dw = tsw.getDateWrapper();
    now->tm_sec = tw.getSeconds();
    now->tm_min = tw.getMinutes();
    now->tm_hour = tw.getHours();
    cout << "Time: " << now->tm_hour << ":" <<  now->tm_min << ":" << now->tm_sec << endl;
    now->tm_mday = dw.getDay();
    now->tm_mon = dw.getMonth();
    now->tm_year = dw.getYear() - 1900;
    time_t nowsecs = std::mktime(now);
    free(now);
    return (int)nowsecs;
}
