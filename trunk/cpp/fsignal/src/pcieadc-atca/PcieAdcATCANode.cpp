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

#include "PcieAdcATCANode.h"
#include "PcieAdcATCAParameter.h"
#include "FSHardware.h"
#include "FSParameter.h"
#include "TimeStampWrapper.h"
#include "EventWrapper.h"
#include "FSignalUtils.h"
#include "pcieAdc_ioctl.h"
//#include "pcieAdc.h"

using namespace std;
using namespace fsignal;
using namespace
    org::cfn::scad::core::corba;

struct DataProducerInit
{
    PcieAdcATCANode *
        node;
    FSHardware *
        hardware;
};

const
    string
    PcieAdcATCANode::DEV_FILE_LOCATIONS_ID = "DevFileLocations";
const
    string
    PcieAdcATCANode::FS_BUFFER_SEND_SIZE = "BufferSendSize";

PcieAdcATCANode::PcieAdcATCANode (const char *propsLoc, wstring mainEventID,
                                  const char *logFileLoc):
FSNode (propsLoc, logFileLoc),
mainEventID (mainEventID)
{

    EventWrapper ew;
    struct DataProducerInit *dpis =
        (struct DataProducerInit *) malloc (sizeof (struct DataProducerInit) *
                                            256);
    aborted = false;
    //Check the location of the device ids
    Utils::tokenize (props.getValue (DEV_FILE_LOCATIONS_ID), devIds);

    bufferSendSize = 0x200000;
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
    LOG4CXX_INFO (Utils::getLogger (), logStream.str ());

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
            PcieAdcATCAParameter *parameter =
                new PcieAdcATCAParameter (parXmls[j], j + 1);
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

    connectToServer ();
    loadSavedValues ();

    //Create all the threads and synch mechanism
    evtSem.Create ();

    //Create all the producers
    //for a driver limitation only a parameter can be acquired at a time...
    pthread_t *threads = (pthread_t *) malloc (sizeof (pthread_t));
    pthread_create (&threads[0], NULL, dataProducer, (void *) dpis);

}

void
PcieAdcATCANode::newDataAvailable (const CORBA::WChar * nodeUniqueID,
                                   const CORBA::WChar * hardwareUniqueID,
                                   const CORBA::WChar * parameterUniqueID,
                                   const fscns::hardware::EventList & eList,
                                   const fscns::util::TimeStamp & tstart,
                                   const fscns::util::TimeStamp & tend)
{
}

void
PcieAdcATCANode::abort ()
{
    logStream.str ("");
    logStream << "Acquisition abort request!";
    LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
    aborted = true;
}

void
PcieAdcATCANode::setEventTable (const org::cfn::scad::core::corba::hardware::
                                EventList & evtList)
{
    wchar_t *evtList_uid = NULL;
    for (unsigned int i = 0; i < evtList.length (); i++)
    {
        evtList_uid = CORBA::wstring_dup (evtList[i].nameUniqueID);
        if (evtList_uid == mainEventID)
        {
            mainEvent = evtList[i];
            setNodeStatus (hardware::OPERATIONAL);
            changeAllHardwareStatus (hardware::RUNNING);
            setNodeStatus (hardware::NRUNNING);
            aborted = false;
            evtSem.Post ();
        }
        CORBA::wstring_free (evtList_uid);
    }
}

