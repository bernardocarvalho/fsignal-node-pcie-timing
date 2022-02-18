/**
 * KC705 PCIe Vivado Project General  
 * Project Name:   
 * Design Name:  Test Timers on KC705 Vivado Project
 * FW Version 
 * working  with kernel 3.10.58 
 *
 * SVN keywords
 * $Date: 2015-11-06 16:46:06 +0000 (Fri, 06 Nov 2015) $
 * $Revision: 7931 $
 * $URL: http://metis.ipfn.ist.utl.pt:8888/svn/cdaq/Users/Bernardo/FPGA/Vivado/KC705/Software/trunk/driver/kc705-pcie-drv.c $
 *
 * Copyright 2014 - 2015 IPFN-Instituto Superior Tecnico, Portugal
 * Creation Date  2014-02-10
 * 
 * Licensed under the EUPL, Version 1.1 or - as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <fcntl.h>
//#include <linux/types.h>
#include <math.h>
//#include <signal.h>
#include <string.h>

#include "kc705-pcie-ioctl.h"

char DEVNAME[] = "/dev/kc705_pcie1";

int main(int argc, char **argv)
{

  int  rc, fd;
  unsigned int uval;
  char * devn  = DEVNAME;
  int flags = 0;
  //  FILE * fd_wrt;
  //  int32_t * dataBuff; //[DMA_ACQ_SIZE / sizeof(int16_t) ]; // user space buffer for data
  //unsigned int Npackets=1;
  /* 
 if(argc > 2)
    devn = argv[2];
  else
    devn = DEVNAME;
  if(argc > 1){
    Npackets = atoi(argv[1]);
  }
  */
  if(argc > 1)
    devn = argv[1];
  //  else
  ///  devn = DEVNAME;
/*
   else{
    printf("%s  [dev_name]\n", argv[0]);
    return -1;
  }
  */

  flags |= O_RDONLY;
  printf("opening device %s\t", devn);
  extern int errno;
  fd=open(devn,flags);

  if (fd < 0)    {
    fprintf (stderr,"Error: cannot open device %s \n", devn);
    fprintf (stderr," errno = %i\n",errno);
    printf ("open error : %s\n", strerror(errno));
    exit(1);
  }
  printf("device opened: \n");  // /Opening the device

  uval =0;
  rc = ioctl(fd, KC705_PCIE_IOCS_TMRGATE, &uval); 

  uval = 0x02000000; //~0.3 s 33 554 432
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR0COUNT, &uval); 

  uval =0x8; //load counter
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR0CTRL, &uval); 

  uval = 50000000; //0.5 s 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR1COUNT, &uval); 
  uval = 60000000; 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR2COUNT, &uval); 
  uval = 70000000; 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR3COUNT, &uval); 
  uval =0x8; //load counter
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR1CTRL, &uval); 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR2CTRL, &uval); 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR3CTRL, &uval); 

  //  uval =0x0; // Trigger  Mode
  uval =0x3; // Square wave Mode
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR0CTRL, &uval); 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR1CTRL, &uval); 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR2CTRL, &uval); 
  rc = ioctl(fd, KC705_PCIE_IOCS_TMR3CTRL, &uval); 

  uval =0xF;
  rc = ioctl(fd, KC705_PCIE_IOCS_TMRGATE, &uval); 

  close(fd);

  return 0;
}
