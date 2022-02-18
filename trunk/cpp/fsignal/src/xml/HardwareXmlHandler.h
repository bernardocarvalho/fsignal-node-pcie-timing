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
#ifndef FSIGNAL_HARDWARE_XMLHANDLER_H
#define FSIGNAL_HARDWARE_XMLHANDLER_H

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <string>
#include <vector>
#include <sstream>
#include "HardwareXml.h"
#include "FieldXml.h"

XERCES_CPP_NAMESPACE_USE

        namespace fsignal {

    class HardwareXmlHandler : public DefaultHandler {
    public:
        void startElement(
                const XMLCh * const uri,
                const XMLCh * const localname,
                const XMLCh * const qname,
                const Attributes& attrs
                );
        void endElement(
                const XMLCh * const uri,
                const XMLCh * const localName,
                const XMLCh * const qName
                );
        void fatalError(const SAXParseException&);
        void characters(const XMLCh * const chars, const unsigned int length);
        const HardwareXml& getHardwareXml();
    private:
        HardwareXml xml;
        FieldXml *currentField;
        static const std::string XML_ID_UNIQUE_ID;
        static const std::string XML_ID_HARDWARE;
        static const std::string XML_ID_PARAMETER;
        static const std::string XML_ID_FIELD;
        static const std::string XML_ID_E_INFO_ID;
        static const std::string XML_ID_KEY_ID;
        static const std::string XML_ID_INFO_ID;
        static const std::string XML_ID_HD_FIELD_ID;

    };

    inline const HardwareXml& HardwareXmlHandler::getHardwareXml() {
        return xml;
    }
}

#endif