void *
PcieAdcATCANode::dataProducer (void *args)
{
    struct DataProducerInit *dpi = (DataProducerInit *) args;
    PcieAdcATCANode *node = dpi[0].node;
    PcieAdcATCAParameter *parameter = NULL;
    FSHardware *hardware = NULL;
    TimeStampWrapper tstart;
    TimeStampWrapper tend;
    EventWrapper mainEvt;
    vector < EventWrapper > eventList;
    unsigned char *buffer;
    stringstream logStream;
    wchar_t *hwUID = NULL;

    vector < int >fd;
    unsigned int nBytes = 0;
    int readBytes = 0;
    //    int                       device      = 0;
    int ret = 0;
    unsigned int bufferSize = 0;
    unsigned char *copyBuffer = NULL;
    bool acqComplete = false;

    for (int i = 0; i < node->nhardware; i++)
    {
        fd.push_back (0);
    }

    while (true)
    {
        node->setNodeStatus (hardware::STAND_BY);
        node->changeAllHardwareStatus (hardware::STOPPED);

        node->evtSem.ResetWait ();

        eventList.clear ();
        //buffer.clear();
        mainEvt = node->mainEvent;
        eventList.push_back (mainEvt);
        int masterBoardId = -1;

        //enable acquisition
        for (int i = 0; i < node->nhardware; i++)
        {
            hardware = dpi[i].hardware;
            if (!hardware->isConnected ())
            {
                continue;
            }
            //device = i;
            fd[i] = open (node->devIds[i].c_str (), O_RDONLY);
            if (fd[i] < 0)
            {
                logStream.str ("");
                logStream <<
                    "While starting acquisition, error opening the device " <<
                    node->devIds[i];
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                hwUID = Utils::widenCorba (hardware->getUniqueID ());
                node->changeHardwareStatus (hwUID, hardware::ERROR_STATE);
            }

            //trigger is static for this board. just task the first parameter
            std::vector < FSParameter * >hwParameters =
                hardware->getAllParameters ();
            if (hwParameters.size () > 0)
            {
                parameter = (PcieAdcATCAParameter *) hwParameters[0];
                if (parameter != NULL)
                {
                    unsigned int delay = parameter->getTrigDelayMicros ();
                    logStream.str ("");
                    logStream << "Delay for " << node->
                        devIds[i] << ':' << delay;
                    LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                    ret = ioctl (fd[i], PCIE_ADC_IOCS_TRIG_DLY, &delay);
                    if (ret < 0)
                    {
                        logStream.str ("");
                        logStream <<
                            "While setting trig delay acquisition, error ioctling the device "
                            << node->devIds[i];
                        LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                        hwUID = Utils::widenCorba (hardware->getUniqueID ());
                        node->changeHardwareStatus (hwUID,
                                                    hardware::ERROR_STATE);

                    }
                }
            }
            ret = ioctl (fd[i], PCIE_ADC_IOCT_INT_ENABLE);
            if (ret < 0)
            {
                logStream.str ("");
                logStream <<
                    "While enabling the interrupts, error ioctling the device "
                    << node->devIds[i];
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
            }
            ret = ioctl (fd[i], PCIE_ADC_IOCT_ACQ_ENABLE);
            if (ret < 0)
            {
                logStream.str ("");
                logStream <<
                    "While enabling acquisition, error ioctling the device "
                    << node->devIds[i];
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                hwUID = Utils::widenCorba (hardware->getUniqueID ());
                node->changeHardwareStatus (hwUID, hardware::ERROR_STATE);

            }
            //Find Master Board Id
            unsigned int mster;
            ret = ioctl (fd[i], PCIE_ADC_IOCG_STATUS, &mster);
            if (mster & 0x00400000)
                masterBoardId = i;

        }
        if (masterBoardId != -1)
        {                       // Master Board Present
            // Send Tyigger SOFT TRIGGER to Master Board
            hardware = dpi[masterBoardId].hardware;
            std::vector < FSParameter * >hwParameters =
                hardware->getAllParameters ();
            if (hwParameters.size () > 0)
            {
                //trigger is static for this board. just task the first parameter
                parameter = (PcieAdcATCAParameter *) hwParameters[0];
                if (parameter != NULL)
                {
                    if (parameter->getTriggerType () ==
                        PcieAdcATCAParameter::TRG_TYPE_SOFTWARE)
                    {
                        logStream.str ("");
                        logStream <<
                            "Sending software trigger to Master device " <<
                            node->devIds[masterBoardId];
                        LOG4CXX_INFO (Utils::getLogger (), logStream.str ());

                        if (ret < 0)
                        {
                            logStream.str ("");
                            logStream <<
                                "While sending the software trigger, error ioctling the device "
                                << node->devIds[masterBoardId];
                            LOG4CXX_INFO (Utils::getLogger (),
                                          logStream.str ());
                        }
                    }
                }
            }
        }
        else
        {                       // Master NOT present. Send Soft trigger to all slaves
            for (int i = 0; i < node->nhardware; i++)
            {
                hardware = dpi[i].hardware;
                if (!hardware->isConnected ())
                {
                    continue;
                }
                std::vector < FSParameter * >hwParameters =
                    hardware->getAllParameters ();
                if (hwParameters.size () > 0)
                {
                    //trigger is static for this board. just task the first parameter
                    parameter = (PcieAdcATCAParameter *) hwParameters[0];
                    if (parameter != NULL)
                    {
                        if (parameter->getTriggerType () ==
                            PcieAdcATCAParameter::TRG_TYPE_SOFTWARE)
                        {

                            logStream.str ("");
                            logStream <<
                                "Sending software trigger to Slave device " <<
                                node->devIds[i];
                            LOG4CXX_INFO (Utils::getLogger (),
                                          logStream.str ());
                            ret = ioctl (fd[i], PCIE_ADC_IOCT_SOFT_TRIG);
                            if (ret < 0)
                            {
                                logStream.str ("");
                                logStream <<
                                    "While sending the software trigger, error ioctling the device "
                                    << node->devIds[i];
                                LOG4CXX_INFO (Utils::getLogger (),
                                              logStream.str ());
                                hwUID =
                                    Utils::widenCorba (hardware->
                                                       getUniqueID ());
                                node->changeHardwareStatus (hwUID,
                                                            hardware::
                                                            ERROR_STATE);

                            }
                        }
                    }
                }
            }
        }
        /**
	// Send Tyigger SOFT TRIGGER to Master Board
	for(int i=0; i<node->nhardware; i++){
	  hardware  = dpi[i].hardware;
	  if(!hardware->isConnected()){
	    continue;
	  }
	  //trigger is static for this board. just task the first parameter
	  std::vector<FSParameter*> hwParameters = hardware->getAllParameters();
	  if(hwParameters.size() > 0){
	    parameter = (PcieAdcATCAParameter *)hwParameters[0];
	    if(parameter != NULL){
	      unsigned int mster;
	      ret = ioctl(fd[i], PCIE_ADC_IOCG_STATUS, &mster);
	      if (mster & 0x00400000){
		if(parameter->getTriggerType() == PcieAdcATCAParameter::TRG_TYPE_SOFTWARE){
		      
		  logStream.str("");
		  logStream << "Sending software trigger to Master device " << node->devIds[i];
		  LOG4CXX_INFO(Utils::getLogger(), logStream.str());
		  ret = ioctl(fd[i], PCIE_ADC_IOCT_SOFT_TRIG);                        
		  if(ret < 0){
		    logStream.str("");
		    logStream << "While sending the software trigger, error ioctling the device " << node->devIds[i];
		    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
		  }
		}
	      }
	    }
	  }
	}  
	*/
        //wait for acquisition completed
        int nAcqCompleted = node->nhardware;
        acqComplete = false;
        unsigned int acqC = 0;
        while (nAcqCompleted > 0 && !node->aborted)
        {
            sleep(1);
            //nAcqCompleted = node->nhardware;
            for (int i = 0; i < node->nhardware; i++)
            {
                hardware = dpi[i].hardware;
                if (!hardware->isConnected ())
                {
                    nAcqCompleted--;
                    continue;
                }
                ret = ioctl (fd[i], PCIE_ADC_IOCG_STATUS, &acqC);
                if (ret < 0)
                {
                    logStream.str ("");
                    logStream <<
                        "While waiting for acquisition complete, error ioctling the device "
                        << node->devIds[i];
                    LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                    //acqComplete = true;
                    nAcqCompleted--;
                    hwUID = Utils::widenCorba (hardware->getUniqueID ());
                    node->changeHardwareStatus (hwUID, hardware::ERROR_STATE);

                    continue;
                }
                if (acqC & 0x80000000)
                    acqComplete = true; //TODO ask bbc how do I know acq completed? (ret >= 0) && (acqC > 0);                
                else
                    acqComplete = false;

                if (acqComplete)
                {
                    nAcqCompleted--;
                }
                //DEBUG CODE
                /**
		logStream.str("");
		logStream << "HW " <<i <<", acqc=0x " << std::hex << acqC << std::dec << ", nA"<< nAcqCompleted;
		LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                sleep(1);
		*/
            }
        }
        if (node->aborted)
        {
            continue;
        }

        int numberOfConnectedChannels = 0;
        int *tempSwap;
        for (int i = 0; i < node->nhardware; i++)
        {
            numberOfConnectedChannels = 0;
            hardware = dpi[i].hardware;
            if (!hardware->isConnected ()
                || hardware->getStatus () == hardware::ERROR_STATE)
            {
                continue;
            }
            //Check how many channels are connected for this hardware           
            std::vector < FSParameter * >hwParameters =
                hardware->getAllParameters ();
            for (unsigned int j = 0; j < hwParameters.size (); j++)
            {
                parameter = (PcieAdcATCAParameter *) hwParameters[j];
                if (parameter->isConnected ())
                {
                    numberOfConnectedChannels++;
                }
            }
            int offset = 0;
            if (numberOfConnectedChannels == 0)
            {
                logStream.str ("");
                logStream << "The hardware " << hardware->
                    getUniqueID () <<
                    " is connected but the number of connected channels is 0";
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                continue;
            }

            unsigned int bufferToSendSize = 0;
            buffer = (unsigned char *) malloc (node->bufferSendSize * 2);
            memset (buffer, node->bufferSendSize, 0);
            for (unsigned int j = 0; j < hwParameters.size (); j++)
            {
                parameter = (PcieAdcATCAParameter *) hwParameters[j];
                if (!parameter->isConnected ())
                {
                    continue;
                }

                LOG4CXX_INFO (Utils::getLogger (),
                              "Starting acquisition for " +
                              parameter->getUniqueID ());
                std::string parameterXML = parameter->getXMLConfiguration ();

                nBytes =
                    (int) (parameter->getAcqTimeMillis () * 1e-3 *
                           parameter->getAcquisitionFrequency () *
                           sizeof (int));
                //find next multiple of DMAByteSize
                bufferSize = parameter->getDMAByteSize ();
                nBytes = (nBytes / bufferSize + 1) * bufferSize;        // integer division
                readBytes = 0;
                /*                //find next power of 2
                   while((nBytes & (nBytes-1)) != 0){
                   nBytes++;
                   }

                   readBytes = 0;
                   bufferSize = parameter->getDMAByteSize();
                 */
                copyBuffer = (unsigned char *) malloc (bufferSize);
                tstart = mainEvt.getEventTime ();
                tstart.addMilliSeconds (parameter->getDelayTimeMillis ());
                tstart.addMicroSeconds (parameter->getTrigDelayMicros ());
                tend = tstart;
                logStream.str ("");
                logStream << "The buffer size is " << bufferSize;
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());

                logStream.str ("");
                logStream << "Number of bytes requested is " << nBytes;
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());

                int counter = 0;
                while (nBytes > 0)
                {
                    if (nBytes < bufferSize)
                    {
                        bufferSize = nBytes;
                    }
                    offset =
                        j * parameter->getChannelOffset () +
                        counter * bufferSize;
                    counter++;
                    ret = ioctl (fd[i], PCIE_ADC_IOCS_RDOFF, &offset);
                    if (ret < 0)
                    {
                        logStream.str ("");
                        logStream << "While setting the offset to: " << offset
                            << ", error ioctling the device " << node->
                            devIds[i];
                        LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                        continue;
                    }

                    /*  ret = ioctl(fd[i], PCIE_ADC_IOCT_DMA_ENABLE);
                       if(ret < 0){
                       logStream.str("");
                       logStream << "While enabling the DMA, error ioctling the device " << node->devIds[i];
                       LOG4CXX_INFO(Utils::getLogger(), logStream.str());
                       continue;
                       } */

                    readBytes = read (fd[i], copyBuffer, bufferSize);
                    if (readBytes < 1)
                    {
                        logStream.str ("");
                        logStream << "Error! Read bytes returned " <<
                            readBytes <<
                            " The number of bytes still to read are " <<
                            nBytes;;
                        LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                        break;
                    }
                    //swap endianity
                    tempSwap = (int *) (&copyBuffer[0]);
                    for (unsigned int s = 0; s < (readBytes / sizeof (int));
                         s++)
                    {
                        *tempSwap =
                            (copyBuffer[4 * s] << 24) | (copyBuffer[4 * s + 1]
                                                         << 16) |
                            (copyBuffer[4 * s + 2] << 8) |
                            (copyBuffer[4 * s + 3]);
                        tempSwap++;
                    }
                    nBytes -= readBytes;
                    if (readBytes > 0)
                    {
                        memcpy (buffer + bufferToSendSize, copyBuffer,
                                readBytes);
                        bufferToSendSize += readBytes;
                    }

                    if ((bufferToSendSize > node->bufferSendSize))
                    {
                        tend.
                            addNanoSeconds ((int)
                                            (bufferToSendSize / sizeof (int) /
                                             (double) (parameter->
                                                       getAcquisitionFrequency
                                                       ()) * 1e9));
                        eventList[0].setEventID (-1);
                        node->sendNewDataAvailable (hardware->getUniqueID (),
                                                    parameter->getUniqueID (),
                                                    tstart,
                                                    tend,
                                                    eventList,
                                                    parameterXML,
                                                    buffer, bufferToSendSize);
                        memset (buffer, bufferToSendSize, 0);
                        bufferToSendSize = 0;
                        tstart = tend;
                    }
                }

                if (bufferToSendSize > 0)
                {
                    tend.
                        addNanoSeconds ((int)
                                        (bufferToSendSize / sizeof (int) /
                                         (double) (parameter->
                                                   getAcquisitionFrequency ())
                                         * 1e9));
                    eventList[0].setEventID (-1);
                    node->sendNewDataAvailable (hardware->getUniqueID (),
                                                parameter->getUniqueID (),
                                                tstart,
                                                tend,
                                                eventList,
                                                parameterXML,
                                                buffer, bufferToSendSize);
                    memset (buffer, bufferToSendSize, 0);
                    bufferToSendSize = 0;
                }

                free (copyBuffer);
            }

            free (buffer);

        }
        //Wait for the queue to be empty before changing the state again
        while (!node->sendQueue.empty ())
        {
            sleep (1);
        }
        for (int i = 0; i < node->nhardware; i++)
        {
            hardware = dpi[i].hardware;
            if (!hardware->isConnected ())
            {
                continue;
            }
            ret = ioctl (fd[i], PCIE_ADC_IOCT_ACQ_DISABLE);
            if (ret < 0)
            {
                logStream.str ("");
                logStream <<
                    "While disabling acquisition, error ioctling the device "
                    << node->devIds[i];
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                acqComplete = true;
                continue;
            }
            ret = ioctl (fd[i], PCIE_ADC_IOCT_INT_DISABLE);
            if (ret < 0)
            {
                logStream.str ("");
                logStream <<
                    "While disabling interrupts, error ioctling the device "
                    << node->devIds[i];
                LOG4CXX_INFO (Utils::getLogger (), logStream.str ());
                acqComplete = true;
                continue;
            }
            LOG4CXX_INFO (Utils::getLogger (),
                          "Acquisition completed for hardware " +
                          hardware->getUniqueID ());
        }
        for (int i = 0; i < node->nhardware; i++)
        {
            close (fd[i]);
            fd[i] = 0;
        }
        LOG4CXX_INFO (Utils::getLogger (), "Acquisition completed for node");
    }
}

 void
PcieAdcATCANode::changeHardwareStatus (const CORBA::WChar * hardwareUniqueID,
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
