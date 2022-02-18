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
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <dirent.h>
#include "FSNode.h"
#include "utils/FSignalUtils.h"
#include "xml/XmlParser.h"
#include "xml/NodeXmlHandler.h"
#include "xml/NodeXml.h"
#include "xml/HardwareXml.h"
#include "xml/HardwareXmlHandler.h"
#include "xml/PersistParameter.h"
#include "xml/PersistParameterHandler.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "log4cxx/propertyconfigurator.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace fsignal;
using namespace std;
using namespace org::cfn::scad::core::corba;

void *FSNode::dataSender(void* args) {
    FSNode *node = (FSNode *)args;
    while(true){
        node->queueMux.Lock();
        if(node->sendQueue.empty()){
            node->queueEvtSem.Reset();
            node->queueMux.UnLock();
            node->queueEvtSem.Wait();
            node->queueMux.Lock();
        }        
        DataContainer *dc = node->sendQueue.front();
        node->queueMux.UnLock();
        wchar_t *nodeUID  = Utils::widenCorba(node->getUniqueID());
        wchar_t *hwUID    = Utils::widenCorba(dc->hardwareUniqueID);
        wchar_t *parUID   = Utils::widenCorba(dc->parameterUniqueID);

        db::Data_var data = new db::Data();
        hardware::EventList_var evtList = new hardware::EventList(dc->eventWrapperList.size());
        evtList->length(dc->eventWrapperList.size());
        for(unsigned int i=0; i<dc->eventWrapperList.size(); i++){
            evtList[i] = dc->eventWrapperList[i].getAsCorbaEvent();
        }


        data->tstart      = dc->ts.getAsCorbaTimeStamp();
        data->tend        = dc->tend.getAsCorbaTimeStamp();
        data->configXML   = Utils::widenCorba(dc->configXML);
        data->eList       = evtList;
        data->datab       = Utils::charArrayToByteArray(dc->data, dc->dataSize);
        node->getCentralServer()->newDataAvailable(nodeUID, hwUID, parUID, data);        
        CORBA::wstring_free(nodeUID);
        CORBA::wstring_free(hwUID);
        CORBA::wstring_free(parUID);
        node->queueMux.Lock(); 
        node->sendQueue.pop();
        if(dc != NULL){
            delete dc;
        }
        node->queueMux.UnLock();
        //If the queue was full allow others to run
        node->queueFullEvtSem.Post();
    }
    return NULL;
}

FSNode::FSNode(const char* propsLoc, const char *loggerFileLocation) : POA_org::cfn::scad::core::corba::hardware::Node(), props(propsLoc), connected(true), nodeStatus(hardware::STAND_BY){
    queueMux.Create();
    queueEvtSem.Create();
    queueEvtSem.Reset();
    queueFullEvtSem.Reset();
    
    checkPersistDir();
    
    //Guess. TODO pass with configuration file
    maxQueueSize = 50;
    
    if(loggerFileLocation == NULL){
        // Set up a simple configuration that logs on the console.
        BasicConfigurator::configure();
        LOG4CXX_INFO(Utils::getLogger(), "No Utils::getLogger() configuration file was provided. Outputing to console");
    }else{
        PropertyConfigurator::configure(loggerFileLocation);
    }
    
    LOG4CXX_INFO(Utils::getLogger(), "Node is being constructed");

    pthread_t threads[2];
    pthread_create(&threads[0], NULL, fileChecker, (void*) this);
    pthread_create(&threads[1], NULL, dataSender, (void *)this);
    
    NodeXmlHandler* nodeXmlHandler = new NodeXmlHandler();
    XmlParser* parser = new XmlParser();
    parser->parse(props.getValue(Properties::KEY_NODE_XML_LOC).c_str(), *nodeXmlHandler);
    setUniqueID(nodeXmlHandler->getNodeXML().getUniqueID());
    
    vector<string> xmlLocs = props.getHardwareXMLs();
    HardwareXmlHandler* handler;
    for(unsigned int i=0; i<xmlLocs.size(); i++) {
        handler = new HardwareXmlHandler();
        parser->parse(xmlLocs[i].c_str(), *handler);
        parsedHardwareXmls.push_back(handler->getHardwareXml());
        delete handler;
    }
    
    delete nodeXmlHandler;
    delete parser;
}

