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
#ifndef _PCI_EPN_LIB_H_
#define _PCI_EPN_LIB_H_
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

#ifdef __cplusplus
extern "C" {
#endif

/**
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


// typedef struct{
  // unsigned int memFull:1;
  // unsigned int AcqStop:1;
  // unsigned int unused:30;
// } MemoryFlags;

    typedef struct {
    unsigned int Operator:5;
    unsigned int Priority:3;
    unsigned int LocalAddr:8;
    unsigned int NodeAddr:8;
    unsigned int WordN:4;
    unsigned int CRC:4;
    unsigned int Time;
} T3P_Event_t; 

/** Status Port **/
    typedef union 
    {
            //declare union to allow access variable as 32 or 64 bits
        double By_64;
            //allow 64 bit access
        struct 
        {
            unsigned int dataLow;
            unsigned int dataHigh;
        }   By_32;
    }  dual64_t;

        //Status Port
    typedef struct status_port
    { 
        unsigned int pciIntSrc;         // interrupt reason register
        unsigned int hostcommand;       // DSP reflects hostComannd
        dual64_t fpgaStat;
        dual64_t fpgaContrl;
        dual64_t timeReg0;
        dual64_t timeReg1;
        dual64_t evntReg;
        dual64_t testReg;
        unsigned int procDataError;
    }   status_port_t;


/** Config port **/
typedef struct {
   unsigned int fwLoaded; //
   unsigned int softVersion;
} config_port_t;


#define PCI_EPN_DMA_SIZE 0x1000	/* DMA Buffer size 4 kb*/
#define PCI_EPN_DMA_SIZE 0x1000	/* DMA Buffer size 4 kb*/


//#define DMA_BLK_SIZE (256 *2) //

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* _PCI_EPN_LIB_H_ */
