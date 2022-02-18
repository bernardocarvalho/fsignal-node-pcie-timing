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
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include "XmlParser.h"
#include <iostream>

XERCES_CPP_NAMESPACE_USE
using namespace std;
using namespace fsignal;

void XmlParser::parse(const char* xmlFile, DefaultHandler& handler, bool useXMLDTD)
{
	try
	{
		XMLPlatformUtils::Initialize();
        }
        catch (const XMLException& toCatch)
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << "Error during initialization! :\n";
		cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return;
        }

	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
	parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);
	XMLCh* valid = XMLString::transcode("http://xml.org/sax/features/validation");
	parser->setFeature(valid, useXMLDTD);
	XMLString::release(&valid);
	parser->setFeature(XMLUni::fgXercesLoadExternalDTD, useXMLDTD);

	parser->setContentHandler(&handler);
        parser->setErrorHandler(&handler);

        try
	{
		parser->parse(xmlFile);
        }
        catch (const XMLException& toCatch)
	{ 
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return;
        }
        catch (const SAXParseException& toCatch)
	{
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
		return;
        }
	catch (...)
	{
		cout << "Unexpected Exception \n" ;
		return;
        }

        delete parser;
	XMLPlatformUtils::Terminate();
}
