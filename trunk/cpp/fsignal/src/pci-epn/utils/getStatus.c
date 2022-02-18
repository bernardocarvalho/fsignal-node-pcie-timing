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
#include <sys/time.h>
#include <time.h>

typedef struct{
    unsigned int mgtDcm:1;
    unsigned int emifaDcm:1;
    unsigned int unused0:6;
    unsigned int laneUp:8;
    unsigned int channelUp:8;
    unsigned int unused:8;
    unsigned int unusedH;
} FpgaStatFlags; 

char errmsg[80];
int rc;
char DEVNAME[] = "/dev/pci_epn_0";
void printStatus(status_port_t * pstatP);

int main(int argc, char **argv) {

  int  fd ;
  char * devn ;
  int flags = 0;
  status_port_t sPort;
  struct timeval tv;
  time_t curtime;
  char buffer[30];

  //  struct timezone tze;
  
  gettimeofday(&tv,  NULL);
  curtime=tv.tv_sec;

  printf("gettimeofday Time: sec:%d, usec:%d\n", (unsigned int) tv.tv_sec,
         (unsigned int) tv.tv_usec);

  strftime(buffer,30,"%m-%d-%Y  %T.",localtime(&curtime));
  printf("%s%ld\n",buffer,tv.tv_usec);

  
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

  rc = ioctl(fd, PCI_EPN_IOCT_PING);
  if (rc == -1) {
    perror("ioctl PCI_EPN_IOCT_PING ");
    exit(1);
  }
  rc = ioctl(fd, PCI_EPN_IOCG_STATUS, &sPort);
  if (rc == -1) {
    perror("ioctl PCI_EPN_IOCG_STATUS ");
    exit(1);
  }
 printStatus(&sPort);

  close(fd);
  return 0;
}

void printStatus(status_port_t * pstatP){

  unsigned int val;

  printf("Status Port: \n");
  printf(".pciIntSrc: 0x%08X.\n", pstatP->pciIntSrc);
  printf(".hostcommand: 0x%0X.\n", pstatP->hostcommand);
  printf(".fpgaStat: 0x%X\n", pstatP->fpgaStat.By_32.dataLow);
  printf(".fpgaContrl: 0x%X\n", pstatP->fpgaContrl.By_32.dataLow);
  printf(".timeReg0: 0x%X\n", pstatP->timeReg0.By_32.dataLow);
  printf(".timeReg1: 0x%X, 0x%X\n", pstatP->timeReg1.By_32.dataHigh,
         pstatP->timeReg1.By_32.dataLow);
  printf(".timeReg1: %d(sec). ", pstatP->timeReg1.By_32.dataHigh);
  printf("%d(nSec)\n", pstatP->timeReg1.By_32.dataLow );

  printf(".evntReg0: 0x%X\n", pstatP->evntReg.By_32.dataLow);
  printf(".testReg: 0x%X. 0x%X\n",  pstatP->testReg.By_32.dataHigh,
         pstatP->testReg.By_32.dataLow);


  
}

