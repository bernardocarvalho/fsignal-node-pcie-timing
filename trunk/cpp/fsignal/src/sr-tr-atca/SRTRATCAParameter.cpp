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
#include "SRTRATCAParameter.h"
#include "../utils/FSignalUtils.h"
#include "../utils/FieldDataWrapper.h"
#include <sstream>
#include <cmath>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "pcieSrTr_ioctl.h"

using namespace org::cfn::scad::core::corba::hardware;
using namespace org::cfn::scad::core::corba;
using namespace fsignal;
using namespace std;

const wstring SRTRATCAParameter::COMPL                = L"COMPL";
const wstring SRTRATCAParameter::ACQ_TYPE             = L"ACQ_TYPE";
const wstring SRTRATCAParameter::ILV_CHANNELS         = L"ILV_CHANNELS";
const wstring SRTRATCAParameter::ACQ_MODE             = L"ACQ_MODE";
const wstring SRTRATCAParameter::TRIGGER_TYPE         = L"TRIGGER_TYPE";
const wstring SRTRATCAParameter::PRE_TRIGGER_TIME     = L"PRE_TRIGGER_TIME";
const wstring SRTRATCAParameter::PROCESS_K            = L"PROCESS_K";
const wstring SRTRATCAParameter::PROCESS_L            = L"PROCESS_L";
const wstring SRTRATCAParameter::PROCESS_M            = L"PROCESS_M";
const wstring SRTRATCAParameter::PROCESS_T            = L"PROCESS_T";
const wstring SRTRATCAParameter::DMA_BYTE_SIZE        = L"DMA_BYTE_SIZE";
const wstring SRTRATCAParameter::DMA_N_BUFFS          = L"DMA_N_BUFFS";
const wstring SRTRATCAParameter::BUFFER_READ_SIZE     = L"BUFFER_READ_SIZE";
const wstring SRTRATCAParameter::FREQUENCY            = L"FREQUENCY";
const wstring SRTRATCAParameter::ACQ_BYTE_SIZE        = L"ACQ_BYTE_SIZE";
const wstring SRTRATCAParameter::BST_PRE_TRIGGER_DATA = L"BST_PRE_TRIGGER_DATA";
const wstring SRTRATCAParameter::BST_PULSE_WIDTH      = L"BST_PULSE_WIDTH";
const wstring SRTRATCAParameter::BST_TRG_ACC          = L"BST_TRG_ACC";
const wstring SRTRATCAParameter::EXT_CONFIG_FILE      = L"EXT_CONFIG_FILE";
const wstring SRTRATCAParameter::TRIGGER_MODE         = L"TRIGGER_MODE";
const wstring SRTRATCAParameter::INPUT_FILTER         = L"INPUT_FILTER";
const wstring SRTRATCAParameter::TRIGGER_FILTER       = L"TRIGGER_FILTER";

const string SRTRATCAParameter::FIR                   = "FIR";
const string SRTRATCAParameter::NONE                  = "none";

const string SRTRATCAParameter::TRIGGER_MODE_LEVEL    = "Level";
const string SRTRATCAParameter::TRIGGER_MODE_FLANK    = "Flank";

const string SRTRATCAParameter::ACQ_TYPE_SINGLE       = "Single";
const string SRTRATCAParameter::ACQ_TYPE_INTERLEAVED  = "Interleaved";

const string SRTRATCAParameter::ACQ_MODE_RAW          = "Raw";
const string SRTRATCAParameter::ACQ_MODE_PROCESSED    = "Processed";
const string SRTRATCAParameter::ACQ_MODE_BURST        = "Burst";
const string SRTRATCAParameter::ACQ_MODE_CALIBRATION  = "Calibration";

const string SRTRATCAParameter::TRG_TYPE_SOFTWARE     = "Software";
const string SRTRATCAParameter::TRG_TYPE_HARDWARE     = "Hardware";

const int    SRTRATCAParameter::FPGA_FREQUENCY        = 4200000;

