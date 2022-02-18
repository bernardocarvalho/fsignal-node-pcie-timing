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
#ifndef FSIGNAL_FIELD_DATA_WRAPPER_H
#define FSIGNAL_FIELD_DATA_WRAPPER_H

#include "../orb/SCADUtils.hh"
#include "../orb/SCAD.hh"
#include "FSignalUtils.h"
#include <string>
#include <iostream>
#include <sstream>
namespace fsignal
{
	class FieldDataWrapper
	{
	public:
		FieldDataWrapper(const std::wstring fieldUniqueID, const std::string val) : fieldUniqueID(fieldUniqueID)
		{
			valueSize = val.length();
			for(int i=0; i<valueSize; i++)
				value.push_back(val[i]);
		}

		FieldDataWrapper(const std::wstring fieldUniqueID, unsigned char* val, const int valueSize) :
			fieldUniqueID(fieldUniqueID), valueSize(valueSize)
		{
			for(int i=0; i<valueSize; i++)
				value.push_back(val[i]);
		}

		FieldDataWrapper(const org::cfn::scad::core::corba::hardware::FieldData& fd)
		{
			this->fieldUniqueID = fd.fieldUniqueID;
			org::cfn::scad::core::corba::util::ByteArray ba = fd.value;
			for(unsigned int i=0; i<ba.length(); i++)
				value.push_back(ba[i]);
		}

		std::wstring getFieldUniqueID();
		std::vector<unsigned char> getValue();
		int getValueSize();
		org::cfn::scad::core::corba::hardware::FieldData_var getAsCorbaFieldData();

		void setFieldUniqueID(const std::wstring fieldUniqueID);
		void setValue(std::vector<unsigned char> value, const int valueSize);
	private:
		std::wstring fieldUniqueID;
		std::vector<unsigned char> value;
		int valueSize;
	};

	inline std::wstring FieldDataWrapper::getFieldUniqueID()
	{
		return fieldUniqueID;
	}
	
	inline std::vector<unsigned char> FieldDataWrapper::getValue()
	{
		return value;
	}	
	
	inline org::cfn::scad::core::corba::hardware::FieldData_var FieldDataWrapper::getAsCorbaFieldData()
	{
		using namespace org::cfn::scad::core::corba;
		hardware::FieldData_var fd = new hardware::FieldData();
		fd->fieldUniqueID = CORBA::wstring_dup(fieldUniqueID.c_str());

		util::ByteArray_var val = new util::ByteArray(valueSize);
		val->length(valueSize);

		for(int j = 0; j<valueSize; j++)
			val[j] = value[j];
		
		fd->value = val;
		return fd;
	}

	inline void FieldDataWrapper::setFieldUniqueID(const std::wstring fieldUniqueID)
	{
		this->fieldUniqueID = fieldUniqueID;
	}
		
	inline void FieldDataWrapper::setValue(std::vector<unsigned char> value, const int valueSize)
	{
		this->value = value;
		this->valueSize = valueSize;
	}

	inline int FieldDataWrapper::getValueSize()
	{
		return valueSize;
	}
}
#endif
