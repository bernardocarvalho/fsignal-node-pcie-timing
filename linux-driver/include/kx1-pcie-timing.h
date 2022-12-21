/**
 * KC705_PCIE barebone device driver
 * Definitions for the Linux Device Driver
 *
 * SVN keywords
 * $Date: 2019-11-06 12:47:38 +0000 (Wed, 06 Nov 2019) $
 * $Revision: 14892 $
 * $URL: http://metis.ipfn.ist.utl.pt:8888/svn/cdaq/INSTRUMENTATION/1U_Data_Aquisition/Firmware/Vivado-16.4/mercury-kx1/Software/trunk/include/kx1-pcie-timing.h $
 *
 */
#ifndef _KX1_PCIE_H_
#define _KX1_PCIE_H_

#ifndef __KERNEL__
#define __u32 unsigned int
#endif

#define DMA_MAX_BYTES  2048
#define NUM_TIMERS  16

//TOD : to be used.
#ifdef __BIG_ENDIAN_BTFLD
 #define BTFLD(a,b) b,a
#else
 #define BTFLD(a,b) a,b
#endif

typedef struct _EVENT_REGS {
  __s32 delay;
  __u32 period;
  __u32 config;
  __u32 reserved;
} EVENT_REGS;

typedef struct _TIMING_CHANNEL {
  __u32 channel;
  EVENT_REGS evnt_regs;
} TIMING_CHANNEL;

/*
typedef struct _OFFSET_REGS {
  u32 offset[16];
} OFFSET_REGS;


typedef struct _DRIFT_REGS {
  u32 drift[16];
} DRIFT_REGS;
*/

/*
 * 8 + 24 bit field

typedef struct _REVID_FLDS {
  u32 RevId:4, TMR:1, HDR:1, DBG:1, reserved:1, none:24;
} REVID_FLDS;
 */



#undef PDEBUG             /* undef it, just in case */
#ifdef ATCA_DEBUG
#  ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "kx1-pcie: " fmt, ## args)
#  else
/* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#endif /* _KX1_PCIE_H_ */
