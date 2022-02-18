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
#include "PcieAdcATCAParameter.h"
#include "../utils/FSignalUtils.h"
#include "../utils/FieldDataWrapper.h"
#include <sstream>
#include <cmath>
#include <sys/ioctl.h>
#include <fcntl.h>

using namespace org::cfn::scad::core::corba::hardware;
using namespace org::cfn::scad::core::corba;
using namespace fsignal;
using namespace std;

const wstring PcieAdcATCAParameter::TRIGGER_TYPE         = L"TRIGGER_TYPE";
const wstring PcieAdcATCAParameter::DMA_BYTE_SIZE        = L"DMA_BYTE_SIZE";
const wstring PcieAdcATCAParameter::FREQUENCY            = L"FREQUENCY";
const wstring PcieAdcATCAParameter::MEM_SIZE             = L"MEM_SIZE";
const wstring PcieAdcATCAParameter::ACQ_TIME_MILLIS      = L"ACQ_TIME_MILLIS";
const wstring PcieAdcATCAParameter::DELAY_TIME_MILLIS    = L"DELAY_TIME_MILLIS";
const wstring PcieAdcATCAParameter::TRIGGER_DELAY_MICROS = L"TRIGGER_DELAY_MICROS";

const string PcieAdcATCAParameter::TRG_TYPE_SOFTWARE     = "Software";
const string PcieAdcATCAParameter::TRG_TYPE_HARDWARE     = "Hardware";

FieldDataWrapper PcieAdcATCAParameter::getValue(const wstring &fieldUniqueID){
    stringstream val;
    
    if(fieldUniqueID == FREQUENCY){
        val << getAcquisitionFrequency();
    }
    else if(fieldUniqueID == DMA_BYTE_SIZE){
        val << getDMAByteSize();
    }
    else if(fieldUniqueID == TRIGGER_TYPE){
        val << getTriggerType();
    }
    else if(fieldUniqueID == MEM_SIZE){
        val << getMemSize();
    }
    else if(fieldUniqueID == ACQ_TIME_MILLIS){
      val << getAcqTimeMillis();
    }
    else if(fieldUniqueID == DELAY_TIME_MILLIS){
      val << getDelayTimeMillis();
    }
    else if(fieldUniqueID == TRIGGER_DELAY_MICROS){
        val << getTrigDelayMicros();
    }

    return FieldDataWrapper(fieldUniqueID, val.str());
}

void PcieAdcATCAParameter::setValue(const FieldDataWrapper &fd){
    FieldDataWrapper fdw = fd;
    wstring fieldUniqueID = fdw.getFieldUniqueID();
    vector<unsigned char> vec_str = fdw.getValue();
    char val_str[vec_str.size() + 1];
    for(unsigned int i=0; i< vec_str.size(); i++){
        val_str[i] = vec_str[i];
    }
    val_str[vec_str.size()] = 0;        
    
    stringstream vals(val_str);
    if(fieldUniqueID == FREQUENCY){
        vals >> acqFrequency;
    }
    else if(fieldUniqueID == DMA_BYTE_SIZE){
        vals >> dmaByteSize;
    }
    else if(fieldUniqueID == TRIGGER_TYPE){        
        vals >> triggerType;
    }
    else if(fieldUniqueID == MEM_SIZE){
        vals >> memSize;
    }
    else if(fieldUniqueID == ACQ_TIME_MILLIS){
        vals >> acqTimeMillis;
    }
    else if(fieldUniqueID == DELAY_TIME_MILLIS){
        vals >> delayTimeMillis;
    }
    else if(fieldUniqueID == TRIGGER_DELAY_MICROS){
        vals >> trigDelayMicros;
    }
}

void PcieAdcATCAParameter::setValues(const vector<FieldDataWrapper> &fdl){
    for(unsigned int i=0; i<fdl.size(); i++)
        setValue(fdl[i]);
}

vector<FieldDataWrapper> PcieAdcATCAParameter::getValues(){
    vector<FieldDataWrapper> vec;
    vec.push_back(getValue(FREQUENCY));
    vec.push_back(getValue(DMA_BYTE_SIZE));
    vec.push_back(getValue(TRIGGER_TYPE));
    vec.push_back(getValue(MEM_SIZE));    
    vec.push_back(getValue(ACQ_TIME_MILLIS));
    vec.push_back(getValue(DELAY_TIME_MILLIS));
    vec.push_back(getValue(TRIGGER_DELAY_MICROS));
    
    return vec;
}

bool PcieAdcATCAParameter::performIOCTL(int ioctlFun, int value){
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

void PcieAdcATCAParameter::setConnected(bool connected){
    FSParameter::setConnected(connected);
}
