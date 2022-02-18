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
#ifndef FSIGNAL_PROPERTIES_H
#define FSIGNAL_PROPERTIES_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

namespace fsignal {
    class Properties {
    public:
        Properties(const char* propsFileLoc);
        std::string getValue(std::string key);
        std::wstring getValueAsWString(std::string key);
        std::vector<std::string> getHardwareXMLs();
        void getCorbaArgs(std::vector<std::string> &corbaArgs);
        static const std::string KEY_ORB_INIT_REF;
        static const std::string KEY_CS_LOC;
        static const std::string KEY_GIOP_MAX_SIZE;
        static const std::string KEY_FILE_EXIT_LOC;
        static const std::string KEY_FILE_EXIT_TIME_OUT;
        static const std::string KEY_PERSIST_DIR_LOC;
        static const std::string KEY_NODE_XML_LOC;
        static const std::string KEY_HW_XMLS_LOC;
        static const std::string KEY_XML_LOCALE;
        static const std::string KEY_EXTRA_LIB;
        /**If the node is running in local mode, where should the files be saved?*/
        static const std::string KEY_LOCAL_DIR_SAVE;
    private:
        std::map<std::string, std::string> rvalues;
    };
    
    inline std::string Properties::getValue(std::string key) {
        std::map<std::string, std::string>::iterator iter = rvalues.find(key);
        if(iter != rvalues.end())
            return iter->second;
        else
            return "";
    }

    inline std::wstring Properties::getValueAsWString(std::string key) {
        std::map<std::string, std::string>::iterator iter = rvalues.find(key);
        if(iter != rvalues.end()){
            std::wstring ret(iter->second.begin(), iter->second.end());
            return ret;
        }
        else
            return L"";
    }
    
    inline void Properties::getCorbaArgs(std::vector<std::string> &corbaArgs) {
        corbaArgs.push_back(std::string("-" + KEY_ORB_INIT_REF));
        corbaArgs.push_back(getValue(KEY_ORB_INIT_REF));
        corbaArgs.push_back(std::string("-" + KEY_GIOP_MAX_SIZE));
        corbaArgs.push_back(getValue(KEY_GIOP_MAX_SIZE));
    }
}

#endif
