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
#ifndef FSIGNAL_EXTRA_INFO_XML_H
#define FSIGNAL_EXTRA_INFO_XML_H

#include <iostream>
#include <string>
#include <vector>

namespace fsignal
{	
	class ExtraInfoXml
	{
	public:
		ExtraInfoXml(){}
		ExtraInfoXml(const std::string &uniqueID, const std::string &info) : uniqueID(uniqueID), info(info){}
		
                const std::string &getUniqueID() const;					
                const std::string &getInfo() const;		
	private:
		std::string uniqueID;
                std::string info;
	};

	inline const std::string &ExtraInfoXml::getUniqueID() const{
		return uniqueID;
	}

	inline const std::string &ExtraInfoXml::getInfo() const{
		return info;
	}
}
#endif