bool FSNode::sendNewDataAvailable(const string &hardwareUniqueID, const string &parameterUniqueID, 
                                        TimeStampWrapper &ts, TimeStampWrapper &tend, vector<EventWrapper> &eventWrapperList, 
                                        const std::string &configXML, unsigned char *acqData, int acqDataSize){
    
    DataContainer *dc     = new DataContainer(acqData, acqDataSize);
    dc->hardwareUniqueID  = hardwareUniqueID;
    dc->parameterUniqueID = parameterUniqueID;
    dc->ts                = ts;
    dc->tend              = tend;
    dc->eventWrapperList  = eventWrapperList;
    dc->configXML         = configXML;
    
    while(sendQueue.size() > maxQueueSize){
        queueFullEvtSem.ResetWait();
    }
    queueMux.Lock();
    sendQueue.push(dc);
    queueEvtSem.Post();
    queueMux.UnLock();    
    
    return true;
}
fscns::hardware::FieldData* FSNode::getParameterValue(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const CORBA::WChar* fieldUniqueID) {
    if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
        return new fscns::hardware::FieldData(); 
    }
    wchar_t *fuid = CORBA::wstring_dup(fieldUniqueID);
    fscns::hardware::FieldData *data = hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getValue(Utils::narrowCorba(parameterUniqueID), fuid);
    CORBA::wstring_free(fuid);
    return data;
}
    
fscns::hardware::FieldDataList* FSNode::getParameterValues(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID) {
    if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
        return new fscns::hardware::FieldDataList(); 
    }    
    return hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getValues(Utils::narrowCorba(parameterUniqueID));
}
    

