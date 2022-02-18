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
#ifndef FSIGNAL_NODE_H
#define FSIGNAL_NODE_H

#include <string>
#include <map>
#include <queue>
#include "orb/SCAD.hh"
#include "orb/SCADUtils.hh"
#include "utils/FSignalUtils.h"
#include "utils/TimeStampWrapper.h"
#include "utils/EventWrapper.h"
#include "utils/MutexSem.h"
#include "utils/EventSem.h"
#include "xml/HardwareXml.h"
#include "Properties.h"
#include "FSHardware.h"
#include "log4cxx/logger.h"

#define fscns org::cfn::scad::core::corba

namespace fsignal {
    
    class DataContainer{                
    public:
        DataContainer(unsigned char *newData, int size){
            data = (unsigned char *)malloc(size);
            if(data != NULL){
                memcpy(data, newData, size);
                dataSize = size;
            }
            else{
                dataSize = -1;
            }
        }
        
        ~DataContainer(){
            if(data != NULL){
                free(data);
            }
        }
        
        std::string               hardwareUniqueID;
        std::string               parameterUniqueID;
        TimeStampWrapper          ts;
        TimeStampWrapper          tend;
        std::vector<EventWrapper> eventWrapperList;
        std::string               configXML;
        unsigned char            *data;
        int                       dataSize;        
    };
    
    class FSNode : public POA_org::cfn::scad::core::corba::hardware::Node {
    public:
        FSNode(const char* propsLoc, const char *loggerFileLocation = NULL);
        ~FSNode(){}
        static void* dataSender(void* args);
        CORBA::WChar* getNodeInfo();
        fscns::util::StringList* getHardwareInfos();
        void ping(){}
        fscns::hardware::ExtraLib* getExtraLibs(const fscns::util::TimeStamp& lastDownload);
        fscns::hardware::ConnectionStatus getConnectionStatus();
        void changeConnectionStatus(fscns::hardware::ConnectionStatus newStatus);
        fscns::hardware::ConnectionStatus getHardwareConnectionStatus(const CORBA::WChar* hardwareUniqueID);
        fscns::hardware::ConnectionList* getAllHardwareConnectionStatus();
        void changeHardwareConnectionStatus(const CORBA::WChar* hardwareUniqueID, fscns::hardware::ConnectionStatus newStatus);
        fscns::hardware::ConnectionList* getAllParametersConnectionStatus(const CORBA::WChar* hardwareUniqueID);
        fscns::hardware::ConnectionStatus getParameterConnectionStatus(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID);
        void changeParameterConnectionStatus(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, fscns::hardware::ConnectionStatus newStatus);
        fscns::hardware::NodeStatus getNodeStatus();
        fscns::hardware::HardwareStatus getHardwareStatus(const CORBA::WChar* hardwareUniqueID);
        fscns::hardware::HardwareStatusList* getAllHardwareStatus();
        void changeHardwareStatus(const CORBA::WChar* hardwareUniqueID, fscns::hardware::HardwareStatus newStatus);
        void changeAllHardwareStatus(fscns::hardware::HardwareStatus newStatus);
        fscns::hardware::TransferFunction* getTransferFunction(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID);
        void setTransferFunction(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::TransferFunction& tf);
        fscns::hardware::FieldData* getParameterValue(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const CORBA::WChar* fieldUniqueID);
        fscns::hardware::FieldDataList* getParameterValues(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID);
        void configure(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::FieldDataList& data);
        std::vector<HardwareXml>& getParsedHardwareXmls();
        virtual void shutdown(const CORBA::WChar* reason);
        virtual void setEventTable(const fscns::hardware::EventList& evtList);
        virtual void newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend);
        virtual void abort();
        
        bool isConnected();
        std::string getUniqueID();
        
        void setConnected(bool connected);
        virtual void setNodeStatus(fscns::hardware::NodeStatus nodeStatus);
        void setUniqueID(std::string uniqueID);
        
