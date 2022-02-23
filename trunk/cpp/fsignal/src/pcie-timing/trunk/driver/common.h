/**
 * Project Name:  Mercury KX1 Timing PCIe Vivado
 * Design Name:    Linux Device Driver Common defs
 *   PCI Device Id: 70
 *   FW Version
 *    working  with kernel 3.10.0 ,
 * Internal definitions for all parts (prototypes, data, macros)
 *
 * SVN keywords
 * $Date: 2017-11-22 17:54:37 +0000 (Wed, 22 Nov 2017) $
 * $Revision: 11341 $
 * $URL: http://metis.ipfn.ist.utl.pt:8888/svn/cdaq/INSTRUMENTATION/1U_Data_Aquisition/Firmware/Vivado-16.4/mercury-kx1/Software/trunk/driver/common.h $
 *
 * Copyright 2014 - 201 IPFN-Instituto Superior Tecnico, Portugal
 * Creation Date  2017-09-10
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
#ifndef _DRIVER_COMMON_H
#define _DRIVER_COMMON_H

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/stat.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wait.h>
/* 
 * <linux/uaccess.h> was added in kernel version 2.6.18, and should be
 * included in preference to <asm/uaccess.h>.  In particular, copy_to_user()
 * and copy_from_user() were moved to <linux/uaccess.h> in kernel version
 * 4.12.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif
#include <asm/atomic.h>
//#include <asm/uaccess.h>
#include <asm/msr.h>

/*
 * Kernel 5.0 removed VERIFY_READ and VERIFY_WRITE and removed the first
 * parameter of access_ok() which was set to VERIFY_READ or VERIFY_WRITE.
 * That has been redundant since kernel 2.5.70, and even then it was only
 * checked for kernels that support old 386 processors.
 *
 * Get rid of the first parameter and always pass VERIFY_WRITE for kernels
 * prior to 5.0.  This will fail for old 386 processors on pre-2.5.70
 * kernels if the memory region is not in fact writeable.
 */
#ifdef VERIFY_WRITE
/* Pre 5.0 kernel. */
static inline int _kcompat_access_ok(unsigned long addr, size_t size)
{
    /* Always use VERIFY_WRITE.  Most architectures ignore it. */
    return access_ok(VERIFY_WRITE, addr, size);
}
/* Redefine access_ok() to remove first parameter. */
#undef access_ok
#define access_ok(addr, size) _kcompat_access_ok((unsigned long)(addr), (size))
#endif

#include "../include/kx1-pcie-timing.h"

/*************************************************************************/
/* Private data types and structures */

typedef struct _BAR_STRUCT {
	unsigned long start;
	unsigned long end;
	unsigned long len;
	unsigned long flags;
	void* vaddr;
} BAR_STRUCT;

/*
typedef struct _DMA_BUF {
	void* addr_v;
	dma_addr_t addr_hw;
} DMA_BUF;

typedef struct _DMA_STRUCT {
  unsigned int buf_size;
  unsigned int buf_actv;
  dma_addr_t hw_actv;
  u32 *  daddr_actv; // ptr to data on active buffer
  DMA_BUF buf[DMA_BUFFS];
} DMA_STRUCT;

typedef struct _READ_BUF {
	int count;
	int total;
	u32* buf;				// assume that ADC data is 32bit wide
} READ_BUF;

  typedef struct _STATUS_FLDS {
  u32 RevId:8,  rsv0:22, DmaC:1, rsv1:1; // msb
} STATUS_FLDS;
/dd*
typedef struct _STATUS_REG {
  union {
    u32 reg32;
    STATUS_FLDS statFlds;
  };
} STATUS_REG;

*/
typedef struct _DMA_CURR_BUFF {
    union
    {
      u32 reg32;
      struct  {
	u32 DmaBuffNum:3, DmaSel:1,  rsv0:28;
      } dmaFlds;
    };
} DMA_CURR_BUFF;