void FSNode::checkPersistDir() {
    DIR* dir = opendir(props.getValue(Properties::KEY_PERSIST_DIR_LOC).c_str());
    if(dir == NULL) {
        int res = mkdir(props.getValue(Properties::KEY_PERSIST_DIR_LOC).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if(res != 0) {
            LOG4CXX_FATAL(Utils::getLogger(), "Could not create persist dir in: " + props.getValue(Properties::KEY_PERSIST_DIR_LOC));
            LOG4CXX_FATAL(Utils::getLogger(), "Exiting...");
            exit(-1);
        }
        
    }
    else
        closedir(dir);
}

void FSNode::loadSavedValues() {
    DIR *dirMain = opendir(props.getValue(Properties::KEY_PERSIST_DIR_LOC).c_str());
    if(dirMain != NULL) {
        chdir(props.getValue(Properties::KEY_PERSIST_DIR_LOC).c_str());
        struct dirent *dpMain = NULL;
        struct dirent *dpXml = NULL;
        
        DIR *dirXml = NULL;
        XmlParser* parser = new XmlParser();
        while((dpMain = readdir(dirMain))) {
            if(dpMain->d_type != DT_DIR || strcmp(dpMain->d_name, ".") == 0 || strcmp(dpMain->d_name, "..") == 0)
                continue;
            
            chdir(dpMain->d_name);
            dirXml = opendir(".");
            if(dirXml != NULL) {
                while((dpXml = readdir(dirXml))) {
                    if(dpXml->d_type == DT_REG && string(dpXml->d_name).find_first_of(".xml") != 0) {
                        PersistParameterHandler* handler = new PersistParameterHandler();
                        parser->parse(dpXml->d_name, *handler, false);
                        map<string, FSHardware*>::iterator iter = hardwareTable.find(dpMain->d_name);
                        if(iter != hardwareTable.end() ) {
                            map<string, FSParameter*> parametersTable = iter->second->getParametersTable();
                            map<string, FSParameter*>::iterator iterp = parametersTable.find(handler->getPersistParameter().getParameterUniqueID());
                            if(iterp != parametersTable.end() ) {
                                iterp->second->setValues(handler->getPersistParameter().getFields());
                                iterp->second->setTransferFunction(handler->getPersistParameter().getTransferFunction());
                                iterp->second->setTransferUnits(handler->getPersistParameter().getTransferUnits());                                
                            }
                        }
                        delete handler;
                    }
                }
                chdir("..");
                closedir(dirXml);
            }
        }
        closedir(dirMain);
        delete parser;
        chdir("..");
    }
    
    //Force reload all (useful for instance for the interface with device drivers)    
    map<string, FSHardware*>::iterator iterh;
    for(iterh = hardwareTable.begin(); iterh != hardwareTable.end(); iterh++){
        map<string, FSParameter*> parametersTable = iterh->second->getParametersTable();
        map<string, FSParameter*>::iterator iterp;
        for(iterp = parametersTable.begin(); iterp != parametersTable.end(); iterp++){
            iterp->second->setValues(iterp->second->getValues());
            iterp->second->setConnected(iterp->second->isConnected());
        }
    }
}

CORBA::WChar* FSNode::getNodeInfo() {
    wstring read = Utils::readFileAsWString(props.getValue(Properties::KEY_NODE_XML_LOC).c_str(), props.getValue(Properties::KEY_XML_LOCALE).c_str());
    
    return CORBA::wstring_dup(read.c_str());
}

util::StringList* FSNode::getHardwareInfos() {
    vector<string> hwXmls = props.getHardwareXMLs();
    util::StringList_var strList = new util::StringList(hwXmls.size());
    strList->length(hwXmls.size());
    
    for(unsigned int i=0; i<hwXmls.size(); i++) {
        wstring hwread;
        Utils::readFileToWString(hwXmls[i].c_str(), hwread, props.getValue(Properties::KEY_XML_LOCALE).c_str());
        strList[i] = CORBA::wstring_dup(hwread.c_str());
    }
    
    return strList._retn();
}

hardware::ExtraLib* FSNode::getExtraLibs(const util::TimeStamp& lastDownload) {
    return Utils::getExtraLib(props.getValue(Properties::KEY_EXTRA_LIB).c_str(), lastDownload);
}

void FSNode::configure(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::FieldDataList& data) {
    if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
        return;
    }
    hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->setValues(Utils::narrowCorba(parameterUniqueID), data);
    
    //Write the new xml
    stringstream fileLoc;
    string       fileLocTemp;
    fileLoc << props.getValue(Properties::KEY_PERSIST_DIR_LOC) << "/" << Utils::narrowCorba(hardwareUniqueID) << "/";
    fileLocTemp = fileLoc.str();
    
    mkdir(fileLoc.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    DIR* dir = opendir(fileLoc.str().c_str());
    if(dir == NULL)
        return;
    else
        closedir(dir);
    
    //Must save all xmls, mostly because of the hardwareFields...
    map<string, FSParameter*> parametersTable = hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getParametersTable();
    map<string, FSParameter*>::iterator iterp;
    string parUID;
    for(iterp = parametersTable.begin(); iterp != parametersTable.end(); iterp++){
        parUID = iterp->first;
        fileLoc.str("");
        fileLoc << fileLocTemp;
        fileLoc << parUID << ".xml";
        ofstream ostream;
        ostream.open(fileLoc.str().c_str());
        if(ostream) {
            ostream << hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getParameter(parUID)->getXMLConfiguration();
            ostream.close();
        }
    }
}

hardware::ConnectionList* FSNode::getAllHardwareConnectionStatus() {
    hardware::ConnectionList_var connList = new hardware::ConnectionList(hardwareTable.size());
    connList->length(hardwareTable.size());
    map<string, FSHardware*>::iterator iter;
    int i=0;
    for(iter = hardwareTable.begin(); iter != hardwareTable.end(); iter++, i++) {
        if(iter->second == NULL){
            continue;
        }
        connList[i].uniqueID = Utils::widenCorba(iter->second->getUniqueID());
        connList[i].status = iter->second->isConnected() ? hardware::CONNECTED : hardware::DISCONNECTED;
    }
    
    return connList._retn();
}

hardware::ConnectionList* FSNode::getAllParametersConnectionStatus(const CORBA::WChar* hardwareUniqueID) {
    if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
        return new hardware::ConnectionList();
    }
    FSHardware* hw = FSNode::hardwareTable[Utils::narrowCorba(hardwareUniqueID)];
    vector<FSParameter*> pars = hw->getAllParameters();
    int npars = hw->getNumberOfParameters();
    hardware::ConnectionList_var connList = new hardware::ConnectionList(npars);
    connList->length(npars);
    for(int i=0; i<npars; i++) {
        connList[i].uniqueID = Utils::widenCorba(pars[i]->getUniqueID());
        connList[i].status = pars[i]->isConnected() ? hardware::CONNECTED : hardware::DISCONNECTED;
    }
    
    return connList._retn();
}