FieldDataWrapper SRTRATCAParameter::getValue(const wstring &fieldUniqueID){
    stringstream val;
    
    if(fieldUniqueID == ACQ_MODE){
        val << getAcqMode();
    }
    else if(fieldUniqueID == ACQ_TYPE){
        val << getAcqType();
    }
    else if(fieldUniqueID == EXT_CONFIG_FILE){
        val << getExtFileContents();
    }
    else if(fieldUniqueID == ILV_CHANNELS){
        val << getInterleavedChannels();
    }
    else if(fieldUniqueID == BUFFER_READ_SIZE){
        val << getBufferReadSize();
    }
    else if(fieldUniqueID == DMA_BYTE_SIZE){
        val << getDMAByteSize();
    }
    else if(fieldUniqueID == DMA_N_BUFFS){
        val << getDMANBuffs();
    }
    else if(fieldUniqueID == FREQUENCY){
        val << getFrequency();
    }
    else if(fieldUniqueID == BST_PRE_TRIGGER_DATA){
        val << getBurstPreTriggerData();
    }
    else if(fieldUniqueID == BST_PULSE_WIDTH){
        val << getBurstPulseWidth();
    }
    else if(fieldUniqueID == BST_TRG_ACC){
        val << getBurstTriggerAcc();
    }
    else if(fieldUniqueID == PRE_TRIGGER_TIME){
        val << getPreTriggerTime();
    }
    else if(fieldUniqueID == PROCESS_K){
        val << getProcessK();
    }
    else if(fieldUniqueID == PROCESS_L){
        val << getProcessL();
    }
    else if(fieldUniqueID == PROCESS_M){
        val << getProcessM();
    }
    else if(fieldUniqueID == PROCESS_T){
        val << getProcessT();
    }
    else if(fieldUniqueID == COMPL){
        val << getCompl();
    }
    else if(fieldUniqueID == TRIGGER_TYPE){
        val << getTriggerType();
    }
    else if(fieldUniqueID == ACQ_BYTE_SIZE){
        val << getAcqByteSize();
    }
    else if(fieldUniqueID == TRIGGER_MODE){
        val << getTriggerMode();
    }
    else if(fieldUniqueID == INPUT_FILTER){
        val << getInputFilter();
    }
    else if(fieldUniqueID == TRIGGER_FILTER){
        val << getTriggerFilter();
    }
    return FieldDataWrapper(fieldUniqueID, val.str());
}

