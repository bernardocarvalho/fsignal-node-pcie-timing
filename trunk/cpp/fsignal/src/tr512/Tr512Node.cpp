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

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cmath>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "Tr512Node.h"
#include "../FSHardware.h"
#include "../FSParameter.h"
#include "../utils/TimeStampWrapper.h"
#include "../utils/FSignalUtils.h"
#include "Tr512Parameter.h"

#include "./include/cfn_c641x_lib.h"
#include "./include/cfn_c641x_ioctl.h"


//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>


using namespace std;
using namespace fsignal;
using namespace org::cfn::scad::core::corba;

// Structure to pass multiple argument to hwThread
typedef struct thread_data{
  Tr512Node* node;
  FSHardware * hardw;
  std::string hdKey;
} thread_data_t;

//Global Values for threads exit Status
const int  thrdOkStatus        = 0;
const int  thrdFailStatus      = -1;

Tr512Node::Tr512Node(const char* propsLoc, wstring mainEventID) : FSNode(propsLoc), 
								  mainEventID(mainEventID)
{
	vector<HardwareXml> hwXmls = getParsedHardwareXmls();
	for(int i=0; i<hwXmls.size(); i++)
	{
		FSHardware* hardware = new FSHardware(hwXmls[i].getUniqueID());		
		vector<string> parXmls = hwXmls[i].getParametersUniqueIDS();
		for(int j=0; j<parXmls.size(); j++)
		{
			Tr512Parameter* parameter = new Tr512Parameter(parXmls[j]);			
			hardware->addParameter(parameter);
		}
		addHardware(hardware);
	}
	
	connectToServer();
	loadSavedValues();
}

void Tr512Node::newDataAvailable(const CORBA::WChar* nodeUniqueID, const CORBA::WChar* hardwareUniqueID, const CORBA::WChar* parameterUniqueID, const fscns::hardware::EventList& eList, const fscns::util::TimeStamp& tstart, const fscns::util::TimeStamp& tend){}

void Tr512Node::setEventTable(const org::cfn::scad::core::corba::hardware::EventList& evtList)
{
	wchar_t* evtList_uid = NULL;	
	for(int i=0; i<evtList.length(); i++)
	{
		evtList_uid = CORBA::wstring_dup(evtList[i].nameUniqueID);
		if(evtList_uid == mainEventID)
		{
			mainEvent = evtList[i];
			pthread_t threads[1];	
			pthread_create(&threads[0], NULL, nodeRun, (void*) this);
		}
		CORBA::wstring_free(evtList_uid);
		cout << "setEventTable End" << endl;
	}
}