        /** This method can be used by specialized classes to send data.
         * It knows how to distinguish between local and remote modes.
         * In local mode it will save data locally and send the filename, otherwise
         * it will convert the array to a CORBA array and send.
         * @param hardwareUniqueID The hardware unique identifier
         * @param parameterUniqueID The parameter unique identifier
         * @param ts The time of the first sample
         * @param tend The time of the last sample
         * @param evtList A list with all the events for which it is replying
         * @param configXML The configuration which regards this cluster of data
         * @param acqData The data itself
         * @param acqDataSize The number of bytes in acqData
         * @return True if data is successfully send.*/
        bool sendNewDataAvailable(const std::string &hardwareUniqueID, const std::string &parameterUniqueID, 
                                    TimeStampWrapper &ts, TimeStampWrapper &tend, std::vector<EventWrapper> &evtList, 
                                    const std::string &configXML, unsigned char *acqData, int acqDataSize);
        
    protected:
        Properties props;
        std::queue<DataContainer *> sendQueue;
    private:
        std::map<std::string, FSHardware*> hardwareTable;        
        EventSem queueEvtSem;
        MutexSem queueMux;
        /**
         * Threads wait on this semaphore when the sending queue is full
         */        
        EventSem queueFullEvtSem;
        /** The maximum size of the sending queue*/
        unsigned int maxQueueSize;
        
        
        bool connected;
        fscns::hardware::NodeStatus nodeStatus;
        std::string uniqueID;
        std::vector<HardwareXml> parsedHardwareXmls;
        void checkPersistDir();
        
        /**
         * A thread which looks for the exit file. When this file is found the
         * node is shutdown.
         */
        static void *fileChecker(void* args);
        
