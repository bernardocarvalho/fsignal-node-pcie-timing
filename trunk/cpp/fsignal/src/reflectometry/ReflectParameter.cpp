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
#include "ReflectParameter.hpp"
#include "../utils/FSignalUtils.h"
#include "../utils/FieldDataWrapper.h"
#include <sstream>
#include <cmath>

using namespace org::cfn::scad::core::corba::hardware;
using namespace org::cfn::scad::core::corba;
using namespace fsignal;
using namespace std;

//CORBA::remove_ref(*ref);

wstring ReflectParameter::MODE_ID = L"MODE";
wstring ReflectParameter::TRIGGER_ID = L"TRIGGER";
wstring ReflectParameter::FREQUENCY_ID = L"FREQUENCY";
wstring ReflectParameter::MINFREQ_ID = L"MIN_FREQ";
wstring ReflectParameter::MAXFREQ_ID = L"MAX_FREQ";
wstring ReflectParameter::SWEEP_RATE_ID = L"SWEEP_RATE";
wstring ReflectParameter::HW_RESPONDING_ID = L"HW_RESPONDING";

string ReflectParameter::getTriggerStatus()
{
	if(isTriggerEnabled())
		return "enabled";
	
	return "disabled";
}

string ReflectParameter::getHwResponse()
{
	if(isHwResponding())
		return "responding";
		
	return "not responding";
}

FieldDataWrapper ReflectParameter::getValue(const wstring &fieldUniqueID)
{
	stringstream val;
	if(fieldUniqueID == MODE_ID)
		val << getMode();
	else if(fieldUniqueID == TRIGGER_ID)
		val << getTriggerStatus();
	else if(fieldUniqueID == FREQUENCY_ID)
		val << getFrequency();
	else if(fieldUniqueID == MINFREQ_ID)
		val << getMinFrequency();
	else if(fieldUniqueID == MAXFREQ_ID)
		val << getMaxFrequency();
	else if(fieldUniqueID == SWEEP_RATE_ID)
		val << getSweepRate();
	else if(fieldUniqueID == HW_RESPONDING_ID)
		val << getHwResponse();

	return FieldDataWrapper(fieldUniqueID, val.str());
}

void ReflectParameter::setValue(const FieldDataWrapper &fd)
{
	FieldDataWrapper fdw = fd;
	wstring fieldUniqueID = fdw.getFieldUniqueID();
	vector<unsigned char> vec_str = fdw.getValue();
	char val_str[vec_str.size() + 1];
	for(unsigned int i=0; i< vec_str.size(); i++)
		val_str[i] = vec_str[i];	

	stringstream vals(val_str);
	if(fieldUniqueID == MODE_ID)
		vals >> mode;
	else if(fieldUniqueID == TRIGGER_ID)
		trigger = (val_str[0] == 'e');
	else if(fieldUniqueID == FREQUENCY_ID)
		vals >> freq;
	else if(fieldUniqueID == MINFREQ_ID)
		vals >> minfreq;
	else if(fieldUniqueID == MAXFREQ_ID)
		vals >> maxfreq;
	else if(fieldUniqueID == SWEEP_RATE_ID)
		vals >> sweepRate;
	else if(fieldUniqueID == HW_RESPONDING_ID)
		hwresp = (val_str[0] == 'r');
}

void ReflectParameter::setValues(const vector<FieldDataWrapper> &fdl)
{
	for(unsigned int i=0; i<fdl.size(); i++)
		setValue(fdl[i]);
}

vector<FieldDataWrapper> ReflectParameter::getValues()
{
	vector<FieldDataWrapper> vec;
	vec.push_back(getValue(MODE_ID));
	vec.push_back(getValue(TRIGGER_ID));
	vec.push_back(getValue(FREQUENCY_ID));
	vec.push_back(getValue(MINFREQ_ID));
	vec.push_back(getValue(MAXFREQ_ID));
	vec.push_back(getValue(SWEEP_RATE_ID));
	vec.push_back(getValue(HW_RESPONDING_ID));
	
	return vec;
}

