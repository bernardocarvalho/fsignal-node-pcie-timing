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
#ifndef FSIGNAL_TIME_STAMP_WRAPPER_H
#define FSIGNAL_TIME_STAMP_WRAPPER_H

#include "TimeWrapper.h"
#include "DateWrapper.h"
#include <iostream>
#include <ctime>
#include <string>
#include <cassert>
#include <cstdlib>

namespace fsignal
{
    class TimeStampWrapper
    {
    public:
        TimeStampWrapper()
        {
            tw = *(new TimeWrapper());
            dw = *(new DateWrapper());
        }

        TimeStampWrapper(TimeWrapper &tw, DateWrapper &dw)
        {
            this->tw = tw;
            this->dw = dw;
        }

        /**Number of seconds since January 1, 1970 UTC.*/
        TimeStampWrapper(time_t* tsecs)
        {		
            tw = *(new TimeWrapper(tsecs));
            dw = *(new DateWrapper(tsecs));				
        }

        /** @return  time measured in seconds, since January 1, 1970 UTC.*/
        int getTimeInSeconds();
        /** @return  time measured in milliseconds, since January 1, 1970 UTC.*/
        long long getTimeInMillis();
        /** @return  time measured in microseconds, since January 1, 1970 UTC.*/
        long long getTimeInMicros();
        /** @return  time measured in nanoseconds, since January 1, 1970 UTC.*/
        long long getTimeInNanos();
        /** @return  time measured in picoseconds, since January 1, 1970 UTC.*/
        //Overflowing...
        long long getTimeInPicos();

        void addSeconds(int amount);
        void addMilliSeconds(int amount);
        void addMicroSeconds(int amount);
        void addNanoSeconds(int amount);

        void subSeconds(int amount);
        void subMilliSeconds(int amount);
        void subMicroSeconds(int amount);
        void subNanoSeconds(int amount);

        void toString(std::string& str);
        
        TimeWrapper &getTimeWrapper(){
            return tw;
        }
        
        DateWrapper &getDateWrapper(){
            return dw;
        }
        
        inline void operator=(TimeStampWrapper &t){
            tw = t.getTimeWrapper();
            dw = t.getDateWrapper();
        }
        
        inline void operator=(TimeStampWrapper *t){
            tw = t->getTimeWrapper();
            dw = t->getDateWrapper();
        }
        
    private:
        TimeWrapper tw;
        DateWrapper dw;
        enum TimeField{SECOND, MILLISECOND, MICROSECOND, NANOSECOND};
        void add(TimeField field, int amount);
    };

    inline int TimeStampWrapper::getTimeInSeconds()
    {
        tm* now = (tm*)malloc(sizeof(struct tm));
        now->tm_sec = tw.getSeconds();
        now->tm_min = tw.getMinutes();
        now->tm_hour = tw.getHours();
        now->tm_mday = dw.getDay();
        now->tm_mon = dw.getMonth();
        now->tm_year = dw.getYear() - 1900;
    //    char *tz;
//        tz = getenv("TZ");
//        setenv("TZ", "", 1);
//        tzset();
        time_t nowsecs = mktime(now);
//        if (tz)
//            setenv("TZ", tz, 1);
//        else
//            unsetenv("TZ");
//        tzset();
//        free(now);
        return (long long)nowsecs;
    }

    inline long long TimeStampWrapper::getTimeInMillis()
    {		
        return getTimeInSeconds() * 1000 + tw.getMillis();
    }

    inline long long TimeStampWrapper::getTimeInMicros()
    {
        return getTimeInMillis() * 1000 + tw.getMicros();
    }

    inline long long TimeStampWrapper::getTimeInNanos()
    {
        return getTimeInMicros() * 1000 + tw.getNanos();
    }

    inline void TimeStampWrapper::add(TimeField field, int amount)
    {
        if(field == SECOND)
        {
            time_t toset = getTimeInSeconds() + amount;
            tw.setTime(&toset);	
            dw.setDate(&toset);
        }
        else if(field == MILLISECOND)
        {
            int total = tw.getMillis() + amount;
            if(total > 999)
            {
                tw.setMillis(total - 1000);
                add(SECOND, 1);
            }
            else if(total < 0)
            {
                tw.setMillis(total + 1000);
                add(SECOND, -1);
            }
            else
                tw.setMillis(total);
        }
        else if(field == MICROSECOND)
        {
            int total = tw.getMicros() + amount;
            if(total > 999)
            {
                tw.setMicros(total - 1000);
                add(MILLISECOND, 1);
            }
            else if(total < 0)
            {
                tw.setMicros(total + 1000);
                add(MILLISECOND, -1);
            }
            else
                tw.setMicros(total);
        }
        else if(field == NANOSECOND)
        {
            int total = tw.getNanos() + amount;
            if(total > 999)
            {
                tw.setNanos(total - 1000);
                add(MICROSECOND, 1);
            }
            else if(total < 0)
            {
                tw.setNanos(total + 1000);
                add(MICROSECOND, -1);
            }
            else
                tw.setNanos(total);
        }
    }

    inline void TimeStampWrapper::addSeconds(int amount)
    {
        if(amount != 0)
            add(SECOND, amount);
    }

    inline void TimeStampWrapper::addMilliSeconds(int amount)
    {
        if(amount != 0)
        {
            addSeconds(amount / 1000);
            add(MILLISECOND, amount % 1000);
        }
    }

    inline void TimeStampWrapper::addMicroSeconds(int amount)
    {
        if(amount != 0)
        {
            addMilliSeconds(amount / 1000);
            add(MICROSECOND, amount % 1000);
        }
    }

    inline void TimeStampWrapper::addNanoSeconds(int amount)
    {
        if(amount != 0)
        {
            addMicroSeconds(amount / 1000);
            add(NANOSECOND, amount % 1000);
        }
    }

    inline void TimeStampWrapper::subSeconds(int amount)
    {
        if(amount != 0)
            add(SECOND, -amount);
    }

    inline void TimeStampWrapper::subMilliSeconds(int amount)
    {
        if(amount != 0)
        {
            subSeconds(amount / 1000);
            add(MILLISECOND, -(amount % 1000));
        }
    }

    inline void TimeStampWrapper::subMicroSeconds(int amount)
    {
        if(amount != 0)
        {
            subMilliSeconds(amount / 1000);
            add(MICROSECOND, -(amount % 1000));
        }
    }

    inline void TimeStampWrapper::subNanoSeconds(int amount)
    {
        if(amount != 0)
        {
            subMicroSeconds(amount / 1000);
            add(NANOSECOND, -(amount % 1000));
        }
    }

    inline void TimeStampWrapper::toString(std::string& str)
    {
        std::string date;
        std::string time;
        dw.toDateString(date);
        tw.toTimeString(time); 
        str = date + " " + time;
    }
    
    //Overflowing...
    /*inline long long TimeStampWrapper::getTimeInPicos()
    {
            return getTimeInNanos() * 1000 + tw.getPicos();
    }*/
}

#endif
