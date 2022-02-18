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
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#include "SRTRATCANode.h"
#include "FSHardware.h"
#include "FSParameter.h"
#include "TimeStampWrapper.h"
#include "EventWrapper.h"
#include "FSignalUtils.h"
#include "SRTRATCAParameter.h"
#include "pcieSrTr_ioctl.h"

using namespace std;
using namespace fsignal;
using namespace org::cfn::scad::core::corba;

struct DataProducerInit{
    SRTRATCANode *node;
    FSHardware   *hardware;
};

const string SRTRATCANode::DEV_FILE_LOCATIONS_ID = "DevFileLocations";
const string SRTRATCANode::FS_BUFFER_SEND_SIZE   = "BufferSendSize";
const string SRTRATCANode::TRIGGER_EVT_IDs       = "AcceptedTriggerEventIDs";
const string SRTRATCANode::END_OF_PULSE_EVT_ID   = "EndOfPulseEventID";

SRTRATCANode::SRTRATCANode(const char* propsLoc, const char *logFileLoc) : FSNode(propsLoc, logFileLoc) {
    
    EventWrapper ew;       
    struct DataProducerInit *dpis = (struct DataProducerInit *)malloc(sizeof(struct DataProducerInit) * 256);
    aborted = false;
    devIds.clear();
    //Check the location of the device ids
    Utils::tokenize(props.getValue(DEV_FILE_LOCATIONS_ID), devIds);

    vector<string> tmp;
    Utils::tokenize(props.getValue(TRIGGER_EVT_IDs), tmp);
    if(tmp.size() < 1){
        logStream.str("At least a trigger event id must be specified, e.g. 0x0000");
        LOG4CXX_INFO(Utils::getLogger(), logStream.str()); 
        exit(-1);
    }
    for(unsigned int c=0; c<tmp.size(); c++){
        wstring wtmp(tmp[c].begin(), tmp[c].end());
        eventTriggerIDs.push_back(wtmp);
    }

    string tmp2 = props.getValue(END_OF_PULSE_EVT_ID);
    if(tmp.size() > 0){
        wstring wtmp2 = wstring(tmp2.begin(), tmp2.end());
        eventEndOfPulseID = wtmp2;
    }
    
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
    
    nhardware   = 0;
    for(unsigned int i=0; i<hwXmls.size(); i++) {
        FSHardware *hardware = new FSHardware(hwXmls[i].getUniqueID(), hwXmls[i].getFields());
        vector<string> parXmls = hwXmls[i].getParametersUniqueIDS();
        for(unsigned int j=0; j<parXmls.size(); j++) {
            SRTRATCAParameter *parameter = new SRTRATCAParameter(parXmls[j], j+1);                        
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
    
    //Check if all the devIds look ok
    for(unsigned int i=0; i<devIds.size(); i++) {
        int fd = open(devIds[i].c_str(), O_RDWR);
        if(fd < 0){
            logStream.str("");
            logStream << "Failed to open device at:" << devIds[i] << " This is a very serious error!";
            LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            //exit(-1);
            wchar_t *hwUID = Utils::widenCorba(dpis[i].hardware->getUniqueID());
            changeHardwareStatus(hwUID, hardware::ERROR_STATE);
            CORBA::wstring_free(hwUID);
        }
        close(fd);
    }    
}

void SRTRATCANode::newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend) {    
}

void SRTRATCANode::abort(){
    logStream.str("");
    logStream << "Acquisition abort request!";
    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
    aborted = true;
}

void SRTRATCANode::setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList) {
    wchar_t *evtList_uid = NULL;
    bool     found       = false;
    for(unsigned int i=0; i<evtList.length() && !found; i++) {
        evtList_uid = CORBA::wstring_dup(evtList[i].nameUniqueID);
        for(unsigned int j=0; j<eventTriggerIDs.size(); j++){
            if(evtList_uid == eventTriggerIDs[j]) {
                mainEvent = evtList[i];
                setNodeStatus(hardware::OPERATIONAL);
                changeAllHardwareStatus(hardware::RUNNING);
	        setNodeStatus(hardware::NRUNNING);
                aborted         = false;
                pulseTerminated = false;
                found           = true;
                evtSem.Post();
            }
            else if(evtList_uid == eventEndOfPulseID){
	        pulseTerminated = true;
                found           = true;
            }
        }
        CORBA::wstring_free(evtList_uid);
    }
}

