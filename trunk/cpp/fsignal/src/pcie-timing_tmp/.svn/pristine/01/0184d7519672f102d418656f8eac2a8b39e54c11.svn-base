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
            delay10nS = 1;
            period10nS        = 0;
            config = 0;
            channelID           = chID;
            logStream.str("");
        }
            FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
            std::vector<FieldDataWrapper> getValues();
            void setValue(const FieldDataWrapper& fd);
            void setValues(const std::vector<FieldDataWrapper>& fdl);

            int getDelayTime10nS(){
                return delay10nS;
            }

            int getPeriodTime10nS(){
                return period10nS;
            }
            int getConfig(){
                return config;
            }

            virtual void setConnected(bool connected);

            //The location of the device
            std::string devLocation;

        private:
            std::stringstream logStream;

            int               delay10nS;
            int               period10nS;
            unsigned int      config;
            int               channelID;

            static const std::wstring DELAY_TIME_10NS;
            static const std::wstring PERIOD_TIME_10NS;
            static const std::wstring CONFIG;

            bool performIOCTL(int ioctlFun, int value);
    };
}

#endif
