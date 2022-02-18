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
#ifndef FSIGNAL_CENTRAL_TEST_PARAMETER_H
#define FSIGNAL_CENTRAL_TEST_PARAMETER_H

#include <string>
#include "../FSParameter.h"
#include "../utils/FieldDataWrapper.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"

namespace fsignal
{
    class CentralTestParameter : public FSParameter
    {
    public:
        CentralTestParameter(std::string uniqueID) : FSParameter(uniqueID), amplitude(2), tbs(1), frequency(1), npoints(100000){}
        FieldDataWrapper getValue(const std::wstring& fieldUniqueID);
        std::vector<FieldDataWrapper> getValues();
        void setValue(const FieldDataWrapper& fd);
        void setValues(const std::vector<FieldDataWrapper>& fdl);
        float getAmplitude();
        int getTBS();
        int getNPoints();
        int getFrequency();        
    private:
        float amplitude;
        int tbs;
        int frequency;
        int npoints;
        static std::wstring AMPLITUDE_ID;
        static std::wstring FREQUENCY_ID;
        static std::wstring N_POINTS_ID;
        static std::wstring TBS_ID;
    };

    inline float CentralTestParameter::getAmplitude()
    {
            return amplitude;
    }

    inline int CentralTestParameter::getTBS()
    {
            return tbs;
    }

    inline int CentralTestParameter::getNPoints()
    {
            return npoints;
    }

    inline int CentralTestParameter::getFrequency()
    {
            return frequency;
    }
}

#endif
