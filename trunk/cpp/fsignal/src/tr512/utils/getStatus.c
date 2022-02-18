#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
//#include "fpgaendpoint_ioctl.h"
#include <cfn_c641x_lib.h>
#include <cfn_c641x_ioctl.h>

typedef struct{
  unsigned int vcxoLck:1; 
  unsigned int samplDcm:1; 
  unsigned int emifaDcm:1; 
  unsigned int brdMaster:1; 
  unsigned int brdSynch:1; 
  unsigned int brdTrig:1; 
  unsigned int unused:26; 
  unsigned int unusedH; 
} FpgaStatFlags; 

char errmsg[80];
int rc;
char DEVNAME[] = "/dev/pci_tr512_0";
void printStatus_tr512(status_port_t * pstatP);
void printStatFlags_tr512(FpgaStatFlags * pstatF);

int main(int argc, char **argv) {

  int  fd ;
  char * devn ;
  int flags = 0;
  status_port_t sPort;

  /* Opening the device  */
  if(argc <2)
    devn = DEVNAME;
  else
    devn = argv[1];
  flags |=  O_RDWR;
  fd=open(devn,flags);
  if (fd==-1)
    { fprintf (stderr,"Error: cannot open device %s \n",argv[1]);
      fprintf (stderr," errno = %i\n",errno);
      exit(1);
    }

  rc = ioctl(fd, CFN_C641X_IOCG_STATUS, &sPort);
  if (rc == -1) {
    perror("ioctl CFN_C641X_IOCG_STATUS");
    exit(1);
  }
 printStatus_tr512(&sPort);

  close(fd);
  return 0;
}

void printStatus_tr512(status_port_t * pstatP){

  printf("Status Port: \n");
  printf(".pciIntSrc: 0x%08X.\n", pstatP->pciIntSrc);
  printf(".hostcommand: 0x%0X.\n", pstatP->hostcommand);
  printf(".blockCounter: %d.\n", pstatP->blockCounter);
  printf(".MemoryFlags: .memFull: %d, ", pstatP->memoryFlags.memFull);
  printf(".AcqStop: %d.\n", pstatP->memoryFlags.AcqStop);
  printf(".triggerBlock: %d, ", pstatP->triggerBlock);
  printf(".triggerOffset: %d, ", pstatP->triggerOffset);
  printf(".triggersReceived: %d.\n", pstatP->triggersReceived);
  printf(".fpgaStat: 0x%X\n", pstatP->fpgaStat);

  printStatFlags_tr512((FpgaStatFlags *) &pstatP->fpgaStat);
  printf("SP.cfgWrd: .clkSrc: %d,", pstatP->configWrd.clkSrc);
  printf(" .polTrig: %d,", pstatP->configWrd.polTrig);
  printf(" .trigAStop: %d,", pstatP->configWrd.trigAutoStop);
  printf(" .streamAcq: %d,", pstatP->configWrd.streamAcq);
  printf(" .decFact: %d,", pstatP->configWrd.decimateFact);
  printf(" .circBfSze: %d.\n", pstatP->configWrd.circBuffSize);
  printf("SP.procDataError: %d.\n", pstatP->procDataError);
}

void printStatFlags_tr512(FpgaStatFlags * pstatF){
  printf("StatFlags. vcxoLck: %d,", pstatF->vcxoLck);
  printf(" samplDcm: %d,", pstatF->samplDcm);
  printf(" emifaDcm: %d,", pstatF->emifaDcm);
  printf(" brdMaster: %d,", pstatF->brdMaster);
  printf(" brdSynch: %d,", pstatF->brdSynch);
  printf(" brdTrig: %d.\n", pstatF->brdTrig);
}
