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

#ifndef FSIGNAL_PARAMETER_H
#define FSIGNAL_PARAMETER_H

#include <string>
#include <vector>
#include "utils/FieldDataWrapper.h"
#include "utils/EventWrapper.h"
#include "orb/SCAD.hh"
#include "orb/SCADUtils.hh"
#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_USE

namespace fsignal
{
    class FSParameter
    {
    public:			
        FSParameter(std::string uniqueID) : uniqueID(uniqueID), connected(true), tfunction(""), units(""){}
        virtual ~FSParameter(){}
        virtual bool isConnected();
        virtual void setConnected(bool connected);
        std::string getUniqueID();			
        std::string getXMLConfiguration();
        std::string& getTransferFunction();			
        std::string& getTransferUnits();
        void setTransferFunction(const std::string& tfunction);			
        void setTransferUnits(const std::string& units);
        virtual FieldDataWrapper getValue(const std::wstring& fieldUniqueID) = 0;
        virtual std::vector<FieldDataWrapper> getValues() = 0;
        virtual void setValue(const FieldDataWrapper& fd) = 0;
        virtual void setValues(const std::vector<FieldDataWrapper>& fdl) = 0;       
    private:                
        std::string uniqueID; 
        bool connected;
        std::string tfunction;
        std::string units;
        
        int serializeDOM(DOMNode* node, std::string &dest); 

    protected:
        std::string getXMLConfiguration(std::vector<std::wstring> toExclude);
    };

    inline bool FSParameter::isConnected()
    {
        return connected;
    }

    inline std::string FSParameter::getUniqueID()
    {
        return uniqueID;
    }

    inline std::string& FSParameter::getTransferFunction()
    {
        return tfunction;
    }

    inline std::string& FSParameter::getTransferUnits()
    {
        return units;
    }

    inline void FSParameter::setConnected(bool connected)
    {
        this->connected = connected;
    }

    inline void FSParameter::setTransferFunction(const std::string& tfunction)
    {
        this->tfunction = tfunction;
    }

    inline void FSParameter::setTransferUnits(const std::string& units)
    {
        this->units = units;
    }
}
#endif