void *SRTRATCANode::dataProducer(void* args) {
    struct DataProducerInit  *dpi = (DataProducerInit *)args;
    SRTRATCANode             *node      = dpi[0].node;
    SRTRATCAParameter        *parameter = NULL;
    FSHardware               *hardware  = NULL;
    TimeStampWrapper          tstart;
    TimeStampWrapper          tend;
    EventWrapper              mainEvt;
    vector<EventWrapper>      eventList;
    unsigned char            *buffer;
    stringstream              logStream;
    
    unsigned int              nBytes        = 0;
    int                       readBytes     = 0;    
    int                       fd            = 0;
    int                       ret           = 0;
    unsigned int              bufferSize    = 0;
    char                     *copyBuffer    = NULL;
    //This flag keeps track if the acquisition in a particular hardware device was terminated
    //When the requested number of bytes (or events) is written in the board, it issues an 
    //interrupt with acquisition completed, 
    bool                      acqComplete   = false;
    //If the acquisition was stopped due to the arrival of a user acquisition
    //stop instead of being terminated by the board (number of events programmed achieved, p.e.)
    bool                      exitedWithPulseTerminated = false;
    //local copy
    vector<string>            devIds      = node->devIds;
    
    while(true){
        node->setNodeStatus(hardware::STAND_BY);
        node->changeAllHardwareStatus(hardware::STOPPED);
        
        node->evtSem.ResetWait();                
        
        eventList.clear();
        //buffer.clear();
        mainEvt = node->mainEvent;
        eventList.push_back(mainEvt);
        
        int numberOfConnectedChannels = 0;
        //enable acquisition
        for(int i=0; i<node->nhardware; i++){
            numberOfConnectedChannels = 0;
            hardware = dpi[i].hardware;
            if(!hardware->isConnected()){
                continue;
            }
            //Get the number of connected channels
            std::vector<FSParameter*> hwParameters = hardware->getAllParameters();
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (SRTRATCAParameter *)hwParameters[j];
                if(!parameter->isConnected()){
                    continue;
                }
                numberOfConnectedChannels++;
            }
            fd = open(devIds[i].c_str(), O_RDWR);
            if(fd < 0){
                logStream.str("");
                logStream << "While starting acquisition, error opening the device " << devIds[i].c_str();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            }
            //patch the number of acq bytes based on the numberOfConnectedChannels
            if(numberOfConnectedChannels > 0){
                unsigned int acqBytes = parameter->getAcqByteSize();
                while(((acqBytes / numberOfConnectedChannels) % parameter->getBufferReadSize()) != 0){
                    acqBytes++;
                }
                if(acqBytes > 0x7FFFFFFF){
                    acqBytes = 0x7FFFFFFF;
                }
                ret = ioctl(fd, PCIE_SRTR_IOCS_ACQBYTESIZE, &acqBytes);
                if(ret < 0){
                    logStream.str("");
                    logStream << "While setting the acqBytes to: " << acqBytes << ", error ioctling the device " << devIds[i].c_str();
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                }
                logStream.str("");
                logStream << "Setting the acqBytes to: " << acqBytes;
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                for(unsigned int j=0; j < hwParameters.size(); j++){
                    parameter = (SRTRATCAParameter *)hwParameters[j];
                    if(parameter->isConnected()){
                        parameter->setPatchedAcqByteSize(acqBytes);
                    }
                }
            }
            //Reset the offset
            int offsetRst = 0;
            ret = ioctl(fd, PCIE_SRTR_IOCS_RDOFF, &offsetRst);
            if(ret < 0){
                logStream.str("");
                logStream << "While setting the offset to: " <<  offsetRst << ", error ioctling the device " << devIds[i].c_str();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                continue;
            }
            ret = ioctl(fd, PCIE_SRTR_IOCT_ACQ_ENABLE);
            if(ret < 0){
                logStream.str("");
                logStream << "While starting acquisition, error ioctling the device " << devIds[i].c_str();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                continue;
            }
            
            //trigger is static for this board. just task the first parameter
            if(hwParameters.size() > 0){
                parameter = (SRTRATCAParameter *)hwParameters[0];
                parameter->dumpConfiguration();
                if(parameter != NULL){
                    if(parameter->getTriggerType() == SRTRATCAParameter::TRG_TYPE_SOFTWARE){
                        ret = ioctl(fd, PCIE_SRTR_IOCT_SOFT_TRIG);
                        if(ret < 0){
                                logStream.str("");
                                logStream << "While sending the software trigger, error ioctling the device " << devIds[i].c_str();
                                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                        }
                        logStream.str("");
                        logStream << "Sent Software trigger for device " << devIds[i].c_str();
                        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                    }
                }
            }
            close(fd);            
        }
                
        //wait for acquisition completed
        int  nAcqCompleted = node->nhardware;
        int  acqC          = 0;
        int  waitCounter   = 0;

        acqComplete        = false;
        while(nAcqCompleted != 0 && !node->aborted && !node->pulseTerminated){
            nAcqCompleted = node->nhardware;
            for(int i=0; i<node->nhardware; i++){
                hardware  = dpi[i].hardware;
                if(!hardware->isConnected()){
                    nAcqCompleted--;
                    continue;
                }
                bool atLeastOneParameter = false;
                std::vector<FSParameter*> hwParameters = hardware->getAllParameters();
                for(unsigned c=0; c<hwParameters.size(); c++){
                    if(hwParameters[c]->isConnected()){
                    	atLeastOneParameter = true;
                        break;
                    }
                }
                if(!atLeastOneParameter){
                    nAcqCompleted--;
                    continue;
                }
                fd = open(devIds[i].c_str(), O_RDWR);
                if(fd < 0){
                    acqComplete = true;
                    continue;
                }

                ret = ioctl(fd, PCIE_SRTR_IOCG_ACQC, &acqC);
                //The new RT version does not use acqC
                acqC = 0;
                if(ret < 0){
                    acqComplete = true;
                    continue;
                }
                if((waitCounter % 60) == 0){
                    logStream.str("");
                    logStream << "acqC for " << devIds[i].c_str() << " = " << acqC;
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                }
                acqComplete = (ret >= 0) && (acqC > 0);
                if(acqComplete){
                    nAcqCompleted--;
                }
                close(fd);
                sleep(1);
                waitCounter++;
            }
        }
        exitedWithPulseTerminated = (nAcqCompleted != 0);       
 
        if(node->aborted){
            continue;
        }
        
        for(int i=0; i<node->nhardware; i++){
            hardware = dpi[i].hardware;
            if(!hardware->isConnected()){
                continue;
            }
            fd = open(devIds[i].c_str(), O_RDWR);
            if(fd < 0){
                logStream.str("");
                logStream << "While disabling acquisition acquisition, error opening the device " << devIds[i].c_str();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                acqComplete = true;
                continue;
            }
            ret = ioctl(fd, PCIE_SRTR_IOCT_ACQ_DISABLE);            
            if(ret < 0){
                logStream.str("");
                logStream << "While disabling acquisition acquisition, error ioctling the device " << devIds[i].c_str();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                acqComplete = true;
                continue;
            }
            close(fd);            
        }
        
        if(node->aborted){
            continue;
        }
        
        for(int i=0; i<node->nhardware; i++){
            numberOfConnectedChannels = 0;
            hardware  = dpi[i].hardware;
            if(!hardware->isConnected()){
                continue;
            }
            //Check how many channels are connected for this hardware           
            std::vector<FSParameter*> hwParameters = hardware->getAllParameters();
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (SRTRATCAParameter *)hwParameters[j];
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
            int offsetJump = 0x80000000 / numberOfConnectedChannels;
            int offset     = 0;            
            
            fd = open(devIds[i].c_str(), O_RDWR);
            if(fd < 0){
                logStream.str("");
                logStream << "While downloading the data, error opening the device " << devIds[i].c_str();
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                acqComplete = true;
                continue;                
            }

            unsigned int bufferToSendSize = 0;
            buffer = (unsigned char *)malloc(node->bufferSendSize * 2);
            memset(buffer, node->bufferSendSize, 0);
            int connectedParameterUnIndex = 0;
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (SRTRATCAParameter *)hwParameters[j];
                if(!parameter->isConnected()){
                    continue;
                }
                if(parameter->getAcqMode() == SRTRATCAParameter::ACQ_MODE_RAW){
                    nBytes = parameter->getPatchedAcqByteSize() / numberOfConnectedChannels;
                }
                else{
                    logStream.str("");
                    logStream << "Temporary patch while the firmware can not distinguish events between different channels";
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                    nBytes = parameter->getPatchedAcqByteSize() / numberOfConnectedChannels;
                   // nBytes = parameter->getNumberOfEventsInBytes();
                }
                if(nBytes < 0){
                    logStream.str("");
                    logStream << "The parameter " << parameter->getUniqueID() << " was connected but no data was acquired";
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                    //Send just one byte for storage
                    bufferToSendSize = 1;
                }
                if(j > 0 && numberOfConnectedChannels == 3){
                    if(j == 1){
                        offset = 0x20000000;
                    }
                    else if(j == 2){
                        offset = 0x40000000;
                    }
                }
                else{
                    offset = connectedParameterUnIndex * offsetJump;
                }
                connectedParameterUnIndex++;

                ret = ioctl(fd, PCIE_SRTR_IOCS_RDOFF, &offset);                
                if(ret < 0){
                    logStream.str("");
                    logStream << "While setting the offset to: " <<  offset << ", error ioctling the device " << devIds[i].c_str();
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                    continue;
                }

                readBytes = 0;
                bufferSize = parameter->getBufferReadSize();
                copyBuffer = (char *)malloc(bufferSize);                
                tstart     = mainEvt.getEventTime();                
                tstart.addMicroSeconds(parameter->getPreTriggerTime());
                
                tend       = tstart;                

                logStream.str("");
                logStream << "The buffer size is " << bufferSize;
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());

                logStream.str("");
                logStream << "Number of bytes requested is " << nBytes;            
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                
                LOG4CXX_INFO(Utils::getLogger(), "Starting acquisition for " + parameter->getUniqueID());
                std::string parameterXML = parameter->getXMLConfiguration();
                readBytes = 0;
                                
                while(nBytes > 0){
                    if(nBytes < bufferSize){
                        bufferSize = nBytes;
                    }
                    
                    readBytes = read(fd, copyBuffer, bufferSize);
                    if(readBytes < 1){
                        logStream.str("");
                        logStream << "Error! Read bytes returned " << readBytes << " The number of bytes still to read are " << nBytes;;
                        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                        break;
                    }
                    nBytes -= readBytes;
                    if(readBytes > 0){
                        memcpy(buffer + bufferToSendSize, copyBuffer, readBytes);
                        bufferToSendSize+=readBytes;
                    }

                    if((bufferToSendSize > node->bufferSendSize)){
                        tend.addNanoSeconds((int)(bufferToSendSize * 1e9 / (double)(parameter->getFrequency() * 1e6)));
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
                        memset(buffer, bufferToSendSize, 0);
                        bufferToSendSize = 0;                        
                        tstart = tend;
                    }                                
                }

                if(bufferToSendSize > 0){
                    tend.addNanoSeconds((int)(bufferToSendSize * 1e9 / (double)(parameter->getFrequency() * 1e6)));
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
                    memset(buffer, bufferToSendSize, 0);
                    bufferToSendSize = 0;
                }

                free(copyBuffer);                
            }
            
            free(buffer);
            close(fd);
            
            //Wait for the queue to be empty before changing the state again
            while(!node->sendQueue.empty()){
                sleep(1);
            }
            LOG4CXX_INFO(Utils::getLogger(), "Acquisition completed for hardware " + hardware->getUniqueID());
        }
        //If exited with EJP, allow some time for data flushing into the FS DB
        if(exitedWithPulseTerminated){
            sleep(30);
        }
        LOG4CXX_INFO(Utils::getLogger(), "Acquisition completed for node");
    }    
}
