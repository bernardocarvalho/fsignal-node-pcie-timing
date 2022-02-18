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
#ifndef FSIGNAL_TR512_PARAMETER_H
#define FSIGNAL_TR512_PARAMETER_H

#include <string>
#include "../FSParameter.h"
#include "../utils/FieldDataWrapper.h"

namespace fsignal
{
	class Tr512Parameter : public FSParameter
	{
	public:
	  Tr512Parameter(std::string uniqueID) : FSParameter(uniqueID), 
	    triggerpol(0), clock(0), decimation(1), nsamples(4096), amplitude(10.0),
	    rdTmOut(30), trigDelay(0){}
		FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
		std::vector<FieldDataWrapper> getValues();
		void setValue(const FieldDataWrapper& fd);
		void setValues(const std::vector<FieldDataWrapper>& fdl);
		char *getDevName();
		  //std::string getDevName();
		float getAmplitude();

		int getTriggerPol();
		int getClock();
		int getDecimation();
		int getNSamples();
		int getRdTmOut();
		int getTrgDlay();
		//		int getTBS();
	private:

		//		std::string dev_name;
		char dev_name[100];
		int triggerpol;
		int clock;
		int decimation;
		int nsamples;
		int rdTmOut;
		// non-hardware Parameters
		int trigDelay;
		float amplitude;
		static std::wstring DEVICE_NAME_ID;
		static std::wstring TRIGGER_POL_ID;
		static std::wstring CLOCK_ID;
		static std::wstring DECIMATION_ID;
		static std::wstring N_SAMPLES_ID;
		static std::wstring READ_TIMOUT_ID;
		static std::wstring TRIGGER_DELAY_ID;
		static std::wstring AMPLITUDE_ID;
		//		static std::wstring TBS_ID;
	};

  ///	inline std::string Tr512Parameter::getDevName()
  //	{
  //		return dev_name;
  //	}
	inline char * Tr512Parameter::getDevName()
	{
		return dev_name;
	}

	/*	inline int Tr512Parameter::getTBS()
	{
		return tbs;
		}*/

	inline int Tr512Parameter::getTriggerPol()
	{
		return triggerpol;
	}

	inline int Tr512Parameter::getClock()
	{
		return clock;
	}
	inline int Tr512Parameter::getDecimation()
	{
		return decimation;
	}
	inline int Tr512Parameter::getNSamples()
	{
		return nsamples;
	}
	inline int Tr512Parameter::getRdTmOut()
	{
		return rdTmOut;
	}
	inline int Tr512Parameter::getTrgDlay()
	{
		return trigDelay;
	}
	inline float Tr512Parameter::getAmplitude()
	{
		return amplitude;
	}
}

#endif
