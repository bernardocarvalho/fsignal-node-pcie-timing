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

const wstring PcieTimingParameter::DELAY_TIME         = L"DELAY_TIME";
const wstring PcieTimingParameter::DELAY_MULTIPLIER_UNIT   = L"DELAY_MULTIPLIER_UNIT";
const wstring PcieTimingParameter::PERIOD_TIME_100NS        = L"PERIOD_TIME_100NS";
const wstring PcieTimingParameter::NUM_PULSES              = L"NUM_PULSES";
const wstring PcieTimingParameter::CHANNEL_MODE            = L"CHANNEL_MODE";
const wstring PcieTimingParameter::CHANNEL_INVERT          = L"CHANNEL_INVERT";

const string PcieTimingParameter::CHANNEL_MODE_RESET        = "Reset";
const string PcieTimingParameter::CHANNEL_MODE_TRIGGER      = "Trigger";
const string PcieTimingParameter::CHANNEL_MODE_SQUARE       = "Square";
const string PcieTimingParameter::CHANNEL_MODE_PULSE_TRAIN  = "PulseTrain";

const string PcieTimingParameter::CHANNEL_INVERT_NO         = "NoInvert";
const string PcieTimingParameter::CHANNEL_INVERT_YES        = "Invert";

const string PcieTimingParameter::UNIT_100_NANO = "100 ns";
const string PcieTimingParameter::UNIT_MICRO = "us";
const string PcieTimingParameter::UNIT_MILLI = "ms";
const string PcieTimingParameter::UNIT_SECONDS = "s";

/*const wstring PciEpnParameter::FREQUENCY            = L"FREQUENCY";

*/

FieldDataWrapper PcieTimingParameter::getValue(const wstring &fieldUniqueID){
    stringstream val;

    if(fieldUniqueID == DELAY_TIME){
        val << getDelayTime();
    }
    else if(fieldUniqueID == DELAY_MULTIPLIER_UNIT){
        val << getDelayMultiplierUnit();
    }
    else if(fieldUniqueID == PERIOD_TIME_100NS){
        val << getPeriodTime100nS();
    }
    else if(fieldUniqueID == NUM_PULSES){
        val << getNumPulses();
    }
    else if(fieldUniqueID == CHANNEL_MODE){
        val << getChannelMode();
    }
    else if(fieldUniqueID == CHANNEL_INVERT){
        val << getChannelInvert();
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
    if(fieldUniqueID == DELAY_TIME){
        vals >> delay;
    }
    else if(fieldUniqueID == DELAY_MULTIPLIER_UNIT){
        vals >> delayMultiplierUnit;
    }
    else if(fieldUniqueID == PERIOD_TIME_100NS){
        vals >> period100nS;
    }
    else if(fieldUniqueID == NUM_PULSES){
        vals >> numPulses;
    }
    else if(fieldUniqueID == CHANNEL_MODE){
        vals >> channelMode;
    }
    else if(fieldUniqueID == CHANNEL_INVERT){
        vals >> channelInvert;
    }

}

void PcieTimingParameter::setValues(const vector<FieldDataWrapper> &fdl){
    for(unsigned int i=0; i<fdl.size(); i++)
        setValue(fdl[i]);
}

vector<FieldDataWrapper> PcieTimingParameter::getValues(){
    vector<FieldDataWrapper> vec;
    vec.push_back(getValue(DELAY_TIME));
    vec.push_back(getValue(DELAY_MULTIPLIER_UNIT));
    vec.push_back(getValue(PERIOD_TIME_100NS));
    vec.push_back(getValue(NUM_PULSES));
    vec.push_back(getValue(CHANNEL_MODE));
    vec.push_back(getValue(CHANNEL_INVERT));

    return vec;
}
void PcieTimingParameter::setConnected(bool connected){
    FSParameter::setConnected(connected);
}

unsigned int PcieTimingParameter::getDelayMultiplier(){
    if(delayMultiplierUnit == UNIT_100_NANO){
        return 1;
    }
    else if(delayMultiplierUnit == UNIT_MICRO){
        return 10;
    }
    else if(delayMultiplierUnit == UNIT_MILLI){
        return 10*1e3;
    }
    else if(delayMultiplierUnit == UNIT_SECONDS){
        return 10*1e6;
    }

    return 1;
}
