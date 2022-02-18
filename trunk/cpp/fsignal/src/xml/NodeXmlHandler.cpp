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
#include "NodeXmlHandler.h"
#include "NodeXml.h"

#include <xercesc/sax2/Attributes.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace fsignal;
XERCES_CPP_NAMESPACE_USE

const string NodeXmlHandler::XML_ID_NODE = "node";
const string NodeXmlHandler::XML_ID_NAME = "name";
const string NodeXmlHandler::XML_ID_DESCRIPTION = "description";
const string NodeXmlHandler::XML_ID_UNIQUE_ID = "uniqueID";
const string NodeXmlHandler::XML_ID_LANG = "xml:lang";

const NodeXml& NodeXmlHandler::getNodeXML()
{
	return nodeXml;
}

void NodeXmlHandler::startElement(	const   XMLCh* const    uri,
					const   XMLCh* const    localname,
            				const   XMLCh* const    qname,
            				const   Attributes&     attrs)
{
	char* xmlread = XMLString::transcode(localname); 
	string read = xmlread;	
	if(read == XML_ID_NODE)
	{
		XMLCh* attr = XMLString::transcode(XML_ID_UNIQUE_ID.c_str());
		char* attrRead = XMLString::transcode(attrs.getValue(attr));
		nodeXml = NodeXml(attrRead);
		XMLString::release(&attr);
		XMLString::release(&attrRead);
	}
	else if(read == XML_ID_NAME)
		currentTag = XML_ID_NAME;		
	else if(read == XML_ID_DESCRIPTION)
		currentTag = XML_ID_DESCRIPTION;		

	if(read == XML_ID_NAME || read == XML_ID_DESCRIPTION)
	{
		XMLCh* attr = XMLString::transcode(XML_ID_LANG.c_str());
		char* attrRead = XMLString::transcode(attrs.getValue(attr));	
		currentLanguage = attrRead;
		XMLString::release(&attr);
		XMLString::release(&attrRead);
	}

	XMLString::release(&xmlread);
}

void NodeXmlHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    cout << "Fatal Error: " << message
         << " at line: " << exception.getLineNumber()
         << endl;
}

void NodeXmlHandler::characters (const XMLCh *const chars, const unsigned int length)
{		
	char* read = XMLString::transcode(chars);
	buffer.write(read, length);
	XMLString::release(&read);
}

void NodeXmlHandler::endElement(const XMLCh* const uri ,
    				const XMLCh* const localName ,
    				const XMLCh* const qName)
{
	if(currentTag == XML_ID_NAME)
		nodeXml.addName(currentLanguage, buffer.str());
	else if(currentTag == XML_ID_DESCRIPTION)
		nodeXml.addDescription(currentLanguage, buffer.str());
	
	buffer.str("");
}
