/**
 *
 * @file ioctl.c
 * @author Bernardo Carvalho
 * @date 2014-06-27
 * @brief .
 *
 *
 * Copyright 2017 IPFN-Instituto Superior Tecnico, Portugal
 *
 * Licensed under the EUPL, Version 1.2 or - as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

#include "EventSem.h"

#include <vector>
//#include <map>
//#include <string>
#include <iostream>
#include <cmath>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <linux/types.h>

#include "PcieTimingNode.h"
#include "PcieTimingParameter.h"
#include "FSHardware.h"
#include "FSParameter.h"
#include "TimeStampWrapper.h"
#include "EventWrapper.h"
#include "FSignalUtils.h"
#include "trunk/include/kx1-pcie-timing-ioctl.h"


using namespace std;
using namespace fsignal;
using namespace
org::cfn::scad::core::corba;

struct DataProducerInit
{
    PcieTimingNode *node;
    FSHardware *hardware;
    //chid      * pv_id;
};

const string PcieTimingNode::DEV_FILE_LOCATIONS_ID = "DevFileLocations";
//const char * PcieTimingNode::PV_VVESSEL_TEMP = "ISTTOK:temperature:VVessel-Temperature";
const char * PcieTimingNode::PV_COUNTDOWN = "ISTTOK:central:COUNTDOWN";

//const string PcieTimingNode::FS_BUFFER_SEND_SIZE = "BufferSendSize";

PcieTimingNode::PcieTimingNode (const char *propsLoc, wstring mainEventID, const char *logFileLoc):
    FSNode (propsLoc, logFileLoc), mainEventID (mainEventID)
{

    EventWrapper ew;
    struct DataProducerInit *dpis = (struct DataProducerInit *) malloc (sizeof (struct DataProducerInit) * 256);
    aborted = false;
    //Check the location of the device ids
    Utils::tokenize (props.getValue (DEV_FILE_LOCATIONS_ID), devIds);

    /*bufferSendSize = 0x200000;
    if (props.getValue (FS_BUFFER_SEND_SIZE) != "")
    {
        bufferSendSize = atoi (props.getValue (FS_BUFFER_SEND_SIZE).c_str ());
        if (bufferSendSize < 1)
        {
            bufferSendSize = 0x200000;
        }
    }
    logStream.str ("");
    logStream << "The buffer send size is: " << bufferSendSize;
    LOG4CXX_INFO (Utils::getLogger (), logStream.str ());*/

    vector < HardwareXml > hwXmls = getParsedHardwareXmls ();
    if (hwXmls.size () != devIds.size ())
    {
        logStream.str ("");
        logStream << "The number of device ids(" << devIds.
            size () << ") is different from the number of hardware xmls(" <<
            hwXmls.size () << ") ABORTING!";
        LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
        exit (-1);
    }

    //Check if all the devIds look ok
    vector < bool > devStatus (devIds.size ());
    for (unsigned int i = 0; i < devIds.size (); i++)
    {
        int fd = open (devIds[i].c_str (), O_RDWR);
        if (fd < 0)
        {
            logStream.str ("");
            logStream << "Failed to open device at:" << devIds[i] <<
                " This is a very serious error!";
            LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
            devStatus[i] = false;
            //exit(-1);
        }
        else
        {
            devStatus[i] = true;
        }
        close (fd);
    }

    nhardware = 0;
    for (unsigned int i = 0; i < hwXmls.size (); i++)
    {
        FSHardware *hardware =
            new FSHardware (hwXmls[i].getUniqueID (), hwXmls[i].getFields ());
        vector < string > parXmls = hwXmls[i].getParametersUniqueIDS ();
        for (unsigned int j = 0; j < parXmls.size (); j++)
        {
            PcieTimingParameter *parameter =
                new PcieTimingParameter (parXmls[j], j + 1);
            hardware->addParameter (parameter);
            parameter->devLocation = devIds[i];
        }
        dpis[i].node = this;
        dpis[i].hardware = hardware;
        nhardware++;
        addHardware (hardware);

        if (devStatus[i] == false)
        {
            hardware->changeStatus (hardware::ERROR_STATE);
            cout << "SEVERE: Failed at opening device for Hardware " <<
                hardware->getUniqueID () << endl;

        }
    }

    int status = ca_create_channel(PV_COUNTDOWN,NULL,NULL,10,&pv_countdwn_id);
    cout << "ca_create " << PV_COUNTDOWN << " status " << status << endl;
    status = ca_pend_io(1.0);
    if(status |= 1L){
        cout << "ca_pend_io " << PV_COUNTDOWN << " status: " << status << endl;
    }
    status = ca_put(DBR_STRING, pv_countdwn_id, "193");
    ca_flush_io();
    //ca_clear_channel(pv_countdwn_id);
    //cout << "ca_put " << PV_COUNTDOWN << " status " << status << endl;
    //dpis[0].pv_id = &pv_countdwn_id;

    connectToServer ();
    loadSavedValues ();

    //Create all the threads and synch mechanism
    evtSem.Create ();

    //Create all the producers
    //for a driver limitation only a parameter can be acquired at a time...
    pthread_t *threads = (pthread_t *) malloc (sizeof (pthread_t));
    pthread_create (&threads[0], NULL, dataProducer, (void *) dpis);

    //Time Correction
    struct timespec timeNow;
    clock_gettime(CLOCK_REALTIME, &timeNow);
    TimeStampWrapper tsw = TimeStampWrapper(&(timeNow.tv_sec));
    timeCorr = timeNow.tv_sec-tsw.getTimeInSeconds();
}

