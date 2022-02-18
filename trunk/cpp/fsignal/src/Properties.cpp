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
#include "Properties.h"

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include "utils/FSignalUtils.h"

using namespace std;
using namespace fsignal;

const std::string Properties::KEY_ORB_INIT_REF        = "ORBInitRef";
const std::string Properties::KEY_GIOP_MAX_SIZE       = "ORBgiopMaxMsgSize";
const std::string Properties::KEY_CS_LOC              = "CServerID";
const std::string Properties::KEY_FILE_EXIT_LOC       = "FileExitName";
const std::string Properties::KEY_FILE_EXIT_TIME_OUT  = "FileExitSleepCheck";
const std::string Properties::KEY_PERSIST_DIR_LOC     = "PersistDir";
const std::string Properties::KEY_NODE_XML_LOC        = "NodeXMLFileLoc";
const std::string Properties::KEY_HW_XMLS_LOC         = "HardwareXMLFilesLoc";
const std::string Properties::KEY_XML_LOCALE          = "XML_Locale";
const std::string Properties::KEY_EXTRA_LIB           = "ExtraLibLoc";
const std::string Properties::KEY_LOCAL_DIR_SAVE      = "LocalDirSave";

Properties::Properties(const char* propsFileLoc) {
    string s;
    vector<string> parsed;
    string t;
    ifstream in(propsFileLoc);
    
    if(!in.is_open()) {
        cout << "Properties file not found... exiting" << endl;
        exit(1);
    }
    
    while(!getline(in, s).eof()) {
        s = Utils::trimStr(s);
        
        if(s.length() == 0 || s.find('#') == 0)
            continue;
        
        parsed.clear();
        Utils::tokenize(s, parsed, "=", true);
        
        t="";
        if(parsed.size() > 1) {
            for(unsigned int i=1; i<parsed.size(); i++) {
                if(i != 1)
                    t += "=" + parsed[i];
                else
                    t += parsed[i];
            }
            rvalues[parsed[0]] = t;
        }
    }
    
    in.close();
}

vector<string> Properties::getHardwareXMLs() {
    vector<string> hwXmls;
    Utils::tokenize(getValue(Properties::KEY_HW_XMLS_LOC), hwXmls);
    return hwXmls;
}
