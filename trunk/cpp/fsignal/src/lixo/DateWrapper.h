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
#ifndef FSIGNAL_DATE_WRAPPER_H
#define FSIGNAL_DATE_WRAPPER_H

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>

namespace fsignal
{
    class DateWrapper
    {
    public:
        DateWrapper() :
                day(1), month(1), year(1970){}
        DateWrapper(const unsigned char day,
                const unsigned char month,
                const short year) :
                day(day), month(month), year(year){}		

        /**Number of seconds since January 1, 1970 UTC.*/
        DateWrapper(time_t* tsecs)
        {		
                setDate(tsecs);
        }

        unsigned char getDay();
        unsigned char getMonth();
        short getYear();
        
        void setDay(const unsigned char day);
        void setMonth(const unsigned char month);
        void setYear(const short year);		
        void setDate(time_t* tsecs);
        void toDateString(std::string& str);
        
        inline void operator=(DateWrapper &d){
            day   = d.getDay();
            month = d.getMonth();
            year  = d.getYear();
        }
                
        
    private:
        unsigned char day;
        unsigned char month;
        short year;
    };

    inline unsigned char DateWrapper::getDay()
    {
        return day;
    }

    inline unsigned char DateWrapper::getMonth()
    {
        return month;
    }

    inline short DateWrapper::getYear()
    {
        return year;
    }

    inline void DateWrapper::setDay(const unsigned char day)
    {
        this->day = day;
    }

    inline void DateWrapper::setMonth(const unsigned char month)
    {
        this->month = month;
    }

    inline void DateWrapper::setYear(const short year)
    {
        this->year = year;
    }		

    inline void DateWrapper::setDate(time_t *tsecs)
    {
        tm *nowtm = localtime(tsecs);
        day = nowtm->tm_mday;
        month = nowtm->tm_mon;
        year = nowtm->tm_year + 1900;
    }

    inline void DateWrapper::toDateString(std::string& str)
    {
        std::ostringstream buffer;
        buffer << (short)day << "-" << (short)(month + 1) << "-" << (short)year;		
        str = buffer.str();
    }        
}
#endif
