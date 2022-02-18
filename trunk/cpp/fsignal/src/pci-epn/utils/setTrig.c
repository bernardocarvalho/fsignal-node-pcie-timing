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
void printStatus(status_port_t * pstatP);

int main(int argc, char **argv) {
    int  fd, chan, val ;
    char * devn ;
    int flags = 0;
    dual64_t var64;
    status_port_t sPort;
    struct timeval tval;
    struct timezone tze;
    
  
  /* Opening the device  */
  if(argc <2)
    devn = DEVNAME;
  else
    devn = argv[1];
  flags |=  O_RDWR;

  gettimeofday(&tval,  &tze);
  printf("size %d, sec:%d, usec:%d\n", sizeof(tval), (unsigned int)tval.tv_sec,
         (unsigned int) tval.tv_usec);
  
  fd=open(devn,flags);
  if (fd==-1)
    { fprintf (stderr,"Error: cannot open device %s \n",argv[1]);
      fprintf (stderr," errno = %i\n",errno);
      exit(1);
    }
  rc  = ioctl(fd, PCI_EPN_IOCT_PING);
  rc += ioctl(fd, PCI_EPN_IOCG_STATUS, &sPort);
  if (rc == -1) {
      perror("ioctl PCI_EPN_IOCG_STATUS ");
      exit(1);
  }
  printf("timeReg1: 0x%X. ", sPort.timeReg1.By_32.dataHigh);
  printf(" 0x%X\n", sPort.timeReg1.By_32.dataLow);
  for( chan=0; chan < 16; chan++){
      
      rc = ioctl(fd, PCI_EPN_IOCS_SET_TRG_CHAN, &chan);
  
//  var64.By_32.dataHigh= 0;
      val= 200000000; // nanoSeconds
      rc = ioctl(fd, PCI_EPN_IOCS_SET_WIDTH, &val);
      if (rc == -1) {
          perror("ioctl PCI_EPN_IOCS_SET_TRG ");
          exit(1);
      }
      
      var64.By_32.dataHigh= sPort.timeReg1.By_32.dataHigh + chan +1 ; // Seconds
      var64.By_32.dataLow= sPort.timeReg1.By_32.dataLow; // nanoSeconds
      
      rc = ioctl(fd, PCI_EPN_IOCS_SET_TRG, &var64);
  }
  
  if (rc == -1) {
      perror("ioctl PCI_EPN_IOCS_SET_TRG ");
      exit(1);
  }
    
  close(fd);
  return 0;
}