        /**ORB**/
        fscns::server::CentralServer_ptr centralServer;
        fscns::server::CentralServerRegister_var centralServerRegister;
        CORBA::ORB_ptr orb;
        fscns::hardware::Node_ptr nodeRef;        
        CORBA::Object_ptr getObjectReference(CORBA::ORB_ptr orb);
        void activate(CORBA::ORB_ptr orb);
        /**~ORB**/        
    protected:
        void connectToServer();
        void disconnectFromServer();
        void addHardware(FSHardware* hw);
        void loadSavedValues();
        const std::map<std::string, FSHardware*>& getHardwareTable();
        const fscns::server::CentralServer_ptr& getCentralServer();
    };
    
    inline const fscns::server::CentralServer_ptr& FSNode::getCentralServer() {
        return centralServer;
    }
    
    inline const std::map<std::string, FSHardware*>& FSNode::getHardwareTable() {
        return hardwareTable;
    }
    
    inline bool FSNode::isConnected() {
        return connected;
    }
    
    inline std::string FSNode::getUniqueID() {
        return uniqueID;
    }
    
    inline void FSNode::setConnected(bool connected) {
        this->connected = connected;
    }
    
    inline void FSNode::setUniqueID(std::string uniqueID) {
        this->uniqueID = uniqueID;
    }
    
    inline fscns::hardware::ConnectionStatus FSNode::getConnectionStatus() {
        return connected ? fscns::hardware::CONNECTED : fscns::hardware::DISCONNECTED;
    }
    
    inline void FSNode::changeConnectionStatus(fscns::hardware::ConnectionStatus newStatus) {
        setConnected(newStatus == fscns::hardware::CONNECTED);
        wchar_t* nodeUID = Utils::widenCorba(uniqueID);
        wchar_t* hwUID = CORBA::wstring_dup(L"");
        wchar_t* parUID = CORBA::wstring_dup(L"");
        centralServer->connectionStatusChanged(nodeUID, hwUID, parUID);
        CORBA::wstring_free(nodeUID);
        CORBA::wstring_free(hwUID);
        CORBA::wstring_free(parUID);
    }
    
    inline fscns::hardware::ConnectionStatus FSNode::getHardwareConnectionStatus(const CORBA::WChar* hardwareUniqueID) {
        FSHardware* hw = hardwareTable[Utils::narrowCorba(hardwareUniqueID)];
        if(hw == NULL){
            return fscns::hardware::DISCONNECTED;
        }
        return hw->isConnected() ? fscns::hardware::CONNECTED : fscns::hardware::DISCONNECTED;
    }
    
    inline void FSNode::changeHardwareConnectionStatus(const CORBA::WChar* hardwareUniqueID, fscns::hardware::ConnectionStatus newStatus) {
        FSHardware* hw = hardwareTable[Utils::narrowCorba(hardwareUniqueID)];
        if(hw == NULL){
            return;
        }
        hw->setConnected(newStatus == fscns::hardware::CONNECTED);
        wchar_t* nodeUID = Utils::widenCorba(uniqueID);
        wchar_t* hwUID = Utils::widenCorba(hw->getUniqueID());
        wchar_t* parUID = CORBA::wstring_dup(L"");
        centralServer->connectionStatusChanged(nodeUID, hwUID, parUID);
        CORBA::wstring_free(nodeUID);
        CORBA::wstring_free(hwUID);
        CORBA::wstring_free(parUID);
    }
    
    inline fscns::hardware::ConnectionStatus FSNode::getParameterConnectionStatus(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID) {
        if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
            return fscns::hardware::DISCONNECTED;
        }
        FSParameter* par = hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getParameter(Utils::narrowCorba(parameterUniqueID));
        if(par == NULL){
            return fscns::hardware::DISCONNECTED;
        }
        return par->isConnected() ? fscns::hardware::CONNECTED : fscns::hardware::DISCONNECTED;
    }
    
    inline void FSNode::changeParameterConnectionStatus(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, fscns::hardware::ConnectionStatus newStatus) {
        if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
            return;
        }
        if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getParameter(Utils::narrowCorba(parameterUniqueID)) == NULL){
            return;
        }
        hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getParameter(Utils::narrowCorba(parameterUniqueID))->setConnected(newStatus == fscns::hardware::CONNECTED);
        wchar_t* nodeUID = Utils::widenCorba(uniqueID);
        wchar_t* hwUID = CORBA::wstring_dup(hardwareUniqueID);
        wchar_t* parUID = CORBA::wstring_dup(parameterUniqueID);
        centralServer->connectionStatusChanged(nodeUID, hwUID, parUID);
        CORBA::wstring_free(nodeUID);
        CORBA::wstring_free(hwUID);
        CORBA::wstring_free(parUID);
    }
    
    inline fscns::hardware::NodeStatus FSNode::getNodeStatus() {
        return nodeStatus;
    }
    
    inline void FSNode::setNodeStatus(fscns::hardware::NodeStatus nodeStatus) {
        this->nodeStatus = nodeStatus;
        wchar_t* nodeUID = Utils::widenCorba(uniqueID);
        centralServer->nodeStatusChange(nodeUID, nodeStatus);
        CORBA::wstring_free(nodeUID);
    }
    
    inline fscns::hardware::HardwareStatus FSNode::getHardwareStatus(const CORBA::WChar* hardwareUniqueID) {
        if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
            return org::cfn::scad::core::corba::hardware::STOPPED;
        }
        return hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getStatus();
    }
    
    inline void FSNode::changeHardwareStatus(const CORBA::WChar* hardwareUniqueID, fscns::hardware::HardwareStatus newStatus) {
        wchar_t* nodeUID = Utils::widenCorba(uniqueID);
        wchar_t* hwUID = CORBA::wstring_dup(hardwareUniqueID);
        if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
            return;
        }        
        hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->changeStatus(newStatus);
        centralServer->hardwareStatusChange(nodeUID, hwUID, hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getStatus());
        CORBA::wstring_free(nodeUID);
        CORBA::wstring_free(hwUID);
    }
    
    inline void FSNode::changeAllHardwareStatus(fscns::hardware::HardwareStatus newStatus) {
        wchar_t* hwUID = NULL;
        std::map<std::string, FSHardware*>::iterator iter;
        for(iter = hardwareTable.begin(); iter != hardwareTable.end(); iter++) {
            hwUID = Utils::widenCorba(iter->first);
            changeHardwareStatus(hwUID, newStatus);
            CORBA::wstring_free(hwUID);
        }
    }
    
    inline void FSNode::shutdown(const CORBA::WChar* reason) {
        std::cout << "Going to shutdown because: " << Utils::narrowCorba(reason) << std::endl;
        disconnectFromServer();
    }
    
    inline void FSNode::addHardware(FSHardware* hw) {
        hardwareTable[hw->getUniqueID()] = hw;
    }
    
    inline std::vector<HardwareXml>& FSNode::getParsedHardwareXmls() {
        return parsedHardwareXmls;
    }
    
    inline void FSNode::setEventTable(const fscns::hardware::EventList& evtList) {
        std::cout << "At FSNode. setEventTable is not implemented... you should override this..." << std::endl;
    }
    
    inline void FSNode::newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend) {
        std::cout << "At FSNode. newDataAvailable is not implemented... you should override this..." << std::endl;
    }
    
    inline void FSNode::abort() {
        std::cout << "At FSNode. abort is not implemented... you should override this..." << std::endl;
    }
}
#endif