void SRTRATCAParameter::setValue(const FieldDataWrapper &fd){
    FieldDataWrapper fdw = fd;
    wstring fieldUniqueID = fdw.getFieldUniqueID();
    vector<unsigned char> vec_str = fdw.getValue();
    char val_str[vec_str.size() + 1];
    for(unsigned int i=0; i< vec_str.size(); i++){
        val_str[i] = vec_str[i];
    }
    val_str[vec_str.size()] = 0;        
    
    stringstream vals(val_str);
    
    int          ioctlVal = 0;
    int          tempi    = 0;
    double       tempd    = 0;    
    string       tempstr;    
    
    if(fieldUniqueID == ACQ_MODE){
        vals >> tempstr;        
        if(tempstr == ACQ_MODE_RAW){
            ioctlVal = 0;
        }
        else if(tempstr == ACQ_MODE_PROCESSED){
            ioctlVal = 1;
        }
        else if(tempstr == ACQ_MODE_BURST){
            ioctlVal = 2;
        }
        else if(tempstr == ACQ_MODE_CALIBRATION){
            ioctlVal = 3;
        }
        else{
            tempi=atoi(tempstr.c_str());
            if(tempi == 0){
                tempstr = ACQ_MODE_RAW;
                ioctlVal = 0;
            }
            else if(tempi == 1){
                tempstr = ACQ_MODE_BURST;
                ioctlVal = 2;
            }
            else if(tempi == 2){
                tempstr = ACQ_MODE_PROCESSED;
                ioctlVal = 1;
            }
            else if(tempi == 3){
                tempstr = ACQ_MODE_CALIBRATION;
                ioctlVal = 3;
            }
            else{
                tempstr = ACQ_MODE_RAW;
                ioctlVal = 0;
            }
        }
        if(performIOCTL(PCIE_SRTR_IOCS_DQTP, ioctlVal)){
            acqMode = tempstr;
        }
    }
    else if(fieldUniqueID == EXT_CONFIG_FILE){
	logStream.str("");
        logStream << "EXT_CONFIG_FILE is not being updated on purpose! As soon as tested with me present, remove me!";
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
//TEMP PATCH. REMOVE ME
#if 0
        bool    errors   = false;
        tempstr          = val_str;
        vector<string> parsed;
        parsed.clear();
        Utils::tokenize(tempstr, parsed, " ", true);        
        for(unsigned int i=0; i<parsed.size(); i++) {
            ioctlVal = atoi(parsed[i].c_str());            
            printf("Trying to upload: %d\n", ioctlVal);
            if(!performIOCTL(PCIE_SRTR_IOCS_FILEDATA, ioctlVal)){
                errors = true;
                //break;
            }
        }       

        if(!errors){
            extFileContents = val_str;
        }
#endif
    }
    else if(fieldUniqueID == ACQ_TYPE || fieldUniqueID == ILV_CHANNELS){
        if(fieldUniqueID == ACQ_TYPE){
            vals >> tempstr;
            if(tempstr == ACQ_TYPE_SINGLE || tempstr == ACQ_TYPE_INTERLEAVED){
                acqType = tempstr;
            }
            else{
                tempi = atoi(tempstr.c_str());
                if(tempi == 1){
                    acqType = ACQ_TYPE_INTERLEAVED;
                }
                else{
                    acqType = ACQ_TYPE_SINGLE;
                }
            }
            tempi = interleavedChannels;
        }
        else if(fieldUniqueID == ILV_CHANNELS){
            vals >> tempi;
        }
        if(acqType == ACQ_TYPE_SINGLE){
            interleavedChannels = 1;
            ioctlVal = 0;            
            performIOCTL(PCIE_SRTR_IOCS_ILVM, ioctlVal);
        }else{
            if(tempi == 4){
                ioctlVal = 3;
            }
            else if(tempi == 2){
                ioctlVal = 1;
            }
            else{
                ioctlVal = 0;
            }
            if(performIOCTL(PCIE_SRTR_IOCS_ILVM, ioctlVal)){
                interleavedChannels = tempi;
                if(interleavedChannels == 4){               
                    setConnected(channelID == 1);                    
                }else if(interleavedChannels == 2){        
                    setConnected(channelID < 3);
                }
            }
        }        
    }
    else if(fieldUniqueID == BUFFER_READ_SIZE){
        vals >> bufReadSize;
    }
    else if(fieldUniqueID == DMA_BYTE_SIZE){
        vals >> dmaByteSize;
    }
    else if(fieldUniqueID == DMA_N_BUFFS){
        vals >> dmaNBuffs;
    }
    else if(fieldUniqueID == FREQUENCY){
        vals >> tempd;
        if(tempd == 250 || tempd == 400){
            ioctlVal = 1;
        }
        else if(tempd == 200){
            ioctlVal = 3;
        }
        else if(tempd == 125 || tempd == 160){
            ioctlVal = 5;
        }
        else if(tempd == 100){
            ioctlVal = 7;
        }
        else if(tempd == 62.5 || tempd == 80){
            ioctlVal = 9;
        }
        else if(tempd == 50){
            ioctlVal = 11;
        }
        else{
            ioctlVal = 1;
        }
        if(performIOCTL(PCIE_SRTR_IOCS_PLLCFG, ioctlVal)){
            frequency = tempd;
        }
    }
    else if(fieldUniqueID == BST_PRE_TRIGGER_DATA){
        vals >> tempi;
        ioctlVal = tempi;
        if(performIOCTL(PCIE_SRTR_IOCS_PTRG, ioctlVal)){
            burstPreTriggerData = tempi;
        }        
    }
    else if(fieldUniqueID == BST_PULSE_WIDTH){
        vals >> tempi;
        ioctlVal = tempi;
        if(performIOCTL(PCIE_SRTR_IOCS_PWIDTH, ioctlVal)){
            burstPulseWidth = tempi;
        }        
    }
    else if(fieldUniqueID == BST_TRG_ACC){
        vals >> tempi;
        //Start in one for the sake of the end user (check spec doc in order to understand)
        ioctlVal = tempi - 1;
        if(performIOCTL(PCIE_SRTR_IOCS_Trg_Acc, ioctlVal)){
            burstTrgAcc = tempi;
        }        
    }
    else if(fieldUniqueID == PRE_TRIGGER_TIME){
        vals >> tempi;    
        ioctlVal = (int)(tempi * (FPGA_FREQUENCY / 1e6)); //Convert from us to ticks
        if(performIOCTL(PCIE_SRTR_IOCS_POST_TRG, ioctlVal)){
            preTriggerTime = tempi;
        }
    }
    else if(fieldUniqueID == PROCESS_K){
        vals >> tempi;
        ioctlVal = tempi;
        if(performIOCTL(PCIE_SRTR_IOCS_DATA_PROC_K, ioctlVal)){
            processK = tempi;
        }        
    }
    else if(fieldUniqueID == PROCESS_L){        
        vals >> tempi;
        ioctlVal = tempi;
        if(performIOCTL(PCIE_SRTR_IOCS_DATA_PROC_L, ioctlVal)){
            processL = tempi;
        }
    }
    else if(fieldUniqueID == PROCESS_M){        
        vals >> tempi;
        ioctlVal = tempi;
        if(performIOCTL(PCIE_SRTR_IOCS_DATA_PROC_M, ioctlVal)){
            processM = tempi;
        }
    }
    else if(fieldUniqueID == PROCESS_T){        
        vals >> tempi;
        ioctlVal = tempi;
        if(performIOCTL(PCIE_SRTR_IOCS_THRESHOLD, ioctlVal)){
            processT = tempi;
        }
    }
    else if(fieldUniqueID == COMPL){
        vals >> tempi;
        ioctlVal = tempi;
        if(performIOCTL(PCIE_SRTR_IOCS_COMPL, ioctlVal)){
            commandCompl = tempi;
        }
    }
    else if(fieldUniqueID == TRIGGER_TYPE){
        vals >> tempstr;
        if(tempstr == TRG_TYPE_SOFTWARE || tempstr == TRG_TYPE_HARDWARE){
            triggerType = tempstr;
        }
        else{
            tempi=atoi(tempstr.c_str());
            if(tempi == 0){
                triggerType = TRG_TYPE_SOFTWARE;           
            }
            else{
                triggerType = TRG_TYPE_HARDWARE;
            }
        }
    }
    else if(fieldUniqueID == TRIGGER_MODE){
        vals >> tempstr;
        if(tempstr != TRIGGER_MODE_LEVEL && tempstr != TRIGGER_MODE_FLANK){
            tempi=atoi(tempstr.c_str());
            if(tempi == 0){
                tempstr = TRIGGER_MODE_LEVEL;
            }
            else{
                tempstr = TRIGGER_MODE_FLANK;
            }
        }
        if(tempstr == TRIGGER_MODE_LEVEL){
            if(performIOCTL(PCIE_SRTR_IOCS_TRGM, 0)){
                triggerMode = tempstr;
            }
        }
        else if(tempstr == TRIGGER_MODE_FLANK){
            if(performIOCTL(PCIE_SRTR_IOCS_TRGM, 1)){
                triggerMode = tempstr;
            }
        }
    }
    else if(fieldUniqueID == INPUT_FILTER){
        vals >> tempstr;
        if(tempstr != FIR && tempstr != NONE){
            tempi=atoi(tempstr.c_str());
            if(tempi == 0){
                tempstr = NONE;
            }
            else{
                tempstr = FIR;
            }
        }
        if(tempstr == NONE){
            if(performIOCTL(PCIE_SRTR_IOCS_INFIR, 0)){
                inputFilter = tempstr;
            }
        }
        else if(tempstr == FIR){
            if(performIOCTL(PCIE_SRTR_IOCS_INFIR, 1)){
                inputFilter = tempstr;
            }
        }
    }
    else if(fieldUniqueID == TRIGGER_FILTER){
        vals >> tempstr;
        if(tempstr != FIR && tempstr != NONE){
            tempi=atoi(tempstr.c_str());
            if(tempi == 0){
                tempstr = NONE;
            }
            else{
                tempstr = FIR;
            }
        }
        if(tempstr == NONE){
            if(performIOCTL(PCIE_SRTR_IOCS_TFIR, 0)){
                triggerFilter = tempstr;
            }
        }
        else if(tempstr == FIR){
            if(performIOCTL(PCIE_SRTR_IOCS_TFIR, 1)){
                triggerFilter = tempstr;
            }
        }
    }
    else if(fieldUniqueID == ACQ_BYTE_SIZE){
        //This is now set in node because we have to take into account the number
        //of connected channels
/*        vals >> tempu;
        //must be a power of DMA_BYTE_SIZE
        while((tempu % bufReadSize) != 0){
            tempu++;
        }
        if(tempu > 0x7FFFFFFF){
            tempu = 0x7FFFFFFF;
        }
        ioctlVal = tempu;
        if(performIOCTL(PCIE_SRTR_IOCS_ACQBYTESIZE, ioctlVal)){
            acqByteSize = tempu;
        }*/
        vals >> acqByteSize;
    }
}

