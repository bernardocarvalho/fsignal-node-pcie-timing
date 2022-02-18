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
#include "PersistParameterHandler.h"
#include "PersistParameter.h"
#include "../utils/FieldDataWrapper.h"
#include "../utils/FSignalUtils.h"

#include <xercesc/sax2/Attributes.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace fsignal;
XERCES_CPP_NAMESPACE_USE

const std::string PersistParameterHandler::XML_ID_PAR_CONFIG_ID = "parameterconfig";
const std::string PersistParameterHandler::XML_ID_UNIQUE_ID = "uniqueID";
const std::string PersistParameterHandler::XML_ID_TRANSFER_FUNCTION = "transferFunction";
const std::string PersistParameterHandler::XML_ID_FUNCTION = "function";
const std::string PersistParameterHandler::XML_ID_UNITS = "units";
const std::string PersistParameterHandler::XML_ID_FIELD = "field";
const std::string PersistParameterHandler::XML_ID_VALUE = "value";

PersistParameter& PersistParameterHandler::getPersistParameter()
{
	return persistParameter;
}

void PersistParameterHandler::startElement(	const   XMLCh* const    uri,
					const   XMLCh* const    localname,
            				const   XMLCh* const    qname,
            				const   Attributes&     attrs)
{
	char* xmlread = XMLString::transcode(localname); 
	string read = xmlread;
	if(read == XML_ID_PAR_CONFIG_ID)
	{
		XMLCh* attr = XMLString::transcode(XML_ID_UNIQUE_ID.c_str());
		char* attrRead = XMLString::transcode(attrs.getValue(attr));
		persistParameter = PersistParameter(attrRead);
		XMLString::release(&attr);
		XMLString::release(&attrRead);
	}
	else if(read == XML_ID_TRANSFER_FUNCTION)
	{
		string function;
		string units;
		XMLCh* attr = XMLString::transcode(XML_ID_FUNCTION.c_str());
		char* attrRead = XMLString::transcode(attrs.getValue(attr));	
		function = attrRead;
		XMLString::release(&attr);	
		XMLString::release(&attrRead);

		attr = XMLString::transcode(XML_ID_UNITS.c_str());
		attrRead = XMLString::transcode(attrs.getValue(attr));	
		units = attrRead;
		XMLString::release(&attr);
		XMLString::release(&attrRead);

		persistParameter.setTransferFunction(function);
		persistParameter.setTransferUnits(units);
	}
	else if(read == XML_ID_FIELD)
	{
		string id;
		string value;
		XMLCh* attr = XMLString::transcode(XML_ID_UNIQUE_ID.c_str());
		char* attrRead = XMLString::transcode(attrs.getValue(attr));	
		id = attrRead;
		XMLString::release(&attr);	
		XMLString::release(&attrRead);

		attr = XMLString::transcode(XML_ID_VALUE.c_str());
		attrRead = XMLString::transcode(attrs.getValue(attr));	
		value = attrRead;
		XMLString::release(&attr);
		XMLString::release(&attrRead);

		FieldDataWrapper fdw(Utils::widen(id), value);
		persistParameter.addField(fdw);
	}

	XMLString::release(&xmlread);
}

void PersistParameterHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    cout << "Fatal Error: " << message
         << " at line: " << exception.getLineNumber()
         << endl;
}

void PersistParameterHandler::characters (const XMLCh *const chars, const unsigned int length)
{		
}

void PersistParameterHandler::endElement(const XMLCh* const uri ,
    				const XMLCh* const localName ,
    				const XMLCh* const qName)
{
}
