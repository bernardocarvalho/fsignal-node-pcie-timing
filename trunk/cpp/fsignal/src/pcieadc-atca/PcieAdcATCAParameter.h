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
#ifndef FSIGNAL_PCIE_ATCA_PARAMETER_H
#define FSIGNAL_PCIE_ATCA_PARAMETER_H

#include <string>
#include "../FSParameter.h"
#include "../utils/FieldDataWrapper.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"

namespace fsignal{
    class PcieAdcATCAParameter : public FSParameter{
    public:
        PcieAdcATCAParameter(std::string uniqueID, int chID) : FSParameter(uniqueID)
                                             {            
            dmaByteSize         = 4096;
            acqFrequency        = 2e6;
            memSize             = 536870911;
            acqTimeMillis       = 0.5;
            delayTimeMillis     = 0;
            trigDelayMicros     = 0;
            triggerType         = TRG_TYPE_SOFTWARE;
            channelID           = chID;
            logStream.str("");
        }
        FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
        std::vector<FieldDataWrapper> getValues();
        void setValue(const FieldDataWrapper& fd);
        void setValues(const std::vector<FieldDataWrapper>& fdl);        
        
        int getDMAByteSize(){
            return dmaByteSize;
        }
        
        float getAcquisitionFrequency(){
            return acqFrequency;
        }
        
        unsigned int getMemSize(){
            return memSize;
        }

        unsigned int getChannelOffset(){
            return (1 << 24);
        }

        double getAcqTimeMillis(){
            return acqTimeMillis;
        }
        
        std::string getTriggerType(){
	        return triggerType; 
        }
        
        int getDelayTimeMillis(){
            return delayTimeMillis;
        }
        int getTrigDelayMicros(){
            return trigDelayMicros;
        }

        virtual void setConnected(bool connected);
        
        //The location of the device
        std::string devLocation;
        
        //The type of trigger
        static const std::string TRG_TYPE_SOFTWARE;
        static const std::string TRG_TYPE_HARDWARE;
    private:
        std::stringstream logStream;
                
        std::string       triggerType;
        int               dmaByteSize;
        float             acqFrequency;
        unsigned int      memSize;
        double            acqTimeMillis;
        int               delayTimeMillis;
	int               trigDelayMicros;
        int               channelID;
        
        static const std::wstring TRIGGER_TYPE;
        static const std::wstring DMA_BYTE_SIZE;
        static const std::wstring FREQUENCY;
        static const std::wstring MEM_SIZE;
        static const std::wstring ACQ_TIME_MILLIS;
        static const std::wstring DELAY_TIME_MILLIS;
        static const std::wstring TRIGGER_DELAY_MICROS;
        
        bool performIOCTL(int ioctlFun, int value);
    };    
}

#endif
