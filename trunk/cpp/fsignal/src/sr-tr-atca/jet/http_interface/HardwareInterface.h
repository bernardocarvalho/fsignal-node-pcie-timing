#ifndef FSIGNAL_HW_INTERFACE_H
#define FSIGNAL_HW_INTERFACE_H

#include <string>
#include <sstream>
#include "HardwareXml.h"
#include "XmlParser.h"
#include "HardwareXmlHandler.h"

namespace fsignal {
    class HardwareInterface{

        public:
            HardwareInterface(std::string &xmlHardwareLoc){
                std::stringstream logStream;
                XmlParser parser;
                HardwareXmlHandler xmlHandler;
                parser.parse(xmlHardwareLoc.c_str(), xmlHandler);    
                hardwareXML = xmlHandler.getHardwareXml();                
            }
            ~HardwareInterface(){}

            virtual bool updateFieldValue(std::string fieldUID, std::string value) = 0;
            virtual std::string getFieldValue(std::string fieldUID) = 0;
            
            const HardwareXml &getHardwareXML() const{
                return hardwareXML;
            }

        private:
            HardwareXml hardwareXML;
    };
}

#endif
