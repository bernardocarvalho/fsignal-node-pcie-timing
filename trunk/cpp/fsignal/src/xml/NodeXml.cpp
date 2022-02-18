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

#include "../utils/FSignalUtils.h"
#include "NodeXml.h"
#include <iostream>
using namespace std;
using namespace fsignal;

NodeXml::NodeXml(const string &uniqueID)
{	
	this->uniqueID = uniqueID;	
}

const string& NodeXml::getName(const string& locale)
{
	string loc = locale;
	Utils::toLowerCase(loc);
	map<string,string>::iterator iter = names.find(loc);
	if(iter != names.end())	
		name = iter->second;
	
	return name;
}

const string& NodeXml::getDescription(const string& locale)
{
	string loc = locale;
	Utils::toLowerCase(loc);
	map<string,string>::iterator iter = descriptions.find(loc);
	if(iter != names.end())	
		description = iter->second;
	
	return description;
}

const string& NodeXml::getName()
{	
	getName("en");
	if(name.length() == 0 && names.size() > 0)		
		name = names.begin()->second;	

	return name;
}

const string& NodeXml::getDescription()
{	
	getDescription("en");
	if(description.length() == 0 && descriptions.size() > 0)		
		description = descriptions.begin()->second;	

	return description;
}

void NodeXml::addName(const std::string& locale, const std::string& name)
{			
	string loc = locale;
	Utils::toLowerCase(loc);
	names[loc] = name;
}

void NodeXml::addDescription(const std::string& locale, const std::string& desc)
{
	string loc = locale;
	Utils::toLowerCase(loc);
	descriptions[locale] = desc;
}
