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
#ifndef JET_HTTP_INTERFACE_H
#define JET_HTTP_INTERFACE_H

#include <cstdio>
#include <sstream>
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "log4cxx/propertyconfigurator.h"
#include "FSignalUtils.h"
#include "HardwareInterface.h"

#define PLAIN_TEXT      "text/plain"
#define XML_TEXT        "text/xml"
#define HTML_TEXT       "text/html"
#define DATA_STREAM     "application/octet-stream"

namespace fsignal{    
    class HTTPInterface{
    public:
        HTTPInterface(int port, HardwareInterface *hwInterface){
            setHardware(hwInterface);
            initHTTPInterface(port);
        }
        static void logMessage(std::stringstream &msg);
        static void initLogger(const char *loggerFileLocation = NULL);
    private:
        void setHardware(HardwareInterface *hwInterface);
        void initHTTPInterface(int port);        
    };
    
    inline void HTTPInterface::logMessage(std::stringstream& msg){
        using namespace log4cxx;
        using namespace log4cxx::helpers;

        LOG4CXX_INFO(Utils::getLogger(), msg.str());
    }
}

#endif