void SRTRATCAParameter::setValues(const vector<FieldDataWrapper> &fdl){
    for(unsigned int i=0; i<fdl.size(); i++)
        setValue(fdl[i]);
}

vector<FieldDataWrapper> SRTRATCAParameter::getValues(){
    vector<FieldDataWrapper> vec;
    vec.push_back(getValue(ACQ_TYPE));
    vec.push_back(getValue(ACQ_MODE));
    vec.push_back(getValue(ILV_CHANNELS));    
    vec.push_back(getValue(BUFFER_READ_SIZE));
    vec.push_back(getValue(DMA_BYTE_SIZE));
    vec.push_back(getValue(DMA_N_BUFFS));
    vec.push_back(getValue(FREQUENCY));
    vec.push_back(getValue(PRE_TRIGGER_TIME));
    vec.push_back(getValue(PROCESS_K));
    vec.push_back(getValue(PROCESS_L));
    vec.push_back(getValue(PROCESS_M));
    vec.push_back(getValue(PROCESS_T));
    vec.push_back(getValue(COMPL));
    vec.push_back(getValue(TRIGGER_TYPE));
    vec.push_back(getValue(ACQ_BYTE_SIZE));    
    vec.push_back(getValue(BST_PRE_TRIGGER_DATA));
    vec.push_back(getValue(BST_PULSE_WIDTH));
    vec.push_back(getValue(BST_TRG_ACC));
    vec.push_back(getValue(EXT_CONFIG_FILE));
    vec.push_back(getValue(INPUT_FILTER));
    vec.push_back(getValue(TRIGGER_TYPE));
    vec.push_back(getValue(TRIGGER_MODE));
    
    return vec;
}

