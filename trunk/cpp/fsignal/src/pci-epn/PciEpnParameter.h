/***************************************************************************
 *   Copyright (C) 2006 by Andre Neto   *
 *   andre.neto@ipfn.ist.utl.pt   *
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
#ifndef FSIGNAL_PCI_EPN_PARAMETER_H
#define FSIGNAL_PCI_EPN_PARAMETER_H

#include <string>
#include "../FSParameter.h"
#include "../utils/FieldDataWrapper.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"

namespace fsignal{
    class PciEpnParameter : public FSParameter{
    public:
        PciEpnParameter(std::string uniqueID, int chID) : FSParameter(uniqueID)
                                             {            
            trigSecs         	= 1;
            trigNSecs        	= 0;
            pulseWidth          = 800000000;
            channelID           = chID;
            logStream.str("");
        }
        FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
        std::vector<FieldDataWrapper> getValues();
        void setValue(const FieldDataWrapper& fd);
        void setValues(const std::vector<FieldDataWrapper>& fdl);        
        
        int getTriggerSecs(){
            return trigSecs;
        }
 
       int getTriggerNSecs(){
            return trigNSecs;
        }
       int getPulseWidth(){
            return pulseWidth;
        }
  
        virtual void setConnected(bool connected);
        
        //The location of the device
        std::string devLocation;
        
    private:
        std::stringstream logStream;
                
        int               trigSecs;
        int               trigNSecs;
        unsigned int      pulseWidth;
        int               channelID;
       
        static const std::wstring TRIGGER_SECS;
        static const std::wstring TRIGGER_NSECS;
        static const std::wstring PULSE_NSECS;

        bool performIOCTL(int ioctlFun, int value);
    };    
}

#endif
