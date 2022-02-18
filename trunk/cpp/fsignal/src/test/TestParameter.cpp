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
#include "TestParameter.h"
#include "../utils/FSignalUtils.h"
#include "../utils/FieldDataWrapper.h"
#include <sstream>
#include <cmath>

using namespace org::cfn::scad::core::corba::hardware;
using namespace org::cfn::scad::core::corba;
using namespace fsignal;
using namespace std;

//CORBA::remove_ref(*ref);

wstring TestParameter::AMPLITUDE_ID = L"AMPLITUDE";
wstring TestParameter::FREQUENCY_ID = L"FREQUENCY";
wstring TestParameter::N_POINTS_ID = L"N_POINTS";
wstring TestParameter::TBS_ID = L"TBS";

FieldDataWrapper TestParameter::getValue(const wstring &fieldUniqueID)
{
	stringstream val;
	if(fieldUniqueID == AMPLITUDE_ID)
		val << getAmplitude();
	else if(fieldUniqueID == FREQUENCY_ID)
		val << getFrequency();
	else if(fieldUniqueID == N_POINTS_ID)
		val << getNPoints();
        else if(fieldUniqueID == TBS_ID)
		val << getTBS();	

	return FieldDataWrapper(fieldUniqueID, val.str());
}

void TestParameter::setValue(const FieldDataWrapper &fd)
{
	FieldDataWrapper fdw = fd;
	wstring fieldUniqueID = fdw.getFieldUniqueID();
	vector<unsigned char> vec_str = fdw.getValue();
	char val_str[vec_str.size() + 1];
	for(unsigned int i=0; i< vec_str.size(); i++)
		val_str[i] = vec_str[i];	

	stringstream vals(val_str);
	if(fieldUniqueID == AMPLITUDE_ID)
                vals >> amplitude;
	else if(fieldUniqueID == FREQUENCY_ID)
		vals >> frequency;
	else if(fieldUniqueID == N_POINTS_ID)
		vals >> npoints;
	else if(fieldUniqueID == TBS_ID)
                vals >> tbs;
}

void TestParameter::setValues(const vector<FieldDataWrapper> &fdl)
{
	for(unsigned int i=0; i<fdl.size(); i++)
		setValue(fdl[i]);
}

vector<FieldDataWrapper> TestParameter::getValues()
{
	vector<FieldDataWrapper> vec;
	vec.push_back(getValue(AMPLITUDE_ID));
	vec.push_back(getValue(FREQUENCY_ID));
	vec.push_back(getValue(N_POINTS_ID));
	vec.push_back(getValue(TBS_ID));
	
	return vec;
}

