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
#ifndef FSIGNAL_SRTRATCA_PARAMETER_H
#define FSIGNAL_SRTRATCA_PARAMETER_H

#include <string>
#include "../FSParameter.h"
#include "../utils/FieldDataWrapper.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"

namespace fsignal{
    class SRTRATCAParameter : public FSParameter{
    public:
        SRTRATCAParameter(std::string uniqueID, int chID) : FSParameter(uniqueID) {            
            frequency           = 250;
            interleavedChannels = 1;
            acqType             = ACQ_TYPE_SINGLE;
            acqMode             = ACQ_MODE_RAW;
            triggerType         = TRG_TYPE_SOFTWARE;
            triggerMode         = TRIGGER_MODE_LEVEL;
            extFileContents     = "";
            burstPreTriggerData = 8;
            burstPulseWidth     = 8;
            burstTrgAcc         = 0;
            preTriggerTime      = 16;
            processK            = 1;
            processL            = 1;
            processM            = 1;
            processT            = 1;
            dmaByteSize         = 2048;
            dmaNBuffs           = 16;
            bufReadSize         = 4096;
            acqByteSize         = 65536;
            patchedByteSize     = 65536;
            commandCompl        = 0;
            channelID           = chID;
            logStream.str("");
        }
        FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
        std::vector<FieldDataWrapper> getValues();
        void setValue(const FieldDataWrapper& fd);
        void setValues(const std::vector<FieldDataWrapper>& fdl);        
       
        //Send to the logger the current parameter configuration
        void dumpConfiguration(); 
        double getFrequency(){
            return frequency;
        }
        
        int getInterleavedChannels(){
            return interleavedChannels;
        }
        
        std::string getAcqType(){
            return acqType;
        }
        
        std::string getAcqMode(){
            return acqMode;
        }
        
        std::string getTriggerType(){
            return triggerType;
        }

        std::string getExtFileContents(){
            return extFileContents;
        }

        std::string getTriggerMode(){
            return triggerMode;
        }

        std::string getTriggerFilter(){
            return triggerFilter;
        }

        std::string getInputFilter(){
            return inputFilter;
        }
        
        int getBurstPreTriggerData(){
            return burstPreTriggerData;
        }

        int getBurstPulseWidth(){
            return burstPulseWidth;
        }

        int getBurstTriggerAcc(){
            return burstTrgAcc;
        }
        
        int getPreTriggerTime(){
            return preTriggerTime;
        }
        
        int getProcessK(){
            return processK;
        }
        
        int getProcessL(){
            return processL;
        }
        
        int getProcessM(){
            return processM;
        }
        
        int getProcessT(){
            return processT;
        }
        
        int getCompl(){
            return commandCompl;
        }

        int getDMAByteSize(){
            return dmaByteSize;
        }
        
        int getDMANBuffs(){
            return dmaNBuffs;
        }
        
        int getBufferReadSize(){
            return bufReadSize;
        }
       
        /**
         * Return the number of events detected
         */  
        int getNumberOfEvents();

        /**
         * Return the number of events already in bytes
         */
        int getNumberOfEventsInBytes();    

        /**
         * @return the number of requested bytes for the full board
         */ 
        unsigned int getAcqByteSize(){
            return acqByteSize;
        }

        /**
         * @return the number of bytes to acquire after being patched
         * by the node to be a multiple of bufReadSize and number
         * of connected channels
         */
        unsigned int getPatchedAcqByteSize(){
            return patchedByteSize;
        }

        void setPatchedAcqByteSize(unsigned int acqBytes){
            patchedByteSize = acqBytes;
        }
        
        virtual void setConnected(bool connected);
        
        //The location of the device
        std::string devLocation;
        
        //The type of trigger
        static const std::string TRG_TYPE_SOFTWARE;
        static const std::string TRG_TYPE_HARDWARE;
        static const int         FPGA_FREQUENCY;

        //Acquisition modes
        static const std::string ACQ_MODE_RAW;
        static const std::string ACQ_MODE_PROCESSED;  
        static const std::string ACQ_MODE_CALIBRATION;
        static const std::string ACQ_MODE_BURST;

    private:
        std::stringstream logStream;
                
        double            frequency;        
        int               interleavedChannels;
        std::string       acqType;
        std::string       acqMode;
        std::string       triggerType;
        std::string       extFileContents;
        std::string       triggerMode;
        std::string       inputFilter;
        std::string       triggerFilter;
        int               preTriggerTime;
        int               processK;
        int               processL;
        int               processM;
        int               processT;
        int               dmaByteSize;
        int               dmaNBuffs;
        int               bufReadSize;
        unsigned int      acqByteSize;
        /**
         * The number of bytes to acquire (divided by the number of connected channels)
         * must be a multiple of bufReadSize
         */
        unsigned int      patchedByteSize;
        int               channelID;
        int               commandCompl;
        int               burstPreTriggerData;
        int               burstPulseWidth;
        int               burstTrgAcc;

        
        static const std::wstring FREQUENCY;        
        static const std::wstring ACQ_TYPE;
        static const std::wstring COMPL;
        static const std::wstring ILV_CHANNELS;
        static const std::wstring ACQ_MODE;
        static const std::wstring TRIGGER_TYPE;
        static const std::wstring PRE_TRIGGER_TIME;
        static const std::wstring PROCESS_K;
        static const std::wstring PROCESS_L;
        static const std::wstring PROCESS_M;
        static const std::wstring PROCESS_T;
        static const std::wstring DMA_BYTE_SIZE;
        static const std::wstring DMA_N_BUFFS;
        static const std::wstring BUFFER_READ_SIZE;
        static const std::wstring ACQ_BYTE_SIZE;
        static const std::wstring EXT_CONFIG_FILE;
        static const std::wstring BST_PRE_TRIGGER_DATA;
        static const std::wstring BST_PULSE_WIDTH;
        static const std::wstring BST_TRG_ACC;
        static const std::wstring TRIGGER_MODE;
        static const std::wstring INPUT_FILTER;
        static const std::wstring TRIGGER_FILTER;
        
        static const std::string ACQ_TYPE_SINGLE;
        static const std::string ACQ_TYPE_INTERLEAVED;
        
        //The trigger mode
        static const std::string TRIGGER_MODE_LEVEL;
        static const std::string TRIGGER_MODE_FLANK;        

        //Input and trigger filters
        static const std::string NONE;        
        static const std::string FIR;        

        bool performIOCTL(int ioctlFun, int value);
    };    
}

#endif
