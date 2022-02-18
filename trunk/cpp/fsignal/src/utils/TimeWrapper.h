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
#ifndef FSIGNAL_TIME_WRAPPER_H
#define FSIGNAL_TIME_WRAPPER_H

#include "../orb/SCADUtils.hh"
#include <string>
#include <sstream>

namespace fsignal
{
    class TimeWrapper
    {
    public:
        TimeWrapper() :
            picos(0), nanos(0), micros(0),
            millis(0), seconds(0), minutes(0), hours(0){}
        TimeWrapper(const short picos,
            const short nanos,
            const short micros,
            const short millis,
            const unsigned char seconds,
            const unsigned char minutes,
            const unsigned char hours) :
            picos(picos), nanos(nanos), micros(micros),
            millis(millis), seconds(seconds), minutes(minutes), hours(hours){}
        TimeWrapper(const short millis,
            const unsigned char seconds,
            const unsigned char minutes,
            const unsigned char hours) :			
            picos(0), nanos(0), micros(0),
            millis(millis), seconds(seconds), minutes(minutes), hours(hours){}
        TimeWrapper(const org::cfn::scad::core::corba::util::Time &t) :
            picos(t.picos), nanos(t.nanos), micros(t.micros),
            millis(t.millis), seconds(t.seconds), minutes(t.minutes), hours(t.hours){}

        /**Number of seconds since January 1, 1970 UTC.*/
        TimeWrapper(time_t* tsecs)
        {
            setTime(tsecs);
            millis = 0;
            micros = 0;
            nanos = 0;
            picos = 0;
        }

        const short getPicos();
        const short getNanos();
        const short getMicros();
        const short getMillis();
        const unsigned char getSeconds();
        const unsigned char getMinutes();
        const unsigned char getHours();
        org::cfn::scad::core::corba::util::Time getAsCorbaTime();

        void setPicos(const short picos);
        void setNanos(const short nanos);
        void setMicros(const short micros);
        void setMillis(const short millis);
        void setSeconds(const unsigned char seconds);
        void setMinutes(const unsigned char minutes);
        void setHours(const unsigned char hours);
        void setTime(time_t* tsecs);
        void toTimeString(std::string& str);
        void toSimpleTimeString(std::string& str);
        
        inline void operator=(TimeWrapper &t){
            t.picos = t.getPicos();
            nanos   = t.getNanos();
            micros  = t.getMicros();
            millis  = t.getMillis();
            seconds = t.getSeconds();
            minutes = t.getMinutes();
            hours   = t.getHours();
        }
        
        inline void operator=(const org::cfn::scad::core::corba::util::Time &t){
            picos   = t.picos;
            nanos   = t.nanos;
            micros  = t.micros;
            millis  = t.millis; 
            seconds = t.seconds;
            minutes = t.minutes;
            hours   = t.hours;
        }
        
    private:
        short picos;
        short nanos;
        short micros;
        short millis;
        unsigned char seconds;
        unsigned char minutes;
        unsigned char hours;
    };

    inline const short TimeWrapper::getPicos()
    {
        return picos;
    }

    inline const short TimeWrapper::getNanos()
    {
        return nanos;
    }

    inline const short TimeWrapper::getMicros()
    {
        return micros;
    }

    inline const short TimeWrapper::getMillis()
    {
        return millis;
    }

    inline const unsigned char TimeWrapper::getSeconds()
    {
        return seconds;
    }

    inline const unsigned char TimeWrapper::getMinutes()
    {
        return minutes;
    }

    inline const unsigned char TimeWrapper::getHours()
    {
        return hours;
    }

    inline org::cfn::scad::core::corba::util::Time TimeWrapper::getAsCorbaTime()
    {
        using namespace org::cfn::scad::core::corba;
        util::Time_var t = new util::Time();
        t->picos=picos;
        t->nanos=nanos;
        t->micros=micros;
        t->millis=millis;
        t->seconds=seconds;
        t->minutes=minutes;
        t->hours=hours;

        return t._retn();
    }

    inline void TimeWrapper::setPicos(const short picos)
    {
        this->picos = picos;
    }

    inline void TimeWrapper::setNanos(const short nanos)
    {
        this->nanos = nanos;
    }

    inline void TimeWrapper::setMicros(const short micros)
    {
        this->micros = micros;
    }

    inline void TimeWrapper::setMillis(const short millis)
    {
        this->millis = millis;
    }

    inline void TimeWrapper::setSeconds(const unsigned char seconds)
    {
        this->seconds = seconds;
    }

    inline void TimeWrapper::setMinutes(const unsigned char minutes)
    {
        this->minutes = minutes;
    }

    inline void TimeWrapper::setHours(const unsigned char hours)
    {
        this->hours = hours;
    }

    inline void TimeWrapper::setTime(time_t* tsecs)
    {
        tm* nowtm = localtime(tsecs);
        seconds = nowtm->tm_sec;
        minutes = nowtm->tm_min;
        hours = nowtm->tm_hour;
    }

    inline void TimeWrapper::toTimeString(std::string& str)
    {
        std::ostringstream buffer;
        buffer << (short)hours << ":" << (short)minutes << ":" << (short)seconds << "." << (short)millis << "." << (short)micros << "." << (short)nanos;
        str = buffer.str();
    }

    inline void TimeWrapper::toSimpleTimeString(std::string& str)
    {
        std::ostringstream buffer;
        buffer << (short)hours << ":" << (short)minutes << ":" << (short)seconds;	
        str = buffer.str();
    }        
}

#endif
