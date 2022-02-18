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
#ifndef FSIGNAL_HARDWARE_XML_H
#define FSIGNAL_HARDWARE_XML_H

#include <iostream>
#include <string>
#include <vector>
#include "ExtraInfoXml.h"
#include "FieldXml.h"

namespace fsignal
{	
	class HardwareXml
	{
	public:
		HardwareXml(){}
		HardwareXml(const std::string uniqueID) : uniqueID(uniqueID){}
		
                const std::string& getUniqueID() const;	
		const std::vector<std::string> &getParametersUniqueIDS() const;
		const void addParameterUniqueID(std::string parUID);
                const std::vector<FieldXml> &getFields() const;
		const void addField(FieldXml fieldXml);                
	private:		
		std::string uniqueID;
		std::vector<std::string> parUIDS;
                std::vector<FieldXml> fields;                
	};

	inline const std::string& HardwareXml::getUniqueID() const{
		return uniqueID;
	}

	inline const std::vector<std::string> &HardwareXml::getParametersUniqueIDS() const{
		return parUIDS;
	}

	inline const void HardwareXml::addParameterUniqueID(std::string parUID){		
		parUIDS.push_back(parUID);
	}
        
        inline const std::vector<FieldXml> &HardwareXml::getFields() const{
		return fields;
	}

	inline const void HardwareXml::addField(FieldXml field){            
            fields.push_back(field);
	}        
}
#endif
