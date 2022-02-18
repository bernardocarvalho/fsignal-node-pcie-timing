#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "pci_epn_lib.h"
#include "pci_epn_ioctl.h"

char errmsg[80];
int rc;
char DEVNAME[] = "/dev/pci_epn_0";

void printStatus_tr512(status_port_t * pstatP);

int main(int argc, char **argv) {
  unsigned char data[0x100000];//byte,dummy;
  unsigned char * pdata = data;;
  unsigned int uval;
  //  unsigned int regval;
  int i, fd, iRet, cnt;
  char * devn ;
  int flags = 0, nval=0;
  static char sInput[256];
  status_port_t sPort;
  char *fileboot="boot.bin";
  FILE *stream_in;

  /* Opening the device  */
  if(argc <2)
    devn = DEVNAME;
  else
    devn = argv[1];
  flags |=  O_WRONLY;//O_RDWR;
  fd=open(devn,flags);
  if (fd==-1)
    { fprintf (stderr,"Error: cannot open device %s for reading\n",argv[1]);
      fprintf (stderr," errno = %i\n",errno);
      exit(1);
    }
  //  n=0;
  if((stream_in = fopen(fileboot, "r"))!= NULL ){
    while( !feof(stream_in) ){
      fscanf(stream_in, "%2x", &uval );
      data[nval++]=uval & 0xFF;
    }
    fclose(stream_in);
    printf("final byte #: %d\n", nval );
  }
  else return -1;

/*   rc = ioctl(fd, CFN_C641X_IOCG_STATUS, &sPort); */
/*   if (rc == -1) { */
/*     perror("ioctl CFN_C641X_IOCG_STATUS"); */
/*     exit(1); */
/*   } */
  rc = ioctl(fd, PCI_EPN_IOCT_DSP_HOLD);
  if (rc == -1) {
    perror("ioctl PCI_EPN_IOCT_DSP_HOLD");
    exit(1);
  }
  //  printStatus_tr512(&sPort);

  while(nval > 0) {
    //Instruction : do analog input reads
    //    pdata=data + addr;
    cnt = (nval>1024)? 1024 : nval;
    rc=write(fd, pdata, cnt);
    if (rc == -1) {
      perror("write error");
      exit(1);
    }
    nval -=cnt;
    pdata+=cnt;
  }
  rc = ioctl(fd, PCI_EPN_IOCT_DSP_FREE);
  if (rc == -1) {
    perror("ioctl PCI_EPN_IOCT_DSP_FREE");
    exit(1);
  }

  close(fd);
  return 0;
}

void printStatus_tr512(status_port_t * pstatP){

  printf("Status Port: pciIntSrc: 0x%08X.\n", pstatP->pciIntSrc);
  printf("Status Port: hostcommand: 0x%0X.\n", pstatP->hostcommand);
/*   printf("Status Port: blockCounter: %d.\n", pstatP->blockCounter); */
/*   printf("Status Port: memoryFull: %d.\n", pstatP->memoryFull); */
/*   printf("Status Port: triggerBlock: %d.\n", pstatP->triggerBlock); */
/*   printf("Status Port: triggerOffset: %d.\n", pstatP->triggerOffset); */
/*   printf("Status Port: triggersReceived: %d.\n", pstatP->triggersReceived); */
  printf("StatPort.");
  //  printStatFlags_tr512((StatFlags *) &pstatP->fpgaStat);
//  printf("StatPort.cfgWrd: .clkSrc: %d,", pstatP->configWrd.clkSrc);
/*   printf(" .polTrig: %d,", pstatP->configWrd.polTrig); */
/*   printf(" .trigAStop: %d,", pstatP->configWrd.trigAutoStop); */
/*   printf(" .decFact: %d,", pstatP->configWrd.decimateFact); */
/*   printf(" .circBfSze: %d.\n", pstatP->configWrd.circBuffSize); */
~  printf("Status Port: procDataError: %d.\n", pstatP->procDataError);
}

/* void printStatFlags_tr512(StatFlags * pstatF){ */
/*   printf("StatFlags. vcxoLck: %d,", pstatF->vcxoLck); */
/*   printf(" samplDcm: %d,", pstatF->samplDcm); */
/*   printf(" emifaDcm: %d,", pstatF->emifaDcm); */
/*   printf(" brdMaster: %d,", pstatF->brdMaster); */
/*   printf(" brdSynch: %d,", pstatF->brdSynch); */
/*   printf(" brdTrig: %d.\n", pstatF->brdTrig); */
/* } */
