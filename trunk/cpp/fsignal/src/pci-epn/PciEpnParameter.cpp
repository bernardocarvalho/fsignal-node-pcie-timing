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
#include "PciEpnParameter.h"
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

const wstring PciEpnParameter::TRIGGER_SECS         = L"TRIGGER_SECS";
const wstring PciEpnParameter::TRIGGER_NSECS        = L"TRIGGER_NSECS";
const wstring PciEpnParameter::PULSE_NSECS          = L"PULSE_NSECS";

/*const wstring PciEpnParameter::FREQUENCY            = L"FREQUENCY";
const wstring PciEpnParameter::MEM_SIZE             = L"MEM_SIZE";
const wstring PciEpnParameter::ACQ_TIME_MILLIS      = L"ACQ_TIME_MILLIS";

const string PciEpnParameter::TRG_TYPE_SOFTWARE     = "Software";
const string PciEpnParameter::TRG_TYPE_HARDWARE     = "Hardware";
*/
FieldDataWrapper PciEpnParameter::getValue(const wstring &fieldUniqueID){
    stringstream val;
    
    if(fieldUniqueID == TRIGGER_SECS){
        val << getTriggerSecs();
    }
    else if(fieldUniqueID == TRIGGER_NSECS){
        val << getTriggerNSecs();
    }
    else if(fieldUniqueID == PULSE_NSECS){
        val << getPulseWidth();
    }
    return FieldDataWrapper(fieldUniqueID, val.str());
}

void PciEpnParameter::setValue(const FieldDataWrapper &fd){
    FieldDataWrapper fdw = fd;
    wstring fieldUniqueID = fdw.getFieldUniqueID();
    vector<unsigned char> vec_str = fdw.getValue();
    char val_str[vec_str.size() + 1];
    for(unsigned int i=0; i< vec_str.size(); i++){
        val_str[i] = vec_str[i];
    }
    val_str[vec_str.size()] = 0;        
    
    stringstream vals(val_str);
    if(fieldUniqueID == TRIGGER_SECS){
        vals >> trigSecs;
    }
    else if(fieldUniqueID == TRIGGER_NSECS){
        vals >> trigNSecs;
    }
    else if(fieldUniqueID == PULSE_NSECS){
        vals >> pulseWidth;
    }
}

void PciEpnParameter::setValues(const vector<FieldDataWrapper> &fdl){
    for(unsigned int i=0; i<fdl.size(); i++)
        setValue(fdl[i]);
}

vector<FieldDataWrapper> PciEpnParameter::getValues(){
    vector<FieldDataWrapper> vec;
    vec.push_back(getValue(TRIGGER_SECS));
    vec.push_back(getValue(TRIGGER_NSECS));
    vec.push_back(getValue(PULSE_NSECS));
    
    return vec;
}
/*
bool PciEpnParameter::performIOCTL(int ioctlFun, int value){
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
*/
void PciEpnParameter::setConnected(bool connected){
    FSParameter::setConnected(connected);
}
