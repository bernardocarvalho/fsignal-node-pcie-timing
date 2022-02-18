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
#ifndef FSIGNAL_UTILS_H
#define FSIGNAL_UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cctype> // for toupper
#include <sys/stat.h>
#include <ctime>
#include <unistd.h>

#include <xercesc/util/XMLString.hpp>

#include "../orb/SCADUtils.hh"
#include "../orb/SCAD.hh"
#include "log4cxx/logger.h"

namespace fsignal {    
    class Utils {
    private:
        static log4cxx::LoggerPtr logger;
        
    public:
        static void toUpperCase(std::string& str);
        static void toLowerCase(std::string& str);
        static void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = ",", bool trim = true);
        static std::string trimStr(const std::string& s);
        static std::wstring widen(const std::string& s);
        /*static std::wstring stringToWString(const std::string& s);
        static std::string wstringToString(const std::wstring &s);*/
        static CORBA::WChar* widenCorba(const std::string& s);
        static void findAndReplace(std::string &str, const std::string &searchString, const std::string &replaceString);
        static std::string narrow(const std::wstring& s);
        static std::string narrowCorba(const CORBA::WChar* s);
        static std::string byteArrayToString(const org::cfn::scad::core::corba::util::ByteArray &ba);
        static org::cfn::scad::core::corba::util::ByteArray* stringToByteArrayPtr(const std::string &s);
        static org::cfn::scad::core::corba::util::ByteArray_var stringToByteArray(const std::string &s);
        
        static void readFileToWString(const char* file, std::wstring& dest, const char* locale);
        static std::wstring readFileAsWString(const char* file, const char* locale);
        static org::cfn::scad::core::corba::hardware::ExtraLib* getEmptyExtraLib();
        static org::cfn::scad::core::corba::hardware::ExtraLib* getExtraLib(const char* fileLocation, const org::cfn::scad::core::corba::util::TimeStamp& lastDownload);
                        
        template <class T>
        static org::cfn::scad::core::corba::util::ByteArray_var
        numberVecToByteArray(const std::vector<T> *vec, bool bigEndian = true);
        
        static org::cfn::scad::core::corba::util::ByteArray_var charArrayToByteArray(const unsigned char *vec, int size);
        
        template <class T>
        static char *numberVecToCharArray(const std::vector<T> *vec, bool bigEndian = true);
                        
        template <class T>
        static org::cfn::scad::core::corba::util::ByteArray_var
        numberArrayToByteArray(T *arr, int arrSize, bool bigEndian = true);
        
        template <class T>
        static char *numberArrayToCharArray(T *arr, int arrSize, bool bigEndian = true);
        
        template <class T>
        static void copyVecToVecArray(const std::vector<T> *src, std::vector<char> &dst, bool bigEndian = true);
        
