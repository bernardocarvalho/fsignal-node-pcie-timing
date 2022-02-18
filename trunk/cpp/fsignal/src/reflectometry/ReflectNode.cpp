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

#if !defined(_POSIX_SOURCE)
#	define _POSIX_SOURCE
#endif
#if !defined(_BSD_SOURCE)
#	define _BSD_SOURCE
#endif

#include "EventSem.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cmath>
#include <queue>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#ifdef HAVE_ZLIB
#include "zlib.h"
#endif

#include "libtelnet.h"

#include "ReflectNode.hpp"
#include "../FSHardware.h"
#include "../FSParameter.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/EventWrapper.h"
#include "../utils/FSignalUtils.h"
#include "ReflectParameter.hpp"

using namespace std;
using namespace fsignal;
using namespace org::cfn::scad::core::corba;

string ReflectNode::TELNET_HOST = "TelnetServerHost";
string ReflectNode::TELNET_PORT = "TelnetServerPort";
int ReflectNode::MODE_FIXED = 0;
int ReflectNode::MODE_SWEEP = 1;

struct DataProducerInit{
    ReflectNode   *node;
    FSHardware    *hardware;
};

/*
 * Definitions for telnet communication
 */
static struct termios orig_tios;
static telnet_t *telnet;
static int do_echo;
queue<string> commandList;

static const telnet_telopt_t telopts[] = {
	{ TELNET_TELOPT_ECHO,		TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DONT },
	{ TELNET_TELOPT_COMPRESS2,	TELNET_WONT, TELNET_DO   },
	{ TELNET_TELOPT_MSSP,		TELNET_WONT, TELNET_DO   },
	{ -1, 0, 0 }
};

static void _cleanup(void) {
	tcsetattr(STDOUT_FILENO, TCSADRAIN, &orig_tios);
}

static void _input(char *buffer, int size) {
	static char crlf[] = { '\r', '\n' };
	int i;

	for (i = 0; i != size; ++i) {
		/* if we got a CR or LF, replace with CRLF
		 * NOTE that usually you'd get a CR in UNIX, but in raw
		 * mode we get LF instead (not sure why)
		 */
		if (buffer[i] == '\r' || buffer[i] == '\n') {
			if (do_echo)
				printf("\r\n");
			telnet_send(telnet, crlf, 2);
		} else {
			if (do_echo)
				putchar(buffer[i]);
			telnet_send(telnet, buffer + i, 1);
		}
	}
	fflush(stdout);
}

static void _send(int sock, const char *buffer, size_t size) {
	int rs;
    stringstream logStream;

	/* send data */
	while (size > 0) {
		if ((rs = send(sock, buffer, size, 0)) == -1) {
			logStream.str("");
			logStream << "Telnet: send() failed: " << strerror(errno);
			LOG4CXX_ERROR(Utils::getLogger(), logStream.str());
			exit(1);
		} else if (rs == 0) {
			logStream.str("");
			logStream << "Telnet: send() unexpectedly returned 0";
			LOG4CXX_ERROR(Utils::getLogger(), logStream.str());
			exit(1);
		}

		/* update pointer and size to see if we've got more to send */
		buffer += rs;
		size -= rs;
	}
}


static void _event_handler(telnet_t *telnet, telnet_event_t *ev,
		void *user_data) {
	int sock = *(int*)user_data;
	stringstream logStream;
	
	switch (ev->type) {
	/* data received */
	case TELNET_EV_DATA:
		//printf("%.*s", (int)ev->data.size, ev->data.buffer);
		//fflush(stdout);
		if(strstr(ev->data.buffer, "> ")!=NULL && !commandList.empty()) {
			char comm[32];
			strcpy(comm, commandList.front().c_str());
			_input(comm, strlen(comm));
			commandList.pop();
		}
		break;
	/* data must be sent */
	case TELNET_EV_SEND:
		_send(sock, ev->data.buffer, ev->data.size);
		break;
	/* request to enable remote feature (or receipt) */
	case TELNET_EV_WILL:
		/* we'll agree to turn off our echo if server wants us to stop */
		if (ev->neg.telopt == TELNET_TELOPT_ECHO)
			do_echo = 0;
		break;
	/* notification of disabling remote feature (or receipt) */
	case TELNET_EV_WONT:
		if (ev->neg.telopt == TELNET_TELOPT_ECHO)
			do_echo = 1;
		break;
	/* request to enable local feature (or receipt) */
	case TELNET_EV_DO:
		break;
	/* demand to disable local feature (or receipt) */
	case TELNET_EV_DONT:
		break;
	/* respond to TTYPE commands */
	case TELNET_EV_TTYPE:
		/* respond with our terminal type, if requested */
		if (ev->ttype.cmd == TELNET_TTYPE_SEND) {
			telnet_ttype_is(telnet, getenv("TERM"));
		}
		break;
	/* respond to particular subnegotiations */
	case TELNET_EV_SUBNEGOTIATION:
		break;
	/* error */
	case TELNET_EV_ERROR:
		fprintf(stderr, "ERROR: %s\n", ev->error.msg);
		logStream.str("");
		logStream << "Telnet: " << ev->error.msg;
		LOG4CXX_ERROR(Utils::getLogger(), logStream.str());
		exit(1);
	default:
		/* ignore */
		break;
	}
	
}

