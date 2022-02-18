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
#ifndef FSIGNAL_FIELD_XML_H
#define FSIGNAL_FIELD_XML_H

#include <iostream>
#include <string>
#include <vector>
#include "ExtraInfoXml.h"
#include "FieldXml.h"

namespace fsignal{	
    class FieldXml{
        public:            
            FieldXml(const std::string &uid, bool hwField = false) : uniqueID(uid), hardwareField(hwField){}
                
            const bool isHardwareField() const{
                return hardwareField;
            }
                
            const std::string &getUniqueID() const{
                return uniqueID;
            }
            
            const std::vector<fsignal::ExtraInfoXml> &getExtraInfo() const{
		return extraInfos;
            }
            
            const void addExtraInfo(ExtraInfoXml &extraInfo){		
		extraInfos.push_back(extraInfo);
            }
        private:		
            std::string uniqueID;
            std::vector<ExtraInfoXml> extraInfos;
            bool hardwareField;
    };     
}
#endif
