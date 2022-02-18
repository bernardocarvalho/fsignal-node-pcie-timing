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
#include "Tr512Parameter.h"
#include "../utils/FSignalUtils.h"
#include "../utils/FieldDataWrapper.h"
#include <sstream>

using namespace org::cfn::scad::core::corba::hardware;
using namespace org::cfn::scad::core::corba;
using namespace fsignal;
using namespace std;

//CORBA::remove_ref(*ref);

wstring Tr512Parameter::DEVICE_NAME_ID = L"DEVICE_NAME";
wstring Tr512Parameter::TRIGGER_POL_ID = L"TRIGGER_POL";
wstring Tr512Parameter::CLOCK_ID = L"CLOCK";
wstring Tr512Parameter::DECIMATION_ID = L"DECIMATION";
wstring Tr512Parameter::N_SAMPLES_ID = L"N_SAMPLES";
wstring Tr512Parameter::READ_TIMOUT_ID = L"READ_TIMOUT";
wstring Tr512Parameter::TRIGGER_DELAY_ID = L"TRIGGER_DELAY";
wstring Tr512Parameter::AMPLITUDE_ID = L"AMPLITUDE";
//wstring Tr512Parameter::TBS_ID = L"TBS";

FieldDataWrapper Tr512Parameter::getValue(const wstring &fieldUniqueID)
{
	stringstream val;
	if(fieldUniqueID == DEVICE_NAME_ID)
		val << getDevName();
	else if(fieldUniqueID == TRIGGER_POL_ID)
		val << getTriggerPol();
	else if(fieldUniqueID == CLOCK_ID)
		val << getClock();
	else if(fieldUniqueID == DECIMATION_ID)
		val << getDecimation();
	else if(fieldUniqueID == N_SAMPLES_ID)
		val << getNSamples();
	else if(fieldUniqueID == READ_TIMOUT_ID)
		val << getRdTmOut();
	else if(fieldUniqueID == TRIGGER_DELAY_ID)
		val << getTrgDlay();
	else if(fieldUniqueID == AMPLITUDE_ID)
		val << getAmplitude();
	//        else if(fieldUniqueID == TBS_ID)
	//	val << getTBS();	

	return FieldDataWrapper(fieldUniqueID, val.str());
}

void Tr512Parameter::setValue(const FieldDataWrapper &fd)
{
	FieldDataWrapper fdw = fd;
	wstring fieldUniqueID = fdw.getFieldUniqueID();
	vector<unsigned char> vec_str = fdw.getValue();
	char val_str[vec_str.size()];
	for(int i=0; i< vec_str.size(); i++)
		val_str[i] = vec_str[i];	

	stringstream vals(val_str);

	if(fieldUniqueID == DEVICE_NAME_ID){
	  memset(dev_name, 0, 100);
	  strncpy(dev_name, val_str, vec_str.size());
	  //	  cout << "setValue Dev Name:"<< dev_name << endl;
	}
	else if(fieldUniqueID == TRIGGER_POL_ID)
		vals >> triggerpol;
	else if(fieldUniqueID == CLOCK_ID)
		vals >> clock;
	else if(fieldUniqueID == DECIMATION_ID)
		vals >> decimation;
	else if(fieldUniqueID == N_SAMPLES_ID)
		vals >> nsamples;
	else if(fieldUniqueID == READ_TIMOUT_ID)
                vals >> rdTmOut;
	else if(fieldUniqueID == TRIGGER_DELAY_ID)
                vals >> trigDelay;
	else if(fieldUniqueID == AMPLITUDE_ID)
                vals >> amplitude;
	//	else if(fieldUniqueID == TBS_ID)
        //        vals >> tbs;
}

void Tr512Parameter::setValues(const vector<FieldDataWrapper> &fdl)
{
	for(int i=0; i<fdl.size(); i++)
		setValue(fdl[i]);
}

vector<FieldDataWrapper> Tr512Parameter::getValues()
{
	vector<FieldDataWrapper> vec;
	vec.push_back(getValue(DEVICE_NAME_ID));
	vec.push_back(getValue(TRIGGER_POL_ID));
	vec.push_back(getValue(CLOCK_ID));
	vec.push_back(getValue(DECIMATION_ID));
	vec.push_back(getValue(N_SAMPLES_ID));
	vec.push_back(getValue(READ_TIMOUT_ID));
	vec.push_back(getValue(TRIGGER_DELAY_ID));
	vec.push_back(getValue(AMPLITUDE_ID));
	
	return vec;
}
