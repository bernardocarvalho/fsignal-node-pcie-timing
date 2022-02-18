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
#include "HardwareXmlHandler.h"
#include "HardwareXml.h"

#include <xercesc/sax2/Attributes.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace fsignal;
XERCES_CPP_NAMESPACE_USE

const string HardwareXmlHandler::XML_ID_HARDWARE    = "hardware";
const string HardwareXmlHandler::XML_ID_PARAMETER   = "parameter";
const string HardwareXmlHandler::XML_ID_FIELD       = "field";
const string HardwareXmlHandler::XML_ID_UNIQUE_ID   = "uniqueID";
const string HardwareXmlHandler::XML_ID_E_INFO_ID   = "extraInfo";
const string HardwareXmlHandler::XML_ID_KEY_ID      = "key";
const string HardwareXmlHandler::XML_ID_INFO_ID     = "info";
const string HardwareXmlHandler::XML_ID_HD_FIELD_ID = "hardwareField";

void HardwareXmlHandler::startElement(const XMLCh * const uri,
        const XMLCh * const localname,
        const XMLCh * const qname,
        const Attributes& attrs) {
        
    char *xmlread = XMLString::transcode(localname);
    string read = xmlread;
    XMLCh *attrUID = XMLString::transcode(XML_ID_UNIQUE_ID.c_str());
    char *attrsReadUID = XMLString::transcode(attrs.getValue(attrUID));

    if (read == XML_ID_HARDWARE)
        xml = HardwareXml(attrsReadUID);
    else if (read == XML_ID_PARAMETER)
        xml.addParameterUniqueID(attrsReadUID);
    else if (read == XML_ID_FIELD){
        bool hwField = false;
        XMLCh *attrsHDFieldUID = XMLString::transcode(XML_ID_HD_FIELD_ID.c_str());
        char  *attrsReadHDFieldUID = XMLString::transcode(attrs.getValue(attrsHDFieldUID));
        if(attrsReadHDFieldUID != NULL){
            hwField = (strncmp(attrsReadHDFieldUID, "yes", 3) == 0);
        }
        XMLString::release(&attrsHDFieldUID);
        XMLString::release(&attrsReadHDFieldUID);                
        
        currentField = new FieldXml(attrsReadUID, hwField);        
    }
    else if (read == XML_ID_E_INFO_ID) {
        XMLCh *attrInfo = XMLString::transcode(XML_ID_INFO_ID.c_str());
        char *attrsReadInfo = XMLString::transcode(attrs.getValue(attrInfo));        
        currentField->addExtraInfo(*(new ExtraInfoXml(attrsReadUID, attrsReadInfo)));
        XMLString::release(&attrInfo);
    }

    XMLString::release(&attrUID);
    XMLString::release(&attrsReadUID);
    XMLString::release(&xmlread);
}

void HardwareXmlHandler::fatalError(const SAXParseException& exception) {
    char* message = XMLString::transcode(exception.getMessage());
    cout << "Fatal Error: " << message
            << " at line: " << exception.getLineNumber()
            << endl;
}

void HardwareXmlHandler::characters(const XMLCh * const chars, const unsigned int length) {
}

void HardwareXmlHandler::endElement(const XMLCh * const uri,
        const XMLCh * const localName,
        const XMLCh * const qName) {
    char *xmlread = XMLString::transcode(localName);
    string read = xmlread;
    if (read == XML_ID_FIELD){
        xml.addField(*currentField);
    }    
}
