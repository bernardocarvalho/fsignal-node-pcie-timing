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
#ifndef _CFN_C641X_H_LIB_
#define _CFN_C641X_H_LIB_
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef FPGAENDPOINT_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "c641x: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

typedef struct{
  unsigned int clkSrc:2;
  unsigned int polTrig:1;       // Trigger polarity
  unsigned int trigAutoStop:1;  // Trigger AutoStop
  unsigned int streamAcq:1;   // Streaming Acquisition
  unsigned int unused:7;
  unsigned int decimateFact:2;
  unsigned int dcOffsetFact:2;
  unsigned int circBuffSize:16; /* 0x2000, 8192:  Optimal for < 4 Mb transfers*/
} ParamWordFields;

typedef struct{
  unsigned int memFull:1;
  unsigned int AcqStop:1;
  unsigned int unused:30;
} MemoryFlags;

/** Status Port **/
typedef struct status_port {
   unsigned int pciIntSrc;       // Interrupt reason register
   unsigned int hostcommand;     // DSP reflects hostComannd
   unsigned int blockCounter;
   MemoryFlags   memoryFlags;
   unsigned int triggerBlock;    // block # when trgger received
   unsigned int triggerOffset;   // in samples
   unsigned int triggersReceived;
   unsigned int fpgaStat;
   ParamWordFields configWrd;
   unsigned int procDataError;
} status_port_t;

/** Config port **/
typedef struct {
   unsigned int fwLoaded; //
   unsigned int softVersion;
   unsigned int maxBlocks;
   unsigned int channelAdd[8];
} config_port_t;


#define C641X_DMA_SIZE 0x1000	/* DMA Buffer size 4 kb*/
#define C641X_DMA_SIZE 0x1000	/* DMA Buffer size 4 kb*/


//#define DMA_BLK_SIZE (256 *2) //

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* _CFN_C641X_H_LIB_ */
