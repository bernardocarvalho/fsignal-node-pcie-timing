/**
 *
 * @file PcieTimingParameter.h
 * @author Bernardo Carvalho
 * @date 2017-10-17
 * @brief .
 *
 *
 * Copyright 2006-2017 IPFN-Instituto Superior Tecnico, Portugal
 *
 * Licensed under the EUPL, Version 1.2 or - as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 *   writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

#ifndef FSIGNAL_PCIE_TIMING_PARAMETER_H
#define FSIGNAL_PCIE_TIMING_PARAMETER_H

#include <string>
#include "../FSParameter.h"
#include "../utils/FieldDataWrapper.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"

namespace fsignal{
    class PcieTimingParameter : public FSParameter{
        public:
            PcieTimingParameter(std::string uniqueID, int chID) : FSParameter(uniqueID)
            {
                delay = 1;
                period100nS = 0;
                numPulses  = 1;
                delayMultiplierUnit = UNIT_100_NANO;
                channelMode = CHANNEL_MODE_TRIGGER;
                channelMode   = CHANNEL_MODE_TRIGGER;
                channelInvert = CHANNEL_INVERT_NO;
                channelID  = chID;
                logStream.str("");
            }
            FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
            std::vector<FieldDataWrapper> getValues();
            void setValue(const FieldDataWrapper& fd);
            void setValues(const std::vector<FieldDataWrapper>& fdl);

            unsigned int getDelayTime100nS(){
                return getDelayTime()*getDelayMultiplier();
            }

            int getDelayTime(){
                return delay;
            }


            std::string getDelayMultiplierUnit(){
                return delayMultiplierUnit;
            }

            unsigned int getPeriodTime100nS(){
                return period100nS;
            }

            unsigned int getNumPulses(){
                return numPulses;
            }

            std::string getChannelMode(){
                return channelMode;
            }

            std::string getChannelInvert(){
                return channelInvert;
            }

            virtual void setConnected(bool connected);

            virtual unsigned int getDelayMultiplier();

            //The location of the device
            std::string devLocation;
            //The type of channel mode
            static const std::string CHANNEL_MODE_RESET;
            static const std::string CHANNEL_MODE_TRIGGER;
            static const std::string CHANNEL_MODE_SQUARE;
            static const std::string CHANNEL_MODE_PULSE_TRAIN;
            //Delay Units
            static const std::string UNIT_100_NANO;
            static const std::string UNIT_MICRO;
            static const std::string UNIT_MILLI;
            static const std::string UNIT_SECONDS;

            static const std::string CHANNEL_INVERT_NO;
            static const std::string CHANNEL_INVERT_YES;

        private:
            std::stringstream logStream;

            int               delay;
            std::string       delayMultiplierUnit;
            unsigned int      period100nS;
            unsigned int      numPulses;
            std::string       channelMode;
            std::string       channelInvert;

            int               channelID;

            static const std::wstring DELAY_TIME;
            static const std::wstring DELAY_MULTIPLIER_UNIT;
            static const std::wstring PERIOD_TIME_100NS;
            static const std::wstring NUM_PULSES;
            static const std::wstring CHANNEL_MODE;
            static const std::wstring CHANNEL_INVERT;

            bool performIOCTL(int ioctlFun, int value);
    };
}

#endif