typedef struct _PCIE_SHAPI_HREGS {
  volatile u32              shapiVersion;      	/*Offset 0x00 ro */
  volatile u32              firstModAddress;    /*Offset 0x04 ro */
  volatile u32              hwIDhwVendorID;     /*Offset 0x08 ro*/
  volatile u32              devFwIDdevVendorID; /*Offset 0x0C ro */
  volatile u32              fwVersion;      	/*Offset 0x10 ro */
  volatile u32              fwTimeStamp;      	/*Offset 0x14 ro*/
  volatile u32              fwName[3];      	/*Offset 0x18 ro*/
  volatile u32              devCapab;      	/*Offset 0x24 ro*/
  volatile u32              devStatus;      	/*Offset 0x28 ro*/
  volatile u32              devControl;      	/*Offset 0x2C rw*/
  volatile u32              devIntMask;      	/*Offset 0x30 rw*/
  volatile u32              devIntFlag;      	/*Offset 0x34 ro*/
  volatile u32              devIntActive;      	/*Offset 0x38 ro*/
  volatile u32              scratchReg;      	/*Offset 0x3C rw*/
} PCIE_SHAPI_HREGS;

typedef struct _SHAPI_MOD_TIMING_HREGS {
  volatile u32              shapiVersion;      	/*Offset 0x00 ro */
  volatile u32              nextModAddress;     /*Offset 0x04 ro */
  volatile u32              modFwIDmodVendorID; /*Offset 0x08 ro*/
  volatile u32              modVersion;      	/*Offset 0x0C ro */
  volatile u32              modName[2];      	/*Offset 0x10 ro*/
  volatile u32              modCapab;      	/*Offset 0x18 ro*/
  volatile u32              modStatus;      	/*Offset 0x1C ro*/
  volatile u32              modControl;      	/*Offset 0x20 rw*/
  volatile u32              modIntID;      	/*Offset 0x24 rw*/
  volatile u32              modIntFlagClear;   	/*Offset 0x28 ro*/
  volatile u32              modIntMask;      	/*Offset 0x2C rw*/
  volatile u32              modIntFlag;      	/*Offset 0x30 ro*/
  volatile u32              modIntActive;      	/*Offset 0x34 ro*/
  volatile u32              _reserved1[3];         /*Offset 0x38 - 0x40  */

  volatile u32              modTimingControl;   /*Offset 0x44 rw*/
  volatile u32              modTimingCount;     /*Offset 0x48 ro*/
  volatile u32              modTimingInput;     /*Offset 0x4C ro*/
  EVENT_REGS                timingRegs[NUM_TIMERS];
} SHAPI_MOD_TIMING_HREGS;

/*Structure for pcie access*/
typedef struct _PCIE_DEV {
  /* char device */
  struct cdev cdev;     /* linux char device structure   */
  struct pci_dev *pdev; /* pci device */
  dev_t devno;          /* char device number */
  struct device *dev;
  unsigned char irq;
  spinlock_t irq_lock; //static
  unsigned int counter;
  unsigned int counter_hw;
  unsigned int open_count;
  struct semaphore open_sem;    //mutual exclusion semaphore
  wait_queue_head_t rd_q;       //read  queues
  long wt_tmout;                //read timeout
  atomic_t rd_condition;
  unsigned int mismatches;
  unsigned int max_buffer_count;
  unsigned int curr_buf;

  BAR_STRUCT memIO[2];
  //DMA_STRUCT dmaIO;
  //READ_BUF bufRD;               // buffer struct for read() ops
  PCIE_SHAPI_HREGS *pShapiHregs;
  SHAPI_MOD_TIMING_HREGS *pTimingHregs;

} PCIE_DEV ;

/*I/O Macros*/
#define PCIE_READ32(addr)			ioread32(addr)
#define PCIE_WRITE32(value, addr)		iowrite32(value, addr)
#define PCIE_FLUSH32()				PCIE_READ32()

/*************************************************************************/
/* Some nice defines that make code more readable */
/* This is to print nice info in the log

#ifdef DEBUG
#define mod_info( args... ) \
  do { printk( KERN_INFO "%s - %s : ", MODNAME , __FUNCTION__ );\
    printk( args ); } while(0)
#define mod_info_dbg( args... ) \
  do { printk( KERN_INFO "%s - %s : ", MODNAME , __FUNCTION__ );\
       printk( args ); } while(0)
#else
#define mod_info( args... ) \
  do { printk( KERN_INFO "%s: ", MODNAME );\
       printk( args ); } while(0)
#define mod_info_dbg( args... )
#endif

#define mod_crit( args... ) \
  do { printk( KERN_CRIT "%s: ", MODNAME );\
       printk( args ); } while(0)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

**/

#endif  // _DRIVER_COMMON_H
