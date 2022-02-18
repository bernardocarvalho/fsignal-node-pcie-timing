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
#ifndef FSIGNAL_REFLECT_PARAMETER_HPP
#define FSIGNAL_REFLECT_PARAMETER_HPP

#include <string>
#include "../FSParameter.h"
#include "../utils/FieldDataWrapper.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"

namespace fsignal
{
    class ReflectParameter : public FSParameter
    {
    public:
        ReflectParameter(std::string uniqueID) : FSParameter(uniqueID), mode(0), trigger(false), freq(9000000), minfreq(9000000), maxfreq(13500000), sweepRate(10), hwresp(false) {}
        FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
        std::vector<FieldDataWrapper> getValues();
        void setValue(const FieldDataWrapper& fd);
        void setValues(const std::vector<FieldDataWrapper>& fdl);
        int getMode();
        bool isTriggerEnabled();
        unsigned long getFrequency();
        unsigned long getMinFrequency();
        unsigned long getMaxFrequency();
        unsigned long getSweepRate();
        bool isHwResponding();
        std::string getTriggerStatus();
        std::string getHwResponse();
        
    private:
        int mode;
        bool trigger;				// trigger enabled
        unsigned long freq;			// fixed frequency
        unsigned long minfreq;		// starting sweep frequency
        unsigned long maxfreq;		// ending sweep frequency
        unsigned short sweepRate;	// sweep rate
        bool hwresp;				// is hardware responding?
        static std::wstring MODE_ID;
        static std::wstring TRIGGER_ID;
        static std::wstring FREQUENCY_ID;
        static std::wstring MINFREQ_ID;
        static std::wstring MAXFREQ_ID;
        static std::wstring SWEEP_RATE_ID;
        static std::wstring HW_RESPONDING_ID;
    };

	inline int ReflectParameter::getMode()
	{
		return mode;
	}
	
	inline bool ReflectParameter::isTriggerEnabled()
	{
		return trigger;
	}
	
	inline unsigned long ReflectParameter::getFrequency()
	{
		return freq;
	}
	
	inline unsigned long ReflectParameter::getMinFrequency()
	{
		return minfreq;
	}
	
	inline unsigned long ReflectParameter::getMaxFrequency()
	{
		return maxfreq;
	}
	
	inline unsigned long ReflectParameter::getSweepRate()
	{
		return sweepRate;
	}
	
	inline bool ReflectParameter::isHwResponding()
	{
		return hwresp;
	}
}

#endif
