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

char errmsg[80];
int rc;
char DEVNAME[] = "/dev/pci_tr512_0";
void printStatus_tr512(status_port_t * pstatP);

int main(int argc, char **argv) {
  //  unsigned char data[0x100000];//byte,dummy;
  //  unsigned char * pdata = data;;
  unsigned int cfg;
  //  unsigned int regval;
  //  int i, fd, iRet, cnt;
  int fd;
  char * devn ;
  int flags = 0;
  FILE *stream_in;



  /* Opening the device  */
  if(argc <2)
    devn = DEVNAME;
  else
    devn = argv[1];

  if((stream_in = fopen("config.txt", "r"))!= NULL ){
    fscanf(stream_in, "%8x", &cfg);
    fclose(stream_in);
  }
  printf("cfg:0x%x\n", cfg);  

  flags |=  O_RDWR;// O_WRONLY;//;
  fd=open(devn,flags);
  if (fd==-1)
    { fprintf (stderr,"Error: cannot open device %s \n",argv[1]);
      fprintf (stderr," errno = %i\n",errno);
      exit(1);
    }

/*   rc = ioctl(fd, CFN_C641X_IOCG_STATUS, &sPort); */
/*   if (rc == -1) { */
/*     perror("ioctl CFN_C641X_IOCG_STATUS"); */
/*     exit(1); */
/*   } */
  rc = ioctl(fd, CFN_C641X_IOCS_CONFIG_ACQ, &cfg);
  if (rc == -1) {
    perror("ioctl CFN_C641X_IOCS_CONFIG_ACQ ");
    exit(1);
  }
  //  printStatus_tr512(&sPort);


  close(fd);
  return 0;
}

void printStatus_tr512(status_port_t * pstatP){

  printf("Status Port: pciIntSrc: 0x%08X.\n", pstatP->pciIntSrc);
  printf("Status Port: hostcommand: 0x%0X.\n", pstatP->hostcommand);
  printf("Status Port: blockCounter: %d.\n", pstatP->blockCounter);
  //  printf("Status Port: memoryFull: %d.\n", pstatP->memoryFull);
  printf("Status Port: triggerBlock: %d.\n", pstatP->triggerBlock);
  printf("Status Port: triggerOffset: %d.\n", pstatP->triggerOffset);
  printf("Status Port: triggersReceived: %d.\n", pstatP->triggersReceived);
  printf("StatPort.");
  //  printStatFlags_tr512((StatFlags *) &pstatP->fpgaStat);
  printf("StatPort.cfgWrd: .clkSrc: %d,", pstatP->configWrd.clkSrc);
  printf(" .polTrig: %d,", pstatP->configWrd.polTrig);
  printf(" .trigAStop: %d,", pstatP->configWrd.trigAutoStop);
  printf(" .decFact: %d,", pstatP->configWrd.decimateFact);
  printf(" .circBfSze: %d.\n", pstatP->configWrd.circBuffSize);
  printf("Status Port: procDataError: %d.\n", pstatP->procDataError);
}

/* void printStatFlags_tr512(StatFlags * pstatF){ */
/*   printf("StatFlags. vcxoLck: %d,", pstatF->vcxoLck); */
/*   printf(" samplDcm: %d,", pstatF->samplDcm); */
/*   printf(" emifaDcm: %d,", pstatF->emifaDcm); */
/*   printf(" brdMaster: %d,", pstatF->brdMaster); */
/*   printf(" brdSynch: %d,", pstatF->brdSynch); */
/*   printf(" brdTrig: %d.\n", pstatF->brdTrig); */
/* } */