void PcieTimingNode::newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID,
        const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend) {
}

void PcieTimingNode::abort(){
    logStream.str("");
    logStream << "Acquisition abort request!";
    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
    aborted = true;
}

void PcieTimingNode::setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList) {
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

void *PcieTimingNode::dataProducer(void* args) {
    struct DataProducerInit  *dpi         = (DataProducerInit *)args;
    PcieTimingNode          *node        = dpi[0].node;
    PcieTimingParameter     *parameter   = NULL;
    FSHardware               *hardware    = NULL;
    TimeStampWrapper          tstart;
    TimeStampWrapper          tend;
    EventWrapper              mainEvt;
    vector<EventWrapper>      eventList;
    //unsigned char            *buffer;
    stringstream              logStream;
    wchar_t *hwUID = NULL;

    vector<int>               fd;

    int                       ret         = 0;
    unsigned int              uval;
    //chid        *ppvid       = dpi[0].pv_id;

    chid        pv_cd_id; // epics Countdown PV */
    TIMING_CHANNEL tRegs;

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
        struct timeval tval;
        gettimeofday(&tval,  NULL);
        long long timeToShot = mainEvt.getEventTime().getTimeInMicros()-(long long)((tval.tv_sec-(node->timeCorr))*1000000L+tval.tv_usec);
        logStream.str("");
        logStream << "Time to shot: " << timeToShot << "us; Shot time: " << mainEvt.getEventTime().getTimeInMicros() << "; Time Correction: " << node->timeCorr;
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());

        /*Reset channels*/
        tRegs.evnt_regs.delay= 0;
        tRegs.evnt_regs.period= 0;
        tRegs.evnt_regs.config= 0;

        //open devices and check ioctl
        for(int i=0; i<node->nhardware; i++){
            hardware = dpi[i].hardware;
            if(!hardware->isConnected()){
                continue;
            }
            //           device = i;
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
            gettimeofday(&tval,  NULL);

            ret = ::ioctl(fd[i], KX1_PCIE_IOCG_MOD_STATUS, &uval );
            if(ret < 0) {
                logStream.str("");
                logStream << "While getting Status, error ioctling the device, ret= " << ret << ", status=" << uval <<  ", " << node->devIds[i];
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            }
            else{
                logStream.str("");
                logStream << "gettimeofday()= " << tval.tv_sec << " sec "<< tval.tv_usec<<" usec" << endl;
                logStream << "HW Status REG 0x" << hex << (int)  uval << dec << endl;
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());
            }
        /*Stop Board Timers*/
            uval =0;
            ret = ioctl(fd[i], KX1_PCIE_IOCS_TIMNG_CNTRL, &uval);

            //cout << "Hardware ID 1" << hardware->getUniqueID() << " in State: " << node->getHardwareStatus(hwUID) << endl;
        }

        tstart     =   mainEvt.getEventTime();
       /* int tmeS = tstart.getTimeInSeconds();
        long long tmemS =tstart.getTimeInMillis();
        long long tmeuS =tstart.getTimeInMicros();
        long long tmenS =tstart.getTimeInNanos();
        logStream.str("");
        logStream << "tstart sec=  " << tmeS <<  " tstart ms =  " << tmemS << " tstart us =  " << tmeuS << " tstart ns =  " << tmenS;
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());*/

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
            //Check how many channels (Parameters) are connected for this hardware
            std::vector<FSParameter*> hwParameters = hardware->getAllParameters();
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (PcieTimingParameter *)hwParameters[j];
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

            //unsigned int bufferToSendSize = 0;
            //buffer = (unsigned char *)malloc(node->bufferSendSize * 2);
            /*Check transposed parameters on memset*/
            //memset(buffer, 0, node->bufferSendSize);
            for(unsigned int j=0; j < hwParameters.size(); j++){
                parameter = (PcieTimingParameter*)hwParameters[j];
                if(!parameter->isConnected()){
                    continue;
                }
                /*Reset channel*/
                tRegs.evnt_regs.delay= 0;
                tRegs.evnt_regs.period= 0;
                if (parameter->getChannelInvert() == PcieTimingParameter::CHANNEL_INVERT_YES)
                    tRegs.evnt_regs.config = 0x80000000;
                else
                    tRegs.evnt_regs.config = 0;
                tRegs.channel=j;
                ret = ::ioctl(fd[i], KX1_PCIE_IOCS_TIMER_REGS, &tRegs);

                std::string parameterXML = parameter->getXMLConfiguration();

                tstart     = mainEvt.getEventTime();

                tend       = tstart;
                //tRegs.channel = j;
                long int tmp = (long) (parameter->getDelayTime100nS()+timeToShot*10);
                if (tmp > 0) {
                    uval = (unsigned int)tmp;
                }
                else {
                    uval = 1;
                    logStream.str("");
                    logStream << "The absolute delay for " << parameter->getUniqueID() << " is zero or negative (" << tmp << "). Using 100 ns.";
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                }
                tRegs.evnt_regs.delay = uval;
                uval = (unsigned int) parameter->getPeriodTime100nS();
                tRegs.evnt_regs.period =  uval;
                tRegs.evnt_regs.config = 0x00000000;
                //tRegs.evnt_regs.config=   0x80000000; // Invert output reset state
                //
                if (parameter->getChannelMode() == PcieTimingParameter::CHANNEL_MODE_RESET)
                    tRegs.evnt_regs.config |=   0x0; // do nothing
                else if (parameter->getChannelMode() == PcieTimingParameter::CHANNEL_MODE_TRIGGER)
                    tRegs.evnt_regs.config |= 0x10000000;
                else if (parameter->getChannelMode() == PcieTimingParameter::CHANNEL_MODE_SQUARE)
                    tRegs.evnt_regs.config |= 0x20000000;
                else if (parameter->getChannelMode() == PcieTimingParameter::CHANNEL_MODE_PULSE_TRAIN){
                    tRegs.evnt_regs.config |= 0x30000000;
                    uval = (unsigned int) parameter->getNumPulses();
                    tRegs.evnt_regs.config |= (0x3FFFFFF & uval); // Use only 26 LSB
                }
                if (parameter->getChannelInvert() == PcieTimingParameter::CHANNEL_INVERT_YES)
                    tRegs.evnt_regs.config |= 0x80000000;
                logStream.str("");
                logStream << "Starting configure channel " << j << ", Delay: " << tRegs.evnt_regs.delay;
                logStream <<  ", Period: " << tRegs.evnt_regs.period;
                logStream << ", Config:0x" << hex << tRegs.evnt_regs.config << dec << endl;
                LOG4CXX_INFO(Utils::getLogger(), logStream.str());

                ret = ::ioctl(fd[i], KX1_PCIE_IOCS_TIMER_REGS, &tRegs);
                if(ret < 0){
                    logStream.str("");
                    logStream << "While setting Timer regs, error ioctling the device ret " << ret << ", dev "<< node->devIds[i];
                    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                }
//                hwUID = Utils::widenCorba(hardware->getUniqueID());
//                cout << "Hardware ID 1" << hardware->getUniqueID() << " in State: " << node->getHardwareStatus(hwUID) << endl;

                eventList[0].setEventID(-1);
                /*node->sendNewDataAvailable(
                        hardware->getUniqueID(),
                        parameter->getUniqueID(),
                        tstart,
                        tend,
                        eventList,
                        parameterXML,
                        buffer,
                        bufferToSendSize
                        );*/
            }
            ret = ::ioctl(fd[i], KX1_PCIE_IOCG_TIMNG_COUNT, &uval);
            cout << "Hardware ID 1 Timer Count (before trigger):" << uval << endl;
            /*Start Board Common Time Counter (Trigger Board)*/
            uval =0x1;
            ret = ::ioctl(fd[i], KX1_PCIE_IOCS_TIMNG_CNTRL, &uval);
            hwUID = Utils::widenCorba(hardware->getUniqueID());
            node->getHardwareStatus(hwUID);
            cout << "Hardware ID " << hardware->getUniqueID() << " in State: " << node->getHardwareStatus(hwUID) << endl;
            //cout << "Hardware ID " << hardware->getUniqueID() << " in State: " << hwUID << endl;
            //free(buffer);

        }

        //Wait for the queue to be empty before changing the state again
        while(!node->sendQueue.empty()){
            sleep(1);
        }
        // really need this for ?
        for(int i=0; i<node->nhardware; i++){
            hardware = dpi[i].hardware;
            ret = ::ioctl(fd[i], KX1_PCIE_IOCG_TIMNG_COUNT, &uval);
            cout << "Hardware ID 1 Timer Count(after sleep(1)):" << uval << endl;
            if(!hardware->isConnected()){
                continue;
            }
            LOG4CXX_INFO(Utils::getLogger(), "Configure completed for hardware " + hardware->getUniqueID());
        }
        cout << "Hardware ID (before close)" << hardware->getUniqueID() << " in State: " << node->getHardwareStatus(hwUID) << endl;

        for(int i=0; i<node->nhardware; i++){
            close(fd[i]);
            fd[i] = 0;
        }
        //SEVCHK(ca_context_create(ca_disable_preemptive_callback),"ca_context_create");
        //cout << "ca_create " << PV_VVESSEL_TEMP << " status " << status << endl;
        //status = ca_pend_io(5.0);
        int status = ca_create_channel(PV_COUNTDOWN,NULL,NULL,10,&pv_cd_id);
        //CA status, 1 = OK
        cout << "ca_create " << PV_COUNTDOWN << " status " << status << endl;
        status = ca_pend_io(1.0);
        cout << "ca_pend_io " << PV_COUNTDOWN << " status " << status << endl;
        //status = ca_put(DBR_STRING, pv_cd_id, "180");
        //status = ca_put(DBR_LONG, pv_cd_id, 180);
        //status = ca_put(DBR_STRING, *ppvid, "180");
        //SEVCHK(ca_put(DBR_STRING, pv_countdwn_id, "180"), "Put failed");
        cout << "Caput (after close) return: " << status << endl;
        ca_flush_io();
        //ca_clear_channel(pv_cd_id);

        LOG4CXX_INFO(Utils::getLogger(), "Configure completed for node");
    }
}
void PcieTimingNode::changeHardwareStatus (const CORBA::WChar * hardwareUniqueID,
        fscns::hardware::
        HardwareStatus newStatus)
{
    if (getHardwareStatus (hardwareUniqueID) == hardware::ERROR_STATE)
    {
        //cout << "Trying to change state of " << hardwareUniqueID << " but is already in ERROR" << endl;
        return;
    }

    FSNode::changeHardwareStatus (hardwareUniqueID, newStatus);
}