ReflectNode::ReflectNode(const char* propsLoc, wstring mainEventID, const char *logFileLoc) : FSNode(propsLoc, logFileLoc), mainEventID(mainEventID) {
    
    EventWrapper ew;
    
    int nparameters = 0;
    struct DataProducerInit *dpis = (struct DataProducerInit *)malloc(sizeof(struct DataProducerInit) * 256);
    
    strcpy(telnetHost, props.getValue(TELNET_HOST).c_str());
    strcpy(telnetPort, props.getValue(TELNET_PORT).c_str());
       
    nhardware = 0;
    vector<HardwareXml> hwXmls = getParsedHardwareXmls();
    for(unsigned int i=0; i<hwXmls.size(); i++) {
        FSHardware* hardware = new FSHardware(hwXmls[i].getUniqueID(), hwXmls[i].getFields());
        vector<string> parXmls = hwXmls[i].getParametersUniqueIDS();        
        for(unsigned int j=0; j<parXmls.size(); j++) {
            ReflectParameter *parameter = new ReflectParameter(parXmls[j]);
            nparameters++;
            hardware->addParameter(parameter);
        }
        dpis[i].node      = this;
        dpis[i].hardware  = hardware;
        nhardware++;
        addHardware(hardware);
    }
	
	// Connect to FS server
	connectToServer();
    loadSavedValues();
    
    //Create all the threads and synch mechanism
    evtSem.Create();
        
    //Create all the producers
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(&threads[0], NULL, dataProducer, (void*) dpis);
}

void ReflectNode::newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend) {
    
}

void ReflectNode::setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList) {
    wchar_t* evtList_uid = NULL;
    for(unsigned int i=0; i<evtList.length(); i++) {
        evtList_uid = CORBA::wstring_dup(evtList[i].nameUniqueID);
        if(evtList_uid == mainEventID) {
            mainEvent = evtList[i];
            setNodeStatus(hardware::OPERATIONAL);
            changeAllHardwareStatus(hardware::RUNNING);
            setNodeStatus(hardware::NRUNNING);
            evtSem.Post();            
        }
        CORBA::wstring_free(evtList_uid);
    }
}

