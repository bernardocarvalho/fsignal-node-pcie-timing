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
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "FSParameter.h"
#include "utils/FSignalUtils.h"
#include "utils/FieldDataWrapper.h"

XERCES_CPP_NAMESPACE_USE
using namespace fsignal;
using namespace std;
using namespace org::cfn::scad::core::corba::hardware;
using namespace org::cfn::scad::core::corba::util;

string FSParameter::getXMLConfiguration(vector<wstring> toExclude)
{
	XMLPlatformUtils::Initialize();

	XMLCh* XML_ID_VALUE = XMLString::transcode("value");
	XMLCh* XML_ID_UNIQUE_ID = XMLString::transcode("uniqueID");
	XMLCh* XML_ID_ROOT = XMLString::transcode("parameterconfig");
	XMLCh* XML_ID_FIELD = XMLString::transcode("field");
	XMLCh* XML_ID_TRANSFER_FUNCTION = XMLString::transcode("transferFunction");
	XMLCh* XML_ID_FUNCTION = XMLString::transcode("function");
	XMLCh* XML_ID_UNITS = XMLString::transcode("units");
	XMLCh* XML_ID_RANGE = XMLString::transcode("Range");
	string HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\012<!DOCTYPE parameterconfig SYSTEM \"ParameterConfig.dtd\">";

	DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(XML_ID_RANGE);
 
	DOMDocument*   doc = impl->createDocument(0, XML_ID_ROOT, 0);
        DOMElement*   root = doc->getDocumentElement();
	XMLCh* uid = XMLString::transcode(getUniqueID().c_str());
	root->setAttribute(XML_ID_UNIQUE_ID, uid);
	XMLString::release(&uid);

        DOMElement* e1 = doc->createElement(XML_ID_TRANSFER_FUNCTION);
	cout << getTransferFunction() << endl;

	XMLCh* tf = XMLString::transcode(getTransferFunction().c_str());
	XMLCh* tfu = XMLString::transcode(getTransferUnits().c_str());
	e1->setAttribute(XML_ID_FUNCTION, tf);
	e1->setAttribute(XML_ID_UNITS, tfu);
        root->appendChild(e1);
	XMLString::release(&tf);
	XMLString::release(&tfu);

	vector<FieldDataWrapper> data = getValues();
	bool exclude = false;
	wchar_t* fieldUID;

	for(unsigned int i=0; i<data.size(); i++){
		exclude = false;
		fieldUID = CORBA::wstring_dup(data[i].getFieldUniqueID().c_str());

		for(unsigned int e=0; e<toExclude.size(); e++)
		{
			if(fieldUID == toExclude[e])
			{
                        	exclude = true;
                        	break;
                    	}
		}

                if(exclude)
                    continue;

		if(data[i].getValueSize() < 1)
			continue;
		
		DOMElement* field = doc->createElement(XML_ID_FIELD);
		
		XMLCh* fuid = XMLString::transcode(Utils::narrow(fieldUID).c_str());
		char val_str[data[i].getValueSize() + 1];
		vector<unsigned char> raw_val = data[i].getValue();
		for(int z=0; z<data[i].getValueSize(); z++)
			val_str[z] = raw_val[z];

		val_str[data[i].getValueSize()] = '\0';
		
		XMLCh* val = XMLString::transcode(val_str);
		field->setAttribute(XML_ID_UNIQUE_ID, fuid);
		field->setAttribute(XML_ID_VALUE, val);
		XMLString::release(&fuid);
		XMLString::release(&val);
		CORBA::wstring_free(fieldUID);
        	root->appendChild(field);
	}

	string xml;
	serializeDOM(root, xml);
	xml = HEADER + xml;

        // optionally, call release() to release the resource associated with the range after done
	DOMRange* range = doc->createRange();
	range->release();

	// done with the document, must call release() to release the entire document resources
        doc->release();	
		
	XMLString::release(&XML_ID_VALUE);
	XMLString::release(&XML_ID_UNIQUE_ID);
	XMLString::release(&XML_ID_ROOT);
	XMLString::release(&XML_ID_FIELD);
	XMLString::release(&XML_ID_TRANSFER_FUNCTION);	
	XMLString::release(&XML_ID_FUNCTION);
	XMLString::release(&XML_ID_UNITS);
	XMLString::release(&XML_ID_RANGE);

	XMLPlatformUtils::Terminate();	

	return xml;
}

string FSParameter::getXMLConfiguration()
{	
	return getXMLConfiguration(vector<wstring>());
}

int FSParameter::serializeDOM(DOMNode* node, string &dest) 
{
	XMLCh tempStr[100];
	XMLString::transcode("LS", tempStr, 99);
	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tempStr);
	DOMLSSerializer* theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMConfiguration* dc = theSerializer->getDomConfig(); 

	// optionally you can set some features on this serializer
	if (dc->canSetParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true))
		dc->setParameter(XMLUni::fgDOMWRTDiscardDefaultContent, true);

	try 
	{
		XMLCh* write = theSerializer->writeToString(node);
		char* w = XMLString::transcode(write);
		dest = w;
		XMLString::release(&w);
		XMLString::release(&write);
	}
	catch (const XMLException& toCatch) 
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << "Exception message is: \n"
			<< message << "\n";
		XMLString::release(&message);
		return -1;
	}
	catch (const DOMException& toCatch) 
	{
		char* message = XMLString::transcode(toCatch.msg);
		cout << "Exception message is: \n"
			<< message << "\n";
		XMLString::release(&message);
		return -1;
	}
	catch (...) 
	{
		cout << "Unexpected Exception \n" ;
		return -1;
	}

	theSerializer->release();
	return 0;
}