hardware::HardwareStatusList* FSNode::getAllHardwareStatus() {
    hardware::HardwareStatusList_var hwstatusList = new hardware::HardwareStatusList(hardwareTable.size());
    hwstatusList->length(hardwareTable.size());
    map<string, FSHardware*>::iterator iter;
    int i=0;
    for(iter = hardwareTable.begin(); iter != hardwareTable.end(); iter++, i++) {
        hwstatusList[i].uniqueID = Utils::widenCorba(iter->second->getUniqueID());
        hwstatusList[i].stat = iter->second->getStatus();
    }
    
    return hwstatusList._retn();
}

hardware::TransferFunction* FSNode::getTransferFunction(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID) {
    if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
        return new hardware::TransferFunction();
    }
    FSParameter* par = hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getParameter(Utils::narrowCorba(parameterUniqueID));
    hardware::TransferFunction_var tf = new hardware::TransferFunction();
    if(par != NULL){
        tf->function = Utils::widenCorba(par->getTransferFunction());
        tf->units = Utils::widenCorba(par->getTransferUnits());
    }
    return tf._retn();
}

void FSNode::setTransferFunction(const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const hardware::TransferFunction& tf) {
    if(hardwareTable[Utils::narrowCorba(hardwareUniqueID)] == NULL){
        return; 
    }
    FSParameter* par = hardwareTable[Utils::narrowCorba(hardwareUniqueID)]->getParameter(Utils::narrowCorba(parameterUniqueID));
    if(par != NULL) {
        par->setTransferFunction(Utils::narrowCorba(tf.function));
        par->setTransferUnits(Utils::narrowCorba(tf.units));
    }
}

void FSNode::connectToServer() {
    try {
        int nargs = 4;
        vector<string> corbaArgsVec;
        props.getCorbaArgs(corbaArgsVec);        
        char **corbaArgs = new char*[corbaArgsVec.size()];
        for (unsigned int i=0; i<corbaArgsVec.size(); i++) {
            corbaArgs[i] = new char[corbaArgsVec[i].size() + 1];
            strcpy(corbaArgs[i], corbaArgsVec[i].c_str());
        }
        LOG4CXX_DEBUG(Utils::getLogger(), "CORBA args:");
        string tolog = string(corbaArgs[0]) + " " + string(corbaArgs[1]);
        LOG4CXX_DEBUG(Utils::getLogger(),  tolog);        
        tolog = string(corbaArgs[2]) + " " + string(corbaArgs[3]);
        LOG4CXX_DEBUG(Utils::getLogger(), tolog);
                
        orb = CORBA::ORB_init(nargs, corbaArgs);
        CORBA::Object_var csRegisterTemp = getObjectReference(orb);
        activate(orb);
        centralServerRegister = server::CentralServerRegister::_narrow(csRegisterTemp);
        nodeRef = _this();
        CORBA::WChar* empty = CORBA::wstring_dup(L"");
        centralServer = centralServerRegister->registerNode(empty, nodeRef);
        CORBA::wstring_free(empty);
        _remove_ref();
    }
    catch(CORBA::TRANSIENT&) {
        LOG4CXX_FATAL(Utils::getLogger(), "Caught system exception TRANSIENT -- unable to contact the server.");
    }
    catch(CORBA::SystemException& ex) {
        string msg = "Caught omniORB::SystemException:";
        LOG4CXX_FATAL(Utils::getLogger(), msg + ex._name());
    }
    catch(CORBA::Exception& ex) {
        string msg = "Caught omniORB::fatalException:";
        LOG4CXX_FATAL(Utils::getLogger(), msg + ex._name());
    }
    catch(omniORB::fatalException& fe) {
        string msg = "Caught omniORB::fatalException:";
        LOG4CXX_FATAL(Utils::getLogger(), msg);
        int line = fe.line();
        msg = "lile : " + line;
        LOG4CXX_FATAL(Utils::getLogger(), msg + " : " + fe.errmsg());
    }
}