void *ReflectNode::dataProducer(void* args) {
    struct DataProducerInit *dpi = (DataProducerInit *)args;
    ReflectNode             *node      = dpi[0].node;
    FSHardware           	*hardware  = NULL;
    ReflectParameter     	*parameter = NULL;
    stringstream         	logStream; 
    stringstream 		defsweepcom;   
    EventWrapper mainEvt;
    vector < EventWrapper > eventList;
    
    char buffer[512];
    int rs;
    int sock;
    struct sockaddr_in addr;
    struct pollfd pfd[2];
    struct addrinfo *ai;
    struct addrinfo hints;
    struct termios tios;

    while(true) {
        node->setNodeStatus (hardware::STAND_BY);
        node->changeAllHardwareStatus (hardware::STOPPED);

        node->evtSem.ResetWait ();
        eventList.clear ();
        mainEvt = node->mainEvent;
        eventList.push_back (mainEvt);

	// for now we assume there is only one hardware and parameter
	hardware  = dpi[0].hardware;
	if(!hardware->isConnected()){
		exit(1);
	}
	
	//Check how many channels are connected for this hardware           
	vector<FSParameter*> hwParameters = hardware->getAllParameters();
    int numberOfConnectedChannels = 0;
	for(unsigned int j=0; j < hwParameters.size(); j++){
		parameter = (ReflectParameter *)hwParameters[j];
		if(parameter->isConnected()){
			numberOfConnectedChannels++;
		}
	}

	if(numberOfConnectedChannels == 0){
		logStream.str("");
		logStream << "The hardware " << hardware->getUniqueID() << " is connected but the number of connected channels is 0";
		LOG4CXX_INFO(Utils::getLogger(), logStream.str());
		exit(1);
	}
	
	parameter = (ReflectParameter*)hwParameters[0];
	if(!parameter->isConnected()){
		exit(1);
	}
	
	if(parameter->getMode() == MODE_FIXED) {
		commandList.push("trigger d\n");
		commandList.push("mode n\n");
        defsweepcom.str("");
		defsweepcom << "freq " << parameter->getFrequency() << '\n';
		commandList.push(defsweepcom.str());
	} 
	else if(parameter->getMode() == MODE_SWEEP) {
		commandList.push("trigger d\n");
		commandList.push("mode n\n");
		defsweepcom.str("");
		defsweepcom << "defsw " << parameter->getMinFrequency() << ' ' << parameter->getMaxFrequency() << ' ' << parameter->getSweepRate() << '\n';
		commandList.push(defsweepcom.str());
		commandList.push("mode s\n");
		commandList.push("trigger e\n");
	}
	else {
	    logStream.str("");
            logStream << "Error in hardware " << hardware->getUniqueID() << " parameter " << parameter->getUniqueID() << ": Mode " << parameter->getMode(); 
	    LOG4CXX_INFO(Utils::getLogger(), logStream.str());
	    exit(1);
	}
	
	commandList.push("exit\n");
	
	//Starting connection to the reflectometry box
    /* look up server host */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rs = getaddrinfo(node->telnetHost, node->telnetPort, &hints, &ai)) != 0) {
		logStream.str("");
		logStream << "Telnet: getaddrinfo() failed for " << node->telnetHost << ":" << gai_strerror(rs);
		LOG4CXX_ERROR(Utils::getLogger(), logStream.str());
		exit(1);
	}
	
	/* create server socket */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		logStream.str("");
		logStream << "Telnet: socket() failed: " << strerror(errno);
		LOG4CXX_ERROR(Utils::getLogger(), logStream.str());
		exit(1);
	}

	/* bind server socket */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		logStream.str("");
		logStream << "Telnet: bind() failed: " << strerror(errno);
		LOG4CXX_ERROR(Utils::getLogger(), logStream.str());
		exit(1);
	}

	/* connect */
	if (connect(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
		logStream.str("");
		logStream << "Telnet: server() failed: " << strerror(errno);
		LOG4CXX_ERROR(Utils::getLogger(), logStream.str());
		exit(1);
	}

	/* free address lookup info */
	freeaddrinfo(ai);

	/* get current terminal settings, set raw mode, make sure we
	 * register atexit handler to restore terminal settings
	 */
	tcgetattr(STDOUT_FILENO, &orig_tios);
	atexit(_cleanup);
	tios = orig_tios;
	cfmakeraw(&tios);
	tcsetattr(STDOUT_FILENO, TCSADRAIN, &tios);

	/* set input echoing on by default */
	do_echo = 1;

	/* initialize telnet box */
	telnet = telnet_init(telopts, _event_handler, 0, &sock);

	/* initialize poll descriptors */
	memset(pfd, 0, sizeof(pfd));
	pfd[0].fd = STDIN_FILENO;
	pfd[0].events = POLLIN;
	pfd[1].fd = sock;
	pfd[1].events = POLLIN;
	// telnet connection should be ready
	
    while (poll(pfd, 2, -1) != -1) {
		/* read from client */
		if (pfd[1].revents & POLLIN) {
			if ((rs = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
				telnet_recv(telnet, buffer, rs);
			} else if (rs == 0) {
				break;
			} else {
				logStream.str("");
				logStream << "Telnet: recv(client) failed: " << strerror(errno);
				LOG4CXX_INFO(Utils::getLogger(), logStream.str());
				break;
			}
		}
	}

        LOG4CXX_INFO (Utils::getLogger (), "Finished sending commands.");
    }
    /* clean up */
    telnet_free(telnet);
    close(sock);   

    /*node->setNodeStatus(hardware::STAND_BY);
    node->changeAllHardwareStatus(hardware::STOPPED);
        
    node->evtSem.ResetWait();

    return NULL; */
}

