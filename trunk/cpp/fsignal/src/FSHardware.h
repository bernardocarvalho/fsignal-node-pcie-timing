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

#ifndef FSIGNAL_HARDWARE_H
#define FSIGNAL_HARDWARE_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "orb/SCAD.hh"
#include "orb/SCADUtils.hh"
#include "utils/FieldDataWrapper.h"
#include "FSParameter.h"
#include "xml/FieldXml.h"

namespace fsignal
{
	class FSHardware
	{
	public:		
		FSHardware(std::string uniqueID, const std::vector<FieldXml> xmlFlds) : 
			uniqueID(uniqueID), 
			connected(true), 
                        xmlFields(xmlFlds),
			status(org::cfn::scad::core::corba::hardware::STOPPED)
			{                    
                        }
		FSHardware(std::string uniqueID, const std::vector<FieldXml> xmlFlds, std::map<std::string, FSParameter*> parametersTable) : 
			uniqueID(uniqueID), 
			connected(true),
                        xmlFields(xmlFlds),
			status(org::cfn::scad::core::corba::hardware::STOPPED), parametersTable(parametersTable)
			{}
                virtual ~FSHardware(){}
		virtual bool isConnected();
		virtual void setConnected(bool connected);
		void addParameter(FSParameter* parameter);
		std::string getUniqueID();
		org::cfn::scad::core::corba::hardware::FieldData* getValue(const std::string& parameterUniqueID, const std::wstring& fieldUniqueID);
		org::cfn::scad::core::corba::hardware::FieldDataList* getValues(const std::string& parameterUniqueID);
		void setValue(const std::string& parameterUniqueID, const org::cfn::scad::core::corba::hardware::FieldData& fd);
		void setValues(const std::string& parameterUniqueID, const org::cfn::scad::core::corba::hardware::FieldDataList& fdl);
		FSParameter* getParameter(const std::string& parameterUniqueID);
		std::vector<FSParameter*> getAllParameters();
		const std::map<std::string, FSParameter*> getParametersTable();
		int getNumberOfParameters();
		org::cfn::scad::core::corba::hardware::HardwareStatus getStatus();
		virtual void changeStatus(org::cfn::scad::core::corba::hardware::HardwareStatus newStatus);
		std::string getTransferFunction(const std::string& parameterUniqueID);			
		std::string getTransferUnits(const std::string& parameterUniqueID);
		void setTransferFunction(const std::string& parameterUniqueID, std::string tfunction);			
		void setTransferUnits(const std::string& parameterUniqueID, std::string units);
                
        bool isHardwareField(std::wstring &fieldUID){                    
            for(unsigned int i=0; i<xmlFields.size(); i++){
                std::string fuid(fieldUID.begin(), fieldUID.end());
                if(xmlFields[i].getUniqueID() == fuid){
                    return xmlFields[i].isHardwareField();
                }
            }
            return false;
        }
	private:
		std::string uniqueID;
        bool connected;
        const std::vector<FieldXml> xmlFields;		
		org::cfn::scad::core::corba::hardware::HardwareStatus status;
        std::map<std::string, FSParameter*> parametersTable;
	};

	inline void FSHardware::addParameter(FSParameter* parameter){
            parametersTable[parameter->getUniqueID()] = parameter;
	}

	inline bool FSHardware::isConnected(){
            return connected;
	}
	
	inline std::string FSHardware::getUniqueID(){
            return uniqueID;
	}

	inline void FSHardware::setConnected(bool connected){
            this->connected = connected;
	}

	inline org::cfn::scad::core::corba::hardware::HardwareStatus FSHardware::getStatus(){
		return status;
	}

	inline FSParameter* FSHardware::getParameter(const std::string& parameterUniqueID){
        return parametersTable[parameterUniqueID];
	}

	inline std::vector<FSParameter*> FSHardware::getAllParameters(){
            std::vector<FSParameter*> pars;
            std::map<std::string, FSParameter*>::iterator iter;
            int i=0;
            for(iter = parametersTable.begin(); iter != parametersTable.end(); iter++, i++)
                pars.push_back(iter->second);
		
            return pars;
	}

	inline int FSHardware::getNumberOfParameters(){
            return parametersTable.size();
	}
		
	inline void FSHardware::changeStatus(org::cfn::scad::core::corba::hardware::HardwareStatus newStatus){
            if(newStatus == org::cfn::scad::core::corba::hardware::CONFIGURE)
                    status = org::cfn::scad::core::corba::hardware::CONFIGURED;
            else if(newStatus == org::cfn::scad::core::corba::hardware::RUN)
                    status = org::cfn::scad::core::corba::hardware::RUNNING;
            else if(newStatus == org::cfn::scad::core::corba::hardware::STOP)
                    status = org::cfn::scad::core::corba::hardware::STOPPED;
            else
                    status = newStatus;
	}

	inline org::cfn::scad::core::corba::hardware::FieldData* FSHardware::getValue(const std::string& parameterUniqueID, const std::wstring& fieldUniqueID){
            return (*getParameter(parameterUniqueID)).getValue(fieldUniqueID).getAsCorbaFieldData()._retn();
	}

	inline org::cfn::scad::core::corba::hardware::FieldDataList* FSHardware::getValues(const std::string& parameterUniqueID){
            using namespace org::cfn::scad::core::corba::hardware;
            std::vector<FieldDataWrapper> vec = (*getParameter(parameterUniqueID)).getValues();
            FieldDataList_var fdl= new FieldDataList(vec.size());
            fdl->length(vec.size());
            for(unsigned int i=0; i<vec.size(); i++){
                fdl[i] = vec[i].getAsCorbaFieldData();
            }

            return fdl._retn();
	}

	inline void FSHardware::setValue(const std::string& parameterUniqueID, const org::cfn::scad::core::corba::hardware::FieldData& fd){
            FieldDataWrapper fdw(fd);
            std::wstring fuid = fdw.getFieldUniqueID();
            
            if(isHardwareField(fuid)){
		std::map<std::string, FSParameter*>::iterator iter;		
                for(iter = parametersTable.begin(); iter != parametersTable.end(); iter++){
                    iter->second->setValue(fdw);
                }
            }else{
                (*getParameter(parameterUniqueID)).setValue(fdw);
            }
	}
		
	inline void FSHardware::setValues(const std::string& parameterUniqueID, const org::cfn::scad::core::corba::hardware::FieldDataList& fdl){
            for(unsigned int i=0; i<fdl.length(); i++){
                setValue(parameterUniqueID, fdl[i]);   
            }
	}

	inline std::string FSHardware::getTransferFunction(const std::string& parameterUniqueID){
            return (*getParameter(parameterUniqueID)).getTransferFunction();
	}			
	
	inline std::string FSHardware::getTransferUnits(const std::string& parameterUniqueID){
            return (*getParameter(parameterUniqueID)).getTransferUnits();
	}

	inline void FSHardware::setTransferFunction(const std::string& parameterUniqueID, std::string tfunction){
            (*getParameter(parameterUniqueID)).setTransferFunction(tfunction);
	}
	
	inline void FSHardware::setTransferUnits(const std::string& parameterUniqueID, std::string units){
            (*getParameter(parameterUniqueID)).setTransferUnits(units);
	}

	inline const std::map<std::string, FSParameter*> FSHardware::getParametersTable(){
            return parametersTable;
	}
}

#endif