void FSNode::disconnectFromServer() {
    centralServerRegister->unregisterNode(nodeRef);
}

CORBA::Object_ptr FSNode::getObjectReference(CORBA::ORB_ptr orb) {
    CosNaming::NamingContext_var rootContext;
    try {
        // Obtain a reference to the root context of the Name service:
        CORBA::Object_var ns;
        ns = orb->resolve_initial_references("NameService");
        // Narrow the reference returned.
        rootContext = CosNaming::NamingContext::_narrow(ns);
        if( CORBA::is_nil(rootContext) ) {
            LOG4CXX_FATAL(Utils::getLogger(), "Failed to narrow the root naming context.");
            return CORBA::Object::_nil();
        }
    }
    catch (CORBA::NO_RESOURCES&) {
        LOG4CXX_FATAL(Utils::getLogger(), "Caught NO_RESOURCES exception. You must configure omniORB with the  location of the naming service.");
        return 0;
    }
    catch(CORBA::ORB::InvalidName& ex) {
        // This should not happen!
        LOG4CXX_FATAL(Utils::getLogger(), "Service required is invalid [does not exist].");
        return CORBA::Object::_nil();
    }
    // Create a name object, containing the name test/context:
    CosNaming::Name name;
    name.length(1);
    name[0].id = props.getValue(Properties::KEY_CS_LOC).c_str() ; // string copied
    name[0].kind = (const char*) ""; // string copied
    // Note on kind: The kind field is used to indicate the type
    // of the object. This is to avoid conventions such as that used
    // by files (name.type -- e.g. test.ps = postscript etc.)
    try {
        // Resolve the name to an object reference.
        //return rootContext->resolve(name);
        //rootContext->bind(name, orb);
        return rootContext->resolve(name);
    }
    catch(CosNaming::NamingContext::NotFound& ex) {
        // This exception is thrown if any of the components of the
        // path [contexts or the object] aren't found:
        LOG4CXX_FATAL(Utils::getLogger(), "Context not found.");
    }
    catch(CORBA::TRANSIENT& ex) {
        LOG4CXX_FATAL(Utils::getLogger(), "Caught system exception TRANSIENT -- unable to contact the naming service. Make sure the naming server is running and that omniORB is configured correctly.");
    }
    catch(CORBA::SystemException& ex) {
        string msg = "Caught a CORBA:: ";
        LOG4CXX_FATAL(Utils::getLogger(), msg + ex._name() + " while using the naming service.");
        return 0;
    }
    
    return CORBA::Object::_nil();
}

void FSNode::activate(CORBA::ORB_ptr orb) {
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
    PortableServer::ObjectId_var c_id = poa->activate_object(this);
    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();
}

void *FSNode::fileChecker(void* args) {
    FSNode* node = (FSNode*)args;
    remove(node->props.getValue(Properties::KEY_FILE_EXIT_LOC).c_str());
    
    LOG4CXX_INFO(Utils::getLogger(), "Going to be looking for the file in: " << node->props.getValue(Properties::KEY_FILE_EXIT_LOC));
    while(1) {
        fstream fin;
        fin.open(node->props.getValue(Properties::KEY_FILE_EXIT_LOC).c_str(), ios::in);
        if( fin.is_open() ) {
            fin.close();
            LOG4CXX_INFO(Utils::getLogger(), "File found. Exiting");
            node->disconnectFromServer();
            sleep(5);
            node->orb->destroy();
            exit(1);
        }
        fin.close();
        sleep(atoi(node->props.getValue(Properties::KEY_FILE_EXIT_TIME_OUT).c_str()));
    }
}
