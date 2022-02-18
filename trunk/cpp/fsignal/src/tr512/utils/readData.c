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
#define DATA_SIZE (256)
//#define N_TRANSFERS 32

int main(int argc, char **argv) {
  //unsigned char buff[DATA_SIZE * 2 ];//byte,dummy;
  unsigned char * buff;
  //  short * pshort =(short *)  buff;
  unsigned int dwBytes, nTransfers;
  int i, fd;
  //  short darray[2][256][4];
  char * devn ;
  int flags = 0;
  int chan;
  short **adcData;
  FILE *stream_out;

  //  static char sInput[256];
  status_port_t sPort;

  //  if(argc <3)
  //    size=1024; // one block
    //else
    //size = aoti( argv[2)];
  /* Opening the device  */
  if(argc <2)
    devn = DEVNAME;
  else
    devn = argv[1];
  flags |= O_RDONLY;
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
  nTransfers= sPort.configWrd.circBuffSize;

  //  printStatus_tr512(&sPort);
  /* Get menu option from user */

  i = 20; // seconds
  rc = ioctl(fd, CFN_C641X_IOCS_TMOUT, &i);
  if (rc == -1) {
    perror("ioctl CFN_C641X_IOCS_TMOUT ");
    exit(1);
  }
  
  rc = ioctl(fd, CFN_C641X_IOCT_INT_ENABLE);
  if (rc == -1) {
    perror("ioctl  CFN_C641X_IOCT_INT_ENABLE  ");
    exit(1);
  }
  chan=0;  
  rc = ioctl(fd, CFN_C641X_IOCS_CHAN, &chan );
  if (rc == -1) {
    perror("ioctl CFN_C641X_IOCTS_CHAN  ");
    exit(1);
  }
   
  rc = ioctl(fd, CFN_C641X_IOCT_SOFT_TRIG );
  if (rc == -1) {
    perror("ioctl CFN_C641X_IOCT_SOFT_TRIG  ");
    exit(1);
  }


  //fgets(sInput, sizeof(sInput), stdin);
  //    iRet = sscanf(sInput, "%ld", pdwOption);

  dwBytes = (nTransfers * 0x100) * sizeof(short); 
  buff = malloc(dwBytes); 
  adcData = (short **) malloc(8 * sizeof(short *)); 
  printf("Waiting Trigger ...\n");
  rc=read(fd, buff, dwBytes);
  if(rc<0){
    printf("Timeout rc: %d\n", rc);
    close(fd);
    exit(1);
  }
  else
    printf("read %d bytes\n", rc);
  /*reopen in non-block mode*/
  //  close(fd);
  flags |= O_NONBLOCK;
  //  fd=open(devn,flags);
  fcntl(fd, F_SETFL, O_NONBLOCK);
  //  fd=open(devn,flags);
  adcData[0] = (short *) buff;
  //  i = 0; // seconds
  //rc = ioctl(fd, CFN_C641X_IOCS_TMOUT, &i);
  for(chan = 1 ; chan < 8 ; chan++){ 
    rc = ioctl(fd, CFN_C641X_IOCS_CHAN, &chan );
    buff = malloc(dwBytes); 
    read(fd, buff, dwBytes);
    adcData[chan] = (short *) buff;

    printf("chan %d: ",chan);
    for(i=0; i<10; i++)
      printf("%d ", adcData[chan][i] );
    printf("\n");
  }
  stream_out = fopen("data.txt", "wt");

  for (i=0;  i < dwBytes  / sizeof(short) ; i++ ) {
    fprintf(stream_out, "%d\t", i);
    for(chan = 0 ; chan < 8 ; chan++)
      fprintf(stream_out, "%d\t", adcData[chan][i]);
    fprintf(stream_out, "\n");
  }
  fflush(stream_out);
  fclose(stream_out);
  for(chan = 0 ; chan < 8 ; chan++){ 
    free((void*)adcData[chan]);
  }
  close(fd);
  return 0;
}
//free(pBuf);

/*   GetData(*phDev, chan_addr[chan], pBuf, &dwBytes); */
/*   //adcData[chan] = (short *) pBuf; */
// Dump data to file
/* stream_out = fopen("data.txt", "wt"); */

/* for (i=0;  i < nTransfers * 0x100 ; i++ ) { */
/*   fprintf(stream_out, "%d\t", i); */
/*   for(chan = 0 ; chan < 8 ; chan++) */
/*     fprintf(stream_out, "%d\t", adcData[chan][i]); */
/*   fprintf(stream_out, "\n"); */
/*  } */
/* fflush(stream_out); */
/* fclose(stream_out); */


/* void printStatFlags_tr512(StatFlags * pstatF){ */
/*   printf("StatFlags. vcxoLck: %d,", pstatF->vcxoLck); */
/*   printf(" samplDcm: %d,", pstatF->samplDcm); */
/*   printf(" emifaDcm: %d,", pstatF->emifaDcm); */
/*   printf(" brdMaster: %d,", pstatF->brdMaster); */
/*   printf(" brdSynch: %d,", pstatF->brdSynch); */
/*   printf(" brdTrig: %d.\n", pstatF->brdTrig); */
/* } */