bool SRTRATCAParameter::performIOCTL(int ioctlFun, int value){
    int fd = open(devLocation.c_str(), O_RDWR);
    int ret = 0;
    if(fd < 0){
        logStream.str("");
        logStream << "Failed to open the device at: " << devLocation << " for operation " << ioctlFun << " with value " << value;
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
        return false;
    }
       
    ret = ioctl(fd, ioctlFun, &value);
    
    close(fd);
    return ret == 0;
}

int SRTRATCAParameter::getNumberOfEvents(){
    int fd     = open(devLocation.c_str(), O_RDWR);
    int ret    = 0;
    int events = -1;
    if(fd < 0){
        logStream.str("");
        logStream << "Failed to open the device at: " << devLocation << " for getting the number of events";
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
        return -1;
    }

    ret = ioctl(fd, PCIE_SRTR_IOCG_EVENT_COUNTER, &events);
    if(ret < 0){
        logStream.str("");
        logStream << "Failed to ioctl the device at: " << devLocation << " while getting the number of events";
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
        close(fd);
        return -1;
    }

    close(fd);
    return events;
}

int SRTRATCAParameter::getNumberOfEventsInBytes(){
    int events = getNumberOfEvents();
    if(events == -1 || acqMode == ACQ_MODE_RAW){
        return events;
    }
    if(acqMode == ACQ_MODE_BURST){
	unsigned int size = events * (getBurstPulseWidth() * 2 + 8);
	while((size % bufReadSize) != 0){
            size++;
        }
	if(size > 0xffffffff){
            size = 0xffffffff;
        }
        return size;
    }
    else if(acqMode == ACQ_MODE_PROCESSED){
        return events * 8;
    }

    return -1;
}

void SRTRATCAParameter::setConnected(bool connected){
    int fd = open(devLocation.c_str(), O_RDWR);
    int ret     = 0;
    int command = 0;
    if(fd < 0){
        logStream.str("");
        logStream << "Failed to open the device at: " << devLocation << " for getting the command value";
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
        return;
    }

    ret = ioctl(fd, PCIE_SRTR_IOCG_COMMAND, &command);
    if(ret < 0){
        logStream.str("");
        logStream << "Failed to ioctl the device at: " << devLocation << " while getting the command value";
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
        close(fd);
        return;
    }

    command &= 0x3C0;
    command >>= 6;


    FSParameter::setConnected(connected);
    if(connected){
        command |= (1 << (channelID - 1));
    }else{
        command &= ~(1 << (channelID - 1));
    }    
    ret = ioctl(fd, PCIE_SRTR_IOCS_CHAN_ON_OFF, &command);
    if(ret < 0){
        logStream.str("");
        logStream << "Failed to ioctl the device at: " << devLocation << " while writing the command value: " << command;
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
        return;
    }
    close(fd);
}

void SRTRATCAParameter::dumpConfiguration(){
    logStream.str("");
    logStream << getUniqueID();
    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
    logStream.str("Connected = ");
    if(isConnected()){
        logStream << "True";
    }
    else{
        logStream << "False";
    }
    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
    vector<FieldDataWrapper> values = SRTRATCAParameter::getValues();
    for(unsigned int i=0; i<values.size(); i++){
        wstring fwuid = values[i].getFieldUniqueID();
        string  fuid  = string(fwuid.begin(), fwuid.end());

        vector<unsigned char> vec_str = values[i].getValue();
        char val_str[vec_str.size() + 1];
        for(unsigned int j=0; j< vec_str.size(); j++){
            val_str[j] = vec_str[j];
        }
        val_str[vec_str.size()] = 0;        

        logStream.str("");
        logStream << fuid << " = " << val_str;
        LOG4CXX_INFO(Utils::getLogger(), logStream.str());
    }
}
