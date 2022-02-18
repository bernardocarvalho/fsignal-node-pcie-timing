#include "HTTPInterface.h"

#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "FSignalUtils.h"
#include "HardwareXml.h"
#include "FieldXml.h"
#include "shttpd.h"

#include "HardwareInterface.h"

#define ALIAS_URI "/my_etc"
#define ALIAS_DIR "/etc/"

using namespace std;
using namespace fsignal;
using namespace log4cxx;
using namespace log4cxx::helpers;

static HardwareXml hwXML;
static stringstream logStream;

static int printParameterValues(struct shttpd_arg *arg){
    int                       len       = 0;
    int                       value     = 0;    
    const vector<FieldXml>    fields    = hwXML.getFields();
    char                      varBuffer   [128];
    string                    queryStr  = shttpd_get_env(arg, "QUERY_STRING");
    
    if(queryStr.c_str() != NULL){
        for(int i=0; i<fields.size(); i++){
            memset(varBuffer, 0, sizeof(varBuffer));        
            //shttpd_get_var(fieldsUIDS[i].c_str(), queryStr.c_str(), strlen(queryStr), varBuffer, sizeof(varBuffer));
            if(queryStr.find(fields[i].getUniqueID().c_str()) > -1){
                //value = atol(varBuffer);
                len += shttpd_printf(arg, "%s=%d\r\n", fields[i].getUniqueID().c_str(), value);
            }           
        }
    }
    return len;
}

static void sendHeader(struct shttpd_arg *arg, const char *type){    
    shttpd_printf(arg, "Content-Type: %s\r\n", type);
}

static void handleParamRequests(struct shttpd_arg *arg){
    int           length = 0;    
    const char   *request_method = shttpd_get_env(arg, "REQUEST_METHOD" );    

    sendHeader(arg, PLAIN_TEXT);
    
    //If it is post we will need to update the parameters, otherwise it only wants
    //to know the values
    logStream.str("");
    logStream << "The request method is " << request_method;
    HTTPInterface::logMessage(logStream);
    
    if(strcmp(request_method, "GET") == 0){
        
    }else{
        
    }
    
    length = printParameterValues(arg);
    
    shttpd_printf(arg, "Content-Length: %d\r\n", length);
    arg->flags |= SHTTPD_END_OF_OUTPUT;
}

void HTTPInterface::initLogger(const char *loggerFileLocation){
    if(loggerFileLocation == NULL){
        // Set up a simple configuration that logs on the console.
        BasicConfigurator::configure();
        logStream.str("");
        logStream << "No Utils::getLogger() configuration file was provided. Outputing to console";
        logMessage(logStream);
    }else{
        PropertyConfigurator::configure(loggerFileLocation);
    }        
}

void HTTPInterface::setHardware(HardwareInterface *hwInterface){
    hwXML = hwInterface[0].getHardwareXML();
    
    logStream.str("");
    logStream << "XML loaded and hardware UID is " << hwXML.getUniqueID();
    logMessage(logStream);    
}

void HTTPInterface::initHTTPInterface(int port){
    struct shttpd_ctx *ctx;    
    
    ctx = shttpd_init(NULL, "aliases", ALIAS_URI "=" ALIAS_DIR, "document_root", ".", NULL);
    shttpd_register_uri(ctx, "/params", &handleParamRequests, NULL);
    shttpd_listen(ctx, port, 0);
    logStream.str("");
    logStream << "Starting http server";
    logMessage(logStream);
    /* Serve connections infinitely until someone kills us TODO SOLVE*/
    for (;;)
        shttpd_poll(ctx, 1000);
    
    shttpd_fini(ctx);
}