unsigned int Tr512Node::bldConfigWdr(unsigned int nSampl, unsigned int decFact, unsigned int polTrigP, 
			  unsigned int clkSrc){
  unsigned int cfg= 0x20000008; // FIXED: 8192 blocks (4Mb) per channel,  auto-stop after EXT trigger

  if(decFact==8)
     cfg+= 0x1000  ; 
  else if(decFact==64)
    cfg+= 0x2000  ; 
  if(polTrigP==1) 
     cfg+= 0x4  ; // Positive Trigger polarity
  return cfg;
}
void * Tr512Node::hwThreadRun(void* args)
{
  thread_data_t *my_data;
  my_data = (thread_data_t *) args;

  Tr512Node* node = my_data->node;
  server::CentralServer_ptr cserver = node->getCentralServer();
  wchar_t* nodeUID = Utils::widenCorba(node->getUniqueID());

  map<string, FSHardware*> hardwareTable = node->getHardwareTable();
  FSHardware* hwPtr = hardwareTable[my_data->hdKey];
  wchar_t* hwUID = Utils::widenCorba(hwPtr->getUniqueID());

  int npars = hwPtr->getNumberOfParameters(); 

  cout << "hw T started: " << my_data->hdKey << ", hwR hd pars: " << npars << endl;
  vector<FSParameter*> pars = hwPtr->getAllParameters();

  // get first parameter to obtain shared fields
  Tr512Parameter par0 = dynamic_cast<Tr512Parameter&>(*pars[0]); 
  unsigned int nSampl  = par0.getNSamples();
  unsigned int decFact = par0.getDecimation();
  unsigned int clkSrc  = par0.getClock();
  unsigned int trgPol  = par0.getTriggerPol();
  unsigned int rdTmOut = par0.getRdTmOut();
  int trgDlay = par0.getTrgDlay();

  node->changeHardwareStatus(hwUID, hardware::CONFIGURING);

  unsigned int cfgWrd=bldConfigWdr(nSampl, decFact, trgPol, clkSrc); 
  //      cout << "cfgWrd: 0x" << hex << cfgWrd  << endl;
  int dev =::open(par0.getDevName(), O_RDWR);
  if (dev == -1){
    cout << "device :"<< par0.getDevName() << " Not opened" << endl;
    node->changeHardwareStatus(hwUID, hardware::ERROR_STATE);
    node->setNodeStatus(hardware::FAULT);
    CORBA::wstring_free(nodeUID);
    CORBA::wstring_free(hwUID);
    pthread_exit((void *) &thrdFailStatus);
  }
  else{
    cout << "dev ::"<< par0.getDevName() << ":: Opened" << endl;
    int rc = ::ioctl(dev, CFN_C641X_IOCS_CONFIG_ACQ, &cfgWrd);
    node->changeHardwareStatus(hwUID, hardware::CONFIGURE);
    usleep(10000); // wait 10ms for hardware
    //    timeout in seconds
    rc = ::ioctl(dev, CFN_C641X_IOCS_TMOUT, &rdTmOut);
    // if (rc == -1) {
    //   perror("::ioctl CFN_C641X_IOCS_TMOUT ");
    //   exit(1);
    // }
    //Just checking if hw is running. 
    //TODO: exit graciouscly
    status_port_t  sPort;
    rc = ::ioctl(dev, CFN_C641X_IOCG_STATUS, &sPort);
    std::cout<<"Block#: "<< sPort.blockCounter ;
    usleep(10000); // wait 10ms for hardware
    rc = ::ioctl(dev, CFN_C641X_IOCG_STATUS, &sPort);
    std::cout<<" Block#: "<< sPort.blockCounter << endl;
    //    ::close(dev);
  }
  node->changeHardwareStatus(hwUID, hardware::RUN);
  //  node->setNodeStatus(hardware::NRUNNING);
  usleep(500000); // sleep 0.5 sec
  vector<short> sBuff(nSampl); 

  db::Data_var data = new db::Data();
  data->tstart = node->mainEvent.tstamp;
  TimeStampWrapper* ts = new TimeStampWrapper(node->mainEvent.tstamp);
  ts->addMicroSeconds( nSampl * decFact / 2 ); // 2MHz / Decimation acquisition
  data->tend = ts->getAsCorbaTimeStamp();
  data->configXML = Utils::widenCorba(par0.getXMLConfiguration());
  /*Get data for each parameter*/
  for(int i=0; i < npars ; i++){ 
    Tr512Parameter par = dynamic_cast<Tr512Parameter&>(*pars[i]);

    if(i==0){
      ::ioctl(dev, CFN_C641X_IOCT_INT_ENABLE);
      if(trgPol==2) /* Soft Trigger the board*/
	::ioctl(dev, CFN_C641X_IOCT_SOFT_TRIG);
    }
    else if(i==1){
      ::fcntl(dev, F_SETFL, O_NONBLOCK); /*change status flag for remaining flags*/
    }
    ::ioctl(dev, CFN_C641X_IOCS_CHAN, &i); /* change channel# to read data */
    int rc = ::read(dev, &sBuff[0], nSampl * sizeof(short));
    if (rc < 0) {
      cout << "read chan:" << i<< ", error:" << errno << endl;
      //   perror("::ioctl CFN_C641X_IOCS_TMOUT ");
      //   exit(1);
    }

    data->datab = Utils::shortVecToByteArray(&sBuff);
    hardware::Event_var evt = new hardware::Event();
    evt->event_unique_id = -1;
    evt->nameUniqueID = node->mainEvent.nameUniqueID;
    TimeStampWrapper* tevt = new TimeStampWrapper(node->mainEvent.tstamp);
    evt->tstamp = tevt->getAsCorbaTimeStamp();

    hardware::EventList_var evtList = new hardware::EventList(1);
    evtList->length(1);
    evtList[0] = evt;
    data->eList = evtList;

    wchar_t* parUID = Utils::widenCorba(par.getUniqueID());
    cserver->newDataAvailable(nodeUID, hwUID, parUID, data);
    CORBA::wstring_free(parUID);

    delete tevt;
  }
  delete ts;
  if (dev != -1)
    ::close(dev);
  node->changeHardwareStatus(hwUID, hardware::STOP);
  //  cout << "hw T stoped: " << my_data->hdKey <<endl;
  CORBA::wstring_free(nodeUID);
  CORBA::wstring_free(hwUID);
  pthread_exit((void *) &thrdOkStatus);  //pthread_exit(NULL);
}

void *Tr512Node::nodeRun(void* args)
{

  Tr512Node* node = (Tr512Node*)args;
  node->setNodeStatus(hardware::OPERATIONAL);

  map<string, FSHardware*> hardwareTable = node->getHardwareTable();
  map<std::string, FSHardware*>::iterator iter;

  unsigned int thrdSz=hardwareTable.size();
  pthread_t * hwThreads = new pthread_t[thrdSz];
  thread_data_t * threadArgs = new thread_data_t[thrdSz];
  pthread_attr_t attr; // is this realy necessary ?

  /* Initialize and set thread detached attribute */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  int thrdCnt=0;
  for(iter = hardwareTable.begin(); iter != hardwareTable.end(); iter++) {
    threadArgs[thrdCnt].node = node;
    threadArgs[thrdCnt].hdKey = iter->first; // Key for hardware
    pthread_create(&hwThreads[thrdCnt], &attr, hwThreadRun, (void*) &threadArgs[thrdCnt]);
    int npars = iter->second->getNumberOfParameters(); // 
    //    cout << "nR hd pars: " << npars << endl;
    thrdCnt++;
  }

  node->setNodeStatus(hardware::NRUNNING);


  /* Free attribute and wait for the other threads */
  pthread_attr_destroy(&attr);

  int allStat=0;
  /*Wait for ALL threads to finish*/ 
  for(thrdCnt=0; thrdCnt < thrdSz; thrdCnt++){
    //    cout << "hw thread waiting: " << threadArgs[thrdCnt].hdKey << endl;
    void * pstat;
    int errcode=pthread_join(hwThreads[thrdCnt], &pstat);

    allStat += *((int*)pstat);
    cout << "hw thread ended: " << threadArgs[thrdCnt].hdKey << " _join error: " << errcode 
	 <<", status:" << *((int*)pstat)  <<endl;
  }
  delete [] threadArgs;
  delete [] hwThreads;

  if(allStat)
    node->setNodeStatus(hardware::FAULT); // if Some hardware failed
  else 
    node->setNodeStatus(hardware::STAND_BY);
  pthread_exit(NULL);
}

/*Testing function in case no hardware */
vector<short>* Tr512Node::getShortWave(Tr512Parameter& par)
{
	float amp = par.getAmplitude();
        int npts = par.getNSamples();

	vector<short>* values = new vector<short>();
	
        for(int i=0; i<npts; i++)
		values -> push_back((short) i );

        return values;
}
