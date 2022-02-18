/**
 * KX1 PCIe Vivado Project General  
 * Project Name:   
 * Design Name:  Test Timers on KX1 Vivado Project
 * FW Version 
 * working  with kernel 3.10.58 
 *
 * SVN keywords
 * $Date: 2015-11-06 16:46:06 +0000 (Fri, 06 Nov 2015) $
 * $Revision: 7931 $
 * $URL: http://metis.ipfn.ist.utl.pt:8888/svn/cdaq/Users/Bernardo/FPGA/Vivado/KC705/Software/trunk/driver/kc705-pcie-drv.c $
 *
 * Copyright 2014 - 2017 IPFN-Instituto Superior Tecnico, Portugal
 * Creation Date  2017-09-04
 * 
 * Licensed under the EUPL, Version 1.2 or - as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
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

#include "kx1-pcie-timing-ioctl.h"

char DEVNAME[] = "/dev/kx1_pcie_timing1";

int main(int argc, char **argv)
{

  int  i, rc, fd;
  TIMING_CHANNEL tRegs;
  unsigned int uval;
  char * devn  = DEVNAME;
  int flags = 0;
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
  if((rc = ioctl(fd, KX1_PCIE_IOCG_SHAPI_VER, &uval)))
	return -1; 
  printf("MOD SHAPI_VER: 0x%08x\n", uval);
  rc = ioctl(fd, KX1_PCIE_IOCG_MOD_FW_ID, &uval); 
  printf("MOD_FW_ID 0x%08x\n", uval);
  rc = ioctl(fd, KX1_PCIE_IOCG_MOD_VER, &uval); 
  printf("MOD_FW_VER 0x%08x\n", uval);
  uval =0;
  rc = ioctl(fd, KX1_PCIE_IOCS_TIMNG_CNTRL, &uval); 
  rc = ioctl(fd, KX1_PCIE_IOCS_SHAPI_SCRATCH, &uval); 
  rc = ioctl(fd, KX1_PCIE_IOCG_TIMNG_CNTRL, &uval); 
  printf("TIMNG_CNTRLT 0x%08x\n", uval);
  rc = ioctl(fd, KX1_PCIE_IOCG_TIMNG_INPUT, &uval); 
  printf("TIMNG_INPUT 0x%08x\n", uval);
 /*Reset channels*/ 
  tRegs.evnt_regs.delay= 0;
  tRegs.evnt_regs.period= 0;
  tRegs.evnt_regs.config= 0; 
  for(i=0; i < NUM_TIMERS; i++){
	tRegs.channel=i;
        rc = ioctl(fd, KX1_PCIE_IOCS_TIMER_REGS, &tRegs); 
  }
 /* Program channels*/ 

  tRegs.evnt_regs.period= 50000000; //0.5 s
  tRegs.evnt_regs.config=   0x1000000A;// 0xC0000000; //0x80000000 LED ON 2/A:1 C:2
  for(i=0; i < NUM_TIMERS; i++){
	tRegs.channel=i;
  	tRegs.evnt_regs.delay= i*20000000;
        rc = ioctl(fd, KX1_PCIE_IOCS_TIMER_REGS, &tRegs); 
  }
  rc = ioctl(fd, KX1_PCIE_IOCG_TIMNG_COUNT, &uval); 
  printf("TIMNG_COUNT %u\n", uval);
  /*Start Time Counter*/
  uval =0x1;
  rc = ioctl(fd, KX1_PCIE_IOCS_TIMNG_CNTRL, &uval); 
  usleep(500000);
  rc = ioctl(fd, KX1_PCIE_IOCG_TIMNG_COUNT, &uval); 
  printf("TIMNG_COUNT %u\n", uval);
  close(fd);

  return 0;
}
