/**
 *
 * @file PcieTimingParameter.cpp
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

#include "PcieTimingParameter.h"
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

const wstring PcieTimingParameter::DELAY_TIME_10NS         = L"DELAY_TIME_10NS";
const wstring PcieTimingParameter::PERIOD_TIME_10NS        = L"PERIOD_TIME_10NS";
const wstring PcieTimingParameter::CONFIG                  = L"CONFIG";

/*const wstring PciEpnParameter::FREQUENCY            = L"FREQUENCY";
  const wstring PciEpnParameter::MEM_SIZE             = L"MEM_SIZE";
  const wstring PciEpnParameter::ACQ_TIME_MILLIS      = L"ACQ_TIME_MILLIS";

  const string PciEpnParameter::TRG_TYPE_SOFTWARE     = "Software";
  const string PciEpnParameter::TRG_TYPE_HARDWARE     = "Hardware";
  */
FieldDataWrapper PcieTimingParameter::getValue(const wstring &fieldUniqueID){
    stringstream val;

    if(fieldUniqueID == DELAY_TIME_10NS){
        val << getDelayTime10nS();
    }
    else if(fieldUniqueID == PERIOD_TIME_10NS){
        val << getPeriodTime10nS();
    }
    else if(fieldUniqueID == CONFIG){
        val << getConfig();
    }
    return FieldDataWrapper(fieldUniqueID, val.str());
}

void PcieTimingParameter::setValue(const FieldDataWrapper &fd){
    FieldDataWrapper fdw = fd;
    wstring fieldUniqueID = fdw.getFieldUniqueID();
    vector<unsigned char> vec_str = fdw.getValue();
    char val_str[vec_str.size() + 1];
    for(unsigned int i=0; i< vec_str.size(); i++){
        val_str[i] = vec_str[i];
    }
    val_str[vec_str.size()] = 0;

    stringstream vals(val_str);
    if(fieldUniqueID == DELAY_TIME_10NS){
        vals >> delay10nS;
    }
    else if(fieldUniqueID == PERIOD_TIME_10NS){
        vals >> period10nS;
    }
    else if(fieldUniqueID == CONFIG){
        vals >> config;
    }
}

void PcieTimingParameter::setValues(const vector<FieldDataWrapper> &fdl){
    for(unsigned int i=0; i<fdl.size(); i++)
        setValue(fdl[i]);
}

vector<FieldDataWrapper> PcieTimingParameter::getValues(){
    vector<FieldDataWrapper> vec;
    vec.push_back(getValue(DELAY_TIME_10NS));
    vec.push_back(getValue(PERIOD_TIME_10NS));
    vec.push_back(getValue(CONFIG));

    return vec;
}
void PcieTimingParameter::setConnected(bool connected){
    FSParameter::setConnected(connected);
}
