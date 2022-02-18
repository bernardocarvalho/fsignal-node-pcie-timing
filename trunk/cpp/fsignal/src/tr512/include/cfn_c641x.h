/*
 * pcie_fpgaendpoint.h -- definitions for the char module
 *
 * Copyright (C) 2007
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: pcie_fpgaendpoint.h,v 1.15 2004/11/04 17:51:18 rubini Exp $
 */
#ifndef _CFN_C641X_H_
#define _CFN_C641X_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/wait.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_START    	0x64000000 //EMIFB CE1 SPACE
#define CHAN_ADDR_0    	0xA0000000
#define DSP_IRAM_PAGE   0x0        //DSP internal Ram Page Space
#define FLASH_PAGE (FLASH_START >>22)

/*communication between the HOST <-> DSP (64kB block) Page 0 */
#define HOST_DSP_COMM  0xD0000    // Host <-> DSP Space
#define HOST2DSP   (HOST_DSP_COMM) // Host -> DSP
#define DSPSTATUS  (HOST_DSP_COMM + 0x1000) // DSP -> Host
#define DSPCONFIG  (HOST_DSP_COMM + 0x2000)

// bits for interrupt reason and control ()
// 1=interrupt occur, enable source,  0=interrupt not occur, disable source
#define INT_PCIDMA	0x08		/* end PCI DMA transfer*/
#define INT_ENDACQ  0x04    	/*  Acquisition  Stopped*/
#define INT_ENDEDMA	0x02		/* A/D FIFO half/full flag */
#define INT_DTRG	0x01		/* external digital trigger */

/** PCI I/O Registers Accessed via I/O Space (Base 2 Memory) **/
enum{
  PCI_IO_HSR = 0x00,
  PCI_IO_HDCR = 0x04,
  PCI_IO_DSPP = 0x08
};

#define HSR_INTSRC 0x1
#define HSR_INTAM_MSK 0x4

/**  PCI Memory-Mapped Peripheral Registers Offsets, Accessed via Base 1 Memory **/
enum{
  PCI_MMAP_RSTSRC = 0x400000,
  PCI_MMAP_PMDCSR = 0x400004,
  PCI_MMAP_PCIIS  = 0x400008,
  PCI_MMAP_PCIIEN = 0x40000C,
  PCI_MMAP_DSPMA  = 0x400010, //DSP master address register
  PCI_MMAP_PCIMA  = 0x400014, //PCI master address register
  PCI_MMAP_PCIMC  = 0x400018, //PCI master control register
  PCI_MMAP_CDSPA  = 0x40001C, //Current DSP address register
  PCI_MMAP_CPCIA  = 0x400020, //Current PCI address register
  PCI_MMAP_CCNT   = 0x400024  //Current byte count register
};

// Host commands:
#define NOTHING        	0x0000
#define INIT           	0x0001
#define CONFIG_ACQ     	0x0002
#define START_ACQ      	0x0003
#define STOP_ACQ       	0x0004
#define CHECK_ERRORS   	0x0005
#define WR_FPGA        	0x0006
#define RD_FPGA        	0x0007
#define SOFT_TRIG      	0x0008
#define PING           	0x000A
#define SET_CLOCK_TYPE 	0x000B
#define TEST_MEM       	0x0010
#define DMA_END       	0x0011

#define HDCR_DSPINT_BIT  0x00000002u
#define HDCR_WARMRESET_BIT  0x00000001u

#define C641X_MEM_BAR0_INDEX		0
#define C641X_MEM_BAR1_INDEX		1
#define C641X_IO_BARINDEX		2

#define C641X_BAR0_PAGE_SIZE    0x400000 // 4MB page

#define TIME_OUT_DLY 500
#define   BUFF_GFPORDER 0 // one page get_zeroed_page

#define NR_CHANNELS 8    /*  */

#define EDMA_BLK_SIZE (256 *2) // fpga->dsp RAM blocks

typedef struct c641x_dev {

  /* char device */
  struct cdev cdev;     	/* Char device structure      */
  struct pci_dev *pdev;		/* pci device */
  dev_t	devno;				/* char device number */
  unsigned open_count;
  unsigned char irq;
  unsigned char brd_rev;	/* PCI_REVISION_ID */
  /* BOARD HW REGISTERS */
  void 	*memio_base; //acsess to 4M-byte prefetchable maps to all of DSP memory
  unsigned long memio_base_phys;
  void 	*mmapreg_base; // access to DSP’s memory-mapped registers
  unsigned short portio_base;
  unsigned int 	modnumber;
  char	valid;		// driver is ok
  unsigned int 	read_count;
  unsigned int 	circ_buf_sz;
  //unsigned int 	ai_scans;
  //unsigned int 	ai_neverending;
  /* DMA BUFFER */
  //  dma_addr_t dma_handle;	/* bus address */
  unsigned int  dma_actbuf;     /* which buffer is used now */
  unsigned int 	dmagfporder;	/* how many gfporder we have allocated */
  unsigned int 	dmabuf_size;	/* DMA buf size in bytes */
  unsigned long dmabuf_virt[2];	/* kernel virtual address  DMA buf*/
  unsigned int 	dmabuf_hw[2];	/* BUS HW address to DMA buf */
  unsigned long chanAddr[8];
  unsigned int 	act_chan;		 /* actual channel to read*/
  struct semaphore open_sem;     /* mutual exclusion semaphore     */
  struct semaphore ioctl_sem;    /* mutual exclusion semaphore     */
  /* statistics */
  //unsigned int overflows;		/* ADC overflow errors */
  //unsigned int overruns;		/* ADC overrun errors */
  unsigned int interrupts;		/* interrupt counter */
  wait_queue_head_t rd_waitq;
  long wt_tmeout;
} c641x_dev_t ;

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* _CFN_C641X_H_ */
