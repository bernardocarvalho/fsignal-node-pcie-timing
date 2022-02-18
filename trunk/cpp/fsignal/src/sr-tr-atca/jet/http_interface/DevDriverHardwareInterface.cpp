#include "DevDriverHardwareInterface.h"
#include <sys/ioctl.h>

using namespace std;
using namespace fsignal;

bool DevDriverHardwareInterface::updateFieldValue(string fieldUID, string value){
    stringstream logStream;
    logStream.str("");
    logStream << "Trying to update the field " << fieldUID << " with value " << value;
    HTTPInterface::logMessage(logStream);
    fieldValues[fieldUID] = value;

    string ioctlName = ioctlLocs.find(fieldUID)->second;        
    if(ioctlName.size() == 0){
        logStream.str("");
        logStream << "ioctl name not found for this field";
        HTTPInterface::logMessage(logStream);
        return false;
    }

    long val = atol(value.c_str());
    if(ioctl(devFileDesc, atol(ioctlLocs[fieldUID].c_str()), &val) < 0){
        logStream.str("");
        logStream << "ioctl failed";
        HTTPInterface::logMessage(logStream);
        return false;
    }
    return true;
}

bool DevDriverHardwareInterface::createHardwareFields(){
    std::stringstream logStream;
    std::vector<FieldXml> fields = getHardwareXML().getFields();
    for(int i=0; i<fields.size(); i++){
        bool found = false;
        std::vector<ExtraInfoXml> extraInfo = fields[i].getExtraInfo();
        fieldValues.insert(std::pair<std::string, std::string>(fields[i].getUniqueID(), ""));
        for(int j=0; j<extraInfo.size(); j++){
            if(extraInfo[j].getUniqueID() == "ioctl_magic"){
                ioctlLocs.insert(std::pair<std::string, std::string>(fields[i].getUniqueID(), extraInfo[j].getInfo()));
                found = true;
                break;
            }            
        }
        
        if(!found){
            logStream.str("");
            logStream << "Could not find the ioctl magic number extraField for the field " << fields[i].getUniqueID();
            HTTPInterface::logMessage(logStream);
        }
    }            
}
