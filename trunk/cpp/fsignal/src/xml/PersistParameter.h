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
#ifndef FSIGNAL_PERSIST_PARAMETER_XML_H
#define FSIGNAL_PERSIST_PARAMETER_XML_H

#include <string>
#include <vector>
#include "../utils/FieldDataWrapper.h"

namespace fsignal
{	
	class PersistParameter
	{
	public:
		PersistParameter(){};
		PersistParameter(const std::string& parUniqueID) :
			parameterUniqueID(parUniqueID){}
		std::vector<FieldDataWrapper> getFields();
		void addField(FieldDataWrapper& fdw);
		std::string getParameterUniqueID();
		std::string getTransferFunction();
		std::string getTransferUnits();
		void setTransferFunction(std::string& tf);
		void setTransferUnits(std::string& units);
	private:
		std::string parameterUniqueID;
		std::string transferFunction;
		std::string units;
		std::vector<FieldDataWrapper> fields;
	};

	inline std::vector<FieldDataWrapper> PersistParameter::getFields()
	{
		return fields;
	}

	inline void PersistParameter::addField(FieldDataWrapper& fdw)
	{
		fields.push_back(fdw);
	}

	inline std::string PersistParameter::getParameterUniqueID()
	{
		return parameterUniqueID;
	}

	inline std::string PersistParameter::getTransferFunction()
	{
		return transferFunction;
	}
		
	inline std::string PersistParameter::getTransferUnits()
	{
		return units;
	}
		
	inline void PersistParameter::setTransferFunction(std::string& tf)
	{
		this->transferFunction = tf;
	}
		
	inline void PersistParameter::setTransferUnits(std::string& units)
	{
		this->units = units;
	}
}
#endif
