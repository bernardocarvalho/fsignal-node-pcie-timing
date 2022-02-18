#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#include "pci_epn_lib.h"
#include "pci_epn_ioctl.h"

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


int main(int argc, char **argv) {
    int  fd ;
    char * devn ;
    int flags = 0;
    dual64_t var64;
    status_port_t sPort;
    struct timeval tval;
        //struct timezone tze;
  
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
  rc += ioctl(fd, PCI_EPN_IOCG_STATUS, &sPort);
  if (rc == -1) {
    perror("ioctl PCI_EPN_IOCG_STATUS ");
    exit(1);
  }
  printf("timeReg1: 0x%X. ", sPort.timeReg1.By_32.dataHigh);
  printf(" 0x%X\n", sPort.timeReg1.By_32.dataLow);

  
  gettimeofday(&tval,  NULL);
  printf("System Time: sec:%d, usec:%d\n", (unsigned int) tval.tv_sec,
         (unsigned int) tval.tv_usec);

  var64.By_32.dataHigh=(unsigned int) tval.tv_sec; // Seconds
  var64.By_32.dataLow = ((unsigned int)tval.tv_usec) * 1000; // nanoSeconds

  rc = ioctl(fd, PCI_EPN_IOCS_SET_TIME, &var64);
  if (rc == -1) {
    perror("ioctl PCI_EPN_IOCS_SET_TIME ");
    exit(1);
  }

  rc = ioctl(fd, PCI_EPN_IOCT_PING);
  rc = ioctl(fd, PCI_EPN_IOCG_STATUS, &sPort);
  if (rc == -1) {
    perror("ioctl PCI_EPN_IOCG_STATUS ");
    exit(1);
  }
  printf("timeReg1:   0x%X %X \n ", sPort.timeReg1.By_32.dataHigh,
         sPort.timeReg1.By_32.dataLow);
  printf("timeReg1:    sec:%d. ", sPort.timeReg1.By_32.dataHigh);
  printf("  nsec:%d\n", sPort.timeReg1.By_32.dataLow);


  close(fd);
  return 0;
}


