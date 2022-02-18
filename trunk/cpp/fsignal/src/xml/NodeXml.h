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
#ifndef FSIGNAL_NODE_XML_H
#define FSIGNAL_NODE_XML_H

#include <string>
#include <map>

namespace fsignal
{	
	class NodeXml
	{
	public:
		NodeXml(){};
		NodeXml(const std::string& uniqueID);
		//inline
		const std::string& getUniqueID() const
		{
			return uniqueID;
		}
		const std::string& getName();
		const std::string& getName(const std::string& locale);
		const std::string& getDescription();
		const std::string& getDescription(const std::string& locale);
		void addName(const std::string& locale, const std::string& name);		
		void addDescription(const std::string& locale, const std::string& desc);		
	private:		
		std::string uniqueID;
		std::map<std::string, std::string> names;
		std::map<std::string, std::string> descriptions;
		std::string name;	
		std::string description;				
	};
}
#endif
