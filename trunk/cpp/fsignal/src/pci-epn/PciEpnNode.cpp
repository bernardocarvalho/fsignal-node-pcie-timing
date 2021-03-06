/***************************************************************************
 *   Copyright (C) 2006 by Andre Neto   *
 *   andre.neto@ipfn.ist.utl.pt   *
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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "PciEpnNode.h"
#include "PciEpnParameter.h"
#include "FSHardware.h"
#include "FSParameter.h"
#include "TimeStampWrapper.h"
#include "EventWrapper.h"
#include "FSignalUtils.h"

#include "pci_epn_lib.h"
#include "pci_epn_ioctl.h"


using namespace std;
using namespace fsignal;
using namespace org::cfn::scad::core::corba;

struct DataProducerInit{
    PciEpnNode *node;
    FSHardware      *hardware;
};

const string PciEpnNode::DEV_FILE_LOCATIONS_ID = "DevFileLocations";
const string PciEpnNode::FS_BUFFER_SEND_SIZE   = "BufferSendSize";

PciEpnNode::PciEpnNode(const char* propsLoc, wstring mainEventID, const char *logFileLoc) : FSNode(propsLoc, logFileLoc), mainEventID(mainEventID){

    EventWrapper ew;
    struct DataProducerInit *dpis = (struct DataProducerInit *)malloc(sizeof(struct DataProducerInit) * 256);
    aborted = false;
    //Check the location of the device ids
    Utils::tokenize(props.getValue(DEV_FILE_LOCATIONS_ID), devIds);

    bufferSendSize = 0x200000;
    if(props.getValue(FS_BUFFER_SEND_SIZE) != ""){
        bufferSendSize = atoi(props.getValue(FS_BUFFER_SEND_SIZE).c_str());
        if(bufferSendSize < 1){
            bufferSendSize = 0x200000;
        }
    }
    logStream.str("");
    logStream << "The buffer send size is: " << bufferSendSize;
    LOG4CXX_INFO(Utils::getLogger(), logStream.str());

    vector<HardwareXml> hwXmls = getParsedHardwareXmls();
    if(hwXmls.size() != devIds.size()){
        logStream.str("");
        logStream << "The number of device ids(" << devIds.size() << ") is different from the number of hardware xmls(" << hwXmls.size() << ") ABORTING!";
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
        exit(-1);
    }

    //Check if all the devIds look ok
    for(unsigned int i=0; i<devIds.size(); i++) {
        int fd = open(devIds[i].c_str(), O_RDWR);
        if(fd < 0){
            logStream.str("");
            logStream << "Failed to open device at:" << devIds[i] << " This is a very serious error!";
            LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            //exit(-1);
        }
        close(fd);
    }

    nhardware   = 0;
    for(unsigned int i=0; i<hwXmls.size(); i++) {
        FSHardware *hardware = new FSHardware(hwXmls[i].getUniqueID(), hwXmls[i].getFields());
        vector<string> parXmls = hwXmls[i].getParametersUniqueIDS();
        for(unsigned int j=0; j<parXmls.size(); j++) {
            PciEpnParameter *parameter = new PciEpnParameter(parXmls[j], j+1);
            hardware->addParameter(parameter);
            parameter->devLocation = devIds[i];
        }
        dpis[i].node      = this;
        dpis[i].hardware  = hardware;
        nhardware++;
        addHardware(hardware);
    }

    connectToServer();
    loadSavedValues();

    //Create all the threads and synch mechanism
    evtSem.Create();

    //Create all the producers
    //for a driver limitation only a parameter can be acquired at a time...
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(&threads[0], NULL, dataProducer, (void *)dpis);

}

void PciEpnNode::newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend) {
}

void PciEpnNode::abort(){
    logStream.str("");
    logStream << "Acquisition abort request!";
    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
    aborted = true;
}

void PciEpnNode::setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList) {
    wchar_t* evtList_uid = NULL;
    for(unsigned int i=0; i<evtList.length(); i++) {
        evtList_uid = CORBA::wstring_dup(evtList[i].nameUniqueID);
        if(evtList_uid == mainEventID) {
            mainEvent = evtList[i];
            setNodeStatus(hardware::OPERATIONAL);
            changeAllHardwareStatus(hardware::RUNNING);
            setNodeStatus(hardware::NRUNNING);
            aborted = false;
            evtSem.Post();
        }
        CORBA::wstring_free(evtList_uid);
    }
}

void *PciEpnNode::dataProducer(void* args) {
    struct DataProducerInit  *dpi         = (DataProducerInit *)args;
    PciEpnNode          *node        = dpi[0].node;
    PciEpnParameter     *parameter   = NULL;
    FSHardware               *hardware    = NULL;
    TimeStampWrapper          tstart;
    TimeStampWrapper          tend;
    EventWrapper              mainEvt;
    vector<EventWrapper>      eventList;
    unsigned char            *buffer;
    stringstream              logStream;
    org::cfn::scad::core::corba::util::TimeStamp tstamp;

    vector<int>               fd;

    int                       device      = 0;
    int                       ret         = 0;
    unsigned int              val;

    status_port_t sPort;

    for(int i=0; i<node->nhardware; i++){
        fd.push_back(0);
    }

    while(true){
        node->setNodeStatus(hardware::STAND_BY);
        node->changeAllHardwareStatus(hardware::STOPPED);

        node->evtSem.ResetWait();

        eventList.clear();

        mainEvt = node->mainEvent;
        eventList.push_back(mainEvt);

        //open devices and check ioctl
        for(int i=0; i<node->nhardware; i++){
            hardware = dpi[i].hardware;
            if(!hardware->isConnected()){
                continue;
            }
            device = i;
            fd[i] = open(node->devIds[i].c_str(), O_RDONLY);
            if(fd[i] < 0){
                logStream.str("");
                logStream << "While starting acquisition, error opening the device " << node->devIds[i];
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            }
            else{
                logStream.str("");
                logStream << "Opening the device " << node->devIds[i];
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            }
            struct timeval tval;
            gettimeofday(&tval,  NULL);
            dual64_t var64;
            var64.By_32.dataHigh=(unsigned int) tval.tv_sec;    // Seconds
            var64.By_32.dataLow = ((unsigned int)tval.tv_usec) * 1000;   // nanoSeconds

            ret = ::ioctl(fd[i],  PCI_EPN_IOCS_SET_TIME, &var64);
            ret += ::ioctl(fd[i], PCI_EPN_IOCT_PING);
            ret += ::ioctl(fd[i], PCI_EPN_IOCG_STATUS, &sPort);
            if(ret < 0){
                logStream.str("");
                logStream << "While getting Status, error ioctling the device " << node->devIds[i];
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            }
            else{
                logStream.str("");
                logStream << "gettimeofday()= " << tval.tv_sec << "sec "<< tval.tv_usec<<" usec" << endl;
                logStream << "HW Timer REG1 " <<  sPort.timeReg1.By_32.dataHigh <<". " << sPort.timeReg1.By_32.dataLow;
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            }

        }




        tstart     =   mainEvt.getEventTime();
        tstamp = tstart.getAsCorbaTimeStamp();
        long tmeS = tstart.getTimeInSeconds();
        long tmenS =tstart.getTimeInNanos() - tmeS *1000000000;
        logStream.str("");
        logStream << "tstart sec=  " << tmeS << " nSec =  " << tmenS ;
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());

        if(node->aborted){
            continue;
        }

        int  numberOfConnectedChannels = 0;

        for(int i=0; i<node->nhardware; i++){
            numberOfConnectedChannels = 0;
            hardware  = dpi[i].hardware;
            if(!hardware->isConnected()){
                continue;
            }
            //Check how many channels are connected for this hardware
            std::vector<FSParameter*> hwParameters = hardware->getAllParameters();
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (PciEpnParameter *)hwParameters[j];
                if(parameter->isConnected()){
                    numberOfConnectedChannels++;
                }
            }
                //int offset     = 0;
            if(numberOfConnectedChannels == 0){
                logStream.str("");
                logStream << "The hardware " << hardware->getUniqueID() << " is connected but the number of connected channels is 0";
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                continue;
            }

            unsigned int bufferToSendSize = 0;
            buffer = (unsigned char *)malloc(node->bufferSendSize * 2);
            memset(buffer, node->bufferSendSize, 0);
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (PciEpnParameter*)hwParameters[j];
                if(!parameter->isConnected()){
                    continue;
                }

                std::string parameterXML = parameter->getXMLConfiguration();

                tstart     = mainEvt.getEventTime();

                tend       = tstart;

                ret = ioctl(fd[i], PCI_EPN_IOCS_SET_TRG_CHAN, &j);// Set chan
                val = (unsigned int) parameter->getPulseWidth();
                logStream.str("");
                logStream << "Starting configure channel " << j << ", Sec: " <<parameter->getTriggerSecs();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                ret += ioctl(fd[i], PCI_EPN_IOCS_SET_WIDTH, &val);
                if(ret < 0){
                    logStream.str("");
                    logStream << "While seting PULSE_WIDTH, error ioctling the device " << node->devIds[i];
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                }
                dual64_t var64;
                unsigned int trgnS  = (unsigned int) ( parameter->getTriggerNSecs() + tmenS);
                var64.By_32.dataHigh= (unsigned int) ( parameter->getTriggerSecs()  + tmeS);    // Seconds
                if(trgnS >= 1000000000){ // crossed 1 sec border
                    trgnS -=1000000000;
                    var64.By_32.dataHigh +=1;
                }
/*                logStream.str("");
                logStream << "parameter->getTriggerSecs()  " << parameter->getTriggerSecs()<< endl;
                logStream << "parameter->getTriggerNSecs()  " << parameter->getTriggerNSecs()<< ", trgnS " << trgnS<<endl;
                logStream << "parameter->getPulseWidth()  " << parameter->getPulseWidth();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
*/

                var64.By_32.dataLow= trgnS;            // nanoSeconds
                ret = ioctl(fd[i], PCI_EPN_IOCS_SET_TRG, &var64);
                if(ret < 0){
                    logStream.str("");
                    logStream << "While seting trig time, error ioctling the device " << node->devIds[i];
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                }

//
//                    tend.addNanoSeconds((int)(bufferToSendSize / sizeof(int) / (double)(parameter->getAcquisitionFrequency()) * 1e9));
                eventList[0].setEventID(-1);
                node->sendNewDataAvailable(
                    hardware->getUniqueID(),
                    parameter->getUniqueID(),
                    tstart,
                    tend,
                    eventList,
                    parameterXML,
                    buffer,
                    bufferToSendSize
                                           );
            }

            free(buffer);

            //Wait for the queue to be empty before changing the state again
            while(!node->sendQueue.empty()){
                sleep(1);
            }
            for(int i=0; i<node->nhardware; i++){
                hardware = dpi[i].hardware;
                if(!hardware->isConnected()){
                    continue;
                }
                LOG4CXX_INFO(Utils::getLogger(), "Configure completed for hardware " + hardware->getUniqueID());
            }
        }

        for(int i=0; i<node->nhardware; i++){
            close(fd[i]);
            fd[i] = 0;
        }

        LOG4CXX_INFO(Utils::getLogger(), "Configure  completed for node");
    }
}