        static const log4cxx::LoggerPtr &getLogger(){
            return logger;
        }
    };
    
    inline org::cfn::scad::core::corba::hardware::ExtraLib* Utils::getEmptyExtraLib() {
        using namespace org::cfn::scad::core::corba;
        hardware::ExtraLib_var lib = new hardware::ExtraLib();
        lib->jarFile = Utils::stringToByteArray("");
        util::Time_var t = new util::Time();
        t->picos=0;
        t->nanos=0;
        t->micros=0;
        t->millis=0;
        t->seconds=0;
        t->minutes=0;
        t->hours=0;
        util::Date_var d = new util::Date();
        d->day=0;
        d->month=0;
        d->year=0;
        util::TimeStamp_var ts = new util::TimeStamp();
        ts->dt=d;
        ts->tm=t;
        lib->lastModified = ts;
        return lib._retn();
    }
    
    inline org::cfn::scad::core::corba::hardware::ExtraLib* Utils::getExtraLib(const char* fileLocation, const org::cfn::scad::core::corba::util::TimeStamp& lastDownload) {
        using namespace org::cfn::scad::core::corba;
        using namespace std;
        
        //check if file exists...
        ifstream istream;
        istream.open(fileLocation, ios::binary);
        if( !istream )
            return getEmptyExtraLib();
        istream.close();
        
        struct stat attrib;// create a file attribute structure
        stat(fileLocation, &attrib);// get the attributes of the file
        tm* clock = gmtime(&(attrib.st_mtime));// Get the last modified time and put it into the time structure
        util::Time_var t = new util::Time();
        t->picos = 0;
        t->nanos = 0;
        t->micros = 0;
        t->millis = 0;
        t->seconds=clock->tm_sec;
        t->minutes=clock->tm_min;
        t->hours=clock->tm_hour;
        util::Date_var d = new util::Date();
        d->day = clock->tm_mday;
        d->month = clock->tm_mon;
        d->year = clock->tm_year + 1900;
        util::TimeStamp_var ts = new util::TimeStamp();
        ts->dt=d;
        ts->tm=t;
        
        //free(clock);
        if(lastDownload.dt.year == d->year && lastDownload.dt.month == d->month && lastDownload.dt.day == d->day &&
                lastDownload.tm.hours == t->hours && lastDownload.tm.minutes == t->minutes && lastDownload.tm.seconds == t-> seconds)
            return getEmptyExtraLib();
        
        hardware::ExtraLib_var lib = new hardware::ExtraLib();
        lib->lastModified = ts;
        
        int R_BUF_SIZE = 2000;
        char buf[R_BUF_SIZE];
        istream.open(fileLocation, ios::binary);
        vector<char> rbytes;
        while(!istream.eof()) {
            istream.read(buf, R_BUF_SIZE);
            for(int i=0; i<istream.gcount(); i++)
                rbytes.push_back(buf[i]);
        }
        
        istream.close();
        
        org::cfn::scad::core::corba::util::ByteArray_var bfile = new org::cfn::scad::core::corba::util::ByteArray(rbytes.size());
        bfile->length(rbytes.size());
        for(unsigned int j = 0; j<rbytes.size(); j++)
            bfile[j] = rbytes[j];
        
        lib->jarFile = bfile;
        
        string logMsg = "returning file after reading: ";
        logMsg += rbytes.size() + " bytes";
        LOG4CXX_INFO(getLogger(), logMsg);
        
        return lib._retn();
    }
    
    inline void Utils::readFileToWString(const char* file, std::wstring& dest, const char* localeKey) {
        std::wstring s;
        std::wostringstream buffer;
        std::wifstream in(file);
        in.imbue(std::locale(localeKey));
        
        while(!getline(in, s).eof())
            buffer << s;
        
        in.close();
        dest = buffer.str();
    }
    
    inline std::wstring Utils::readFileAsWString(const char* file, const char* localeKey) {
        std::wstring s;
        std::wostringstream buffer;
        std::wifstream in(file);
        in.imbue(std::locale(localeKey));
        
        while(!getline(in, s).eof())
            buffer << s;
        
        in.close();
        return buffer.str();
    }
    
    inline int upper(int c) {
        return std::toupper((unsigned char)c);
    }
    
    inline int lower(int c) {
        return std::tolower((unsigned char)c);
    }
    
    inline void Utils::toUpperCase(std::string& str) {
        using namespace std;
        transform(str.begin(), str.end(), str.begin(), upper);
    }
    
    inline void Utils::toLowerCase(std::string& str) {
        using namespace std;
        transform(str.begin(), str.end(), str.begin(), lower);
    }
    
    inline void Utils::tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters, bool trim) {
        using namespace std;
        // Skip delimiters at beginning.
        string::size_type lastPos = str.find_first_not_of(delimiters, 0);
        // Find first "non-delimiter".
        string::size_type pos = str.find_first_of(delimiters, lastPos);
        
        while (string::npos != pos || string::npos != lastPos) {
            // Found a token, add it to the vector.
            if(!trim)
                tokens.push_back(str.substr(lastPos, pos - lastPos));
            else
                tokens.push_back(trimStr(str.substr(lastPos, pos - lastPos)));
            // Skip delimiters.  Note the "not_of"
            lastPos = str.find_first_not_of(delimiters, pos);
            // Find next "non-delimiter"
            pos = str.find_first_of(delimiters, lastPos);
        }
    }
    
    inline std::string Utils::trimStr(const std::string& s) {
        using namespace std;
        if(s.length() == 0)
            return s;
        int b = s.find_first_not_of(" \t\n");
        int e = s.find_last_not_of(" \t\n");
        if(b == -1) // No non-spaces
            return "";
        
        return std::string(s, b, e - b + 1);
    }
    
    inline std::wstring Utils::widen(std::string const &s) {
        return std::wstring(s.begin(), s.end());
    }
    
    inline CORBA::WChar *Utils::widenCorba(const std::string &s) {
        return CORBA::wstring_dup(widen(s).c_str());
    }
    
    inline std::string Utils::narrow(std::wstring const &s) {
        return std::string(s.begin(), s.end());
    }
    
    inline std::string Utils::narrowCorba(const CORBA::WChar *s) {
        wchar_t* str = CORBA::wstring_dup(s);
        std::wstring ret(str);
        CORBA::wstring_free(str);
        return narrow(ret);
    }
    
    inline std::string Utils::byteArrayToString(const org::cfn::scad::core::corba::util::ByteArray &ba) {
        std::stringstream val;
        for(unsigned int j=0; j<ba.length(); j++)
            val << ba[j];
        
        return val.str();
    }
    
    inline org::cfn::scad::core::corba::util::ByteArray_var Utils::stringToByteArray(const std::string &s) {
        org::cfn::scad::core::corba::util::ByteArray_var ret = new org::cfn::scad::core::corba::util::ByteArray(s.size());
        ret->length(s.size());
        for(unsigned int j = 0; j<s.length(); j++)
            ret[j] = s[j];
        
        return ret;
    }
    
    inline org::cfn::scad::core::corba::util::ByteArray* Utils::stringToByteArrayPtr(const std::string &s) {
        return stringToByteArray(s)._retn();
    }
    
    template <class T>
    inline org::cfn::scad::core::corba::util::ByteArray_var Utils::numberVecToByteArray(const std::vector<T> *vec, bool bigEndian) {
        int typeSize = sizeof(T);
        std::vector<char> *byteVec = (std::vector<char> *)vec;
        org::cfn::scad::core::corba::util::ByteArray_var oct = new org::cfn::scad::core::corba::util::ByteArray(byteVec->size());
        oct->length(byteVec->size());        
        
        if(!bigEndian) {
            for(unsigned int i=0; i<byteVec->size(); i++)
                oct[i] = byteVec->at(i);
        }
        else {
            for(unsigned int i=0; i<byteVec->size() / typeSize; i++)
                for(int j=typeSize - 1; j>=0; j--){                    
                    oct[typeSize * i + j] = byteVec->at(typeSize * i + typeSize -1 - j);
                }
        }
                
        return oct;
    }
    
    inline org::cfn::scad::core::corba::util::ByteArray_var Utils::charArrayToByteArray(const unsigned char *vec, int size) {
        org::cfn::scad::core::corba::util::ByteArray_var oct = new org::cfn::scad::core::corba::util::ByteArray(size);
        oct->length(size);
        
        memcpy(oct->NP_data(), vec, size);
                
        return oct;
    }
    
    template <class T>
    inline org::cfn::scad::core::corba::util::ByteArray_var Utils::numberArrayToByteArray(T *arr, int arrSize, bool bigEndian){
        int  typeSize  = sizeof(T);
        int  size      = typeSize * arrSize;
        char *byteVec  = (char *)arr;
        org::cfn::scad::core::corba::util::ByteArray_var oct = new org::cfn::scad::core::corba::util::ByteArray(size);
        oct->length(size);
        
        if(!bigEndian) {
            for(int i=0; i<size; i++)
                oct[i] = byteVec[i];
        }
        else {
            for(int i=0; i<arrSize; i++)
                for(int j=typeSize - 1; j>=0; j--)
                    oct[typeSize * i + j] = byteVec[typeSize * i + typeSize -1 - j];
        }
        
        return oct;
    }
    
    template <class T>
    inline char *Utils::numberVecToCharArray(const std::vector<T> *vec, bool bigEndian) {
        int typeSize = sizeof(T);
        std::vector<char> *byteVec = (std::vector<char> *)vec;
        char *ret = (char *)malloc(byteVec->size());
        
        if(!bigEndian) {
            for(unsigned int i=0; i<byteVec->size(); i++)
                ret[i] = byteVec->at(i);
        }
        else {
            for(unsigned int i=0; i<byteVec->size() / typeSize; i++)
                for(unsigned int j=typeSize - 1; j>=0; j--)
                    ret[typeSize * i + j] = byteVec->at(typeSize * i + typeSize -1 - j);
        }
        
        return ret;
    }
    
    template <class T>
    inline char *Utils::numberArrayToCharArray(T *arr, int arrSize, bool bigEndian) {
        int  typeSize = sizeof(T);
        int  size      = arrSize * typeSize;
        char *byteVec  = (char *)arr;
        char *ret      = (char *)malloc(size);
        
        if(!bigEndian) {
            for(int i=0; i<size; i++)
                ret[i] = byteVec[i];
        }
        else {
            for(int i=0; i<arrSize; i++)
                for(int j=typeSize - 1; j>=0; j--)
                    ret[typeSize * i + j] = byteVec[typeSize * i + typeSize -1 - j];
        }
        
        return ret;
    }        
    
    template <class T>
    inline void Utils::copyVecToVecArray(const std::vector<T> *src, std::vector<char> &dst, bool bigEndian) {
        int typeSize = sizeof(T);
        std::vector<char> *tempVec = (std::vector<char> *)src;
        dst.resize(tempVec->size());
        if(!bigEndian){
            for(int i=0; i<tempVec->size(); i++){                
                dst[i] = tempVec->at(i);                
            }
        }
        else{                        
            for(int i=0; i<tempVec->size() / typeSize; i++)
                for(int j=typeSize - 1; j>=0; j--)
                    dst[typeSize * i + j] = tempVec->at(typeSize * i + typeSize -1 - j);
        }                
    }
    
/*    inline std::wstring Utils::stringToWString(const std::string &s) {
        int size = s.length();
        wchar_t *wc = (wchar_t *)malloc(size * sizeof(wchar_t));
        mbstowcs(wc, s.c_str(), size);
        std::wstring wsr(wc);
        delete []wc;
        return wsr;
    }
        
    inline void Utils::wstringToString(const std::wstring &ws, std::string &newStr) {
        newStr = std::string(ws.begin(), ws.end());        
    }*/
    
    inline void Utils::findAndReplace(std::string &str, const std::string &searchString, const std::string &replaceString){
        std::string::size_type pos = 0;
        while ( (pos = str.find(searchString, pos)) != std::string::npos ) {
            str.replace( pos, searchString.size(), replaceString );
            pos++;
        }
    }
}
#endif
