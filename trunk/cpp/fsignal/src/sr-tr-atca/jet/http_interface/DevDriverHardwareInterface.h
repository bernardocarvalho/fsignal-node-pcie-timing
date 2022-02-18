#ifndef FSIGNAL_DEV_DRIVER_HW_INTERFACE_H
#define FSIGNAL_DEV_DRIVER_HW_INTERFACE_H

#include "HardwareInterface.h"
#include "HTTPInterface.h"
#include <string>
#include <map>
#include <errno.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace fsignal {
    class DevDriverHardwareInterface : public HardwareInterface{

    public:
        DevDriverHardwareInterface(std::string &hardwareXML, std::string &devFileLocation) : HardwareInterface(hardwareXML){
            std::stringstream logStream;

            createHardwareFields();
            devFileDesc = open(devFileLocation.c_str(), O_RDWR);
            if(devFileDesc < 1){                        
                logStream.str("");
                logStream << "Failed to open the file at " << devFileLocation;
                HTTPInterface::logMessage(logStream);
                logStream.str("");
                logStream << "Reason: " << strerror(errno);
                HTTPInterface::logMessage(logStream);
            }
            else{
                logStream.str("");
                logStream << "Opened file successfully with file descriptor: " << devFileDesc;
                HTTPInterface::logMessage(logStream);
            }
        }

        ~DevDriverHardwareInterface(){
            std::stringstream logStream;
            if(devFileDesc > 1){
                int res = close(devFileDesc);
                if(res < 0){
                    logStream.str("");
                    logStream << "Failed to close file: " << devFileDesc;
                    HTTPInterface::logMessage(logStream);
                    logStream.str("");
                    logStream << "Reason: " << strerror(errno);
                    HTTPInterface::logMessage(logStream);
                }else{
                    logStream.str("");
                    logStream << "File closed successfully";
                    HTTPInterface::logMessage(logStream);
                }
            }
        }

        bool deviceOK(){
            return (devFileDesc > 0);
        }

        bool updateFieldValue(std::string fieldUID, std::string value);
        
        std::string getFieldValue(std::string fieldUID){
            return fieldValues[fieldUID];
        }

    private:
        int                                devFileDesc;
        std::map<std::string, std::string> ioctlLocs;
        std::map<std::string, std::string> fieldValues;

        bool createHardwareFields();
    };
}

#endif
