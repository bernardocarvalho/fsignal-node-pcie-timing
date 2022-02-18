/**
 * KX1_PCIE barebone device driver
 * IOCTL Definitions for the Linux Device Driver
 *
 * SVN keywords
 * $Date: 2017-09-08 13:40:33 +0100 (Fri, 08 Sep 2017) $
 * $Revision: 10769 $
 * $URL: http://metis.ipfn.ist.utl.pt:8888/svn/cdaq/INSTRUMENTATION/1U_Data_Aquisition/Firmware/Vivado-16.4/mercury-kx1/Software/trunk/include/kx1-pcie-timing-ioctl.h $
 *
 */

#ifndef  _KX1_PCIE_IOCTL_H_
#define _KX1_PCIE_IOCTL_H_

#include "kx1-pcie-timing.h"

/*
 * IOCTL definitions
 */

#define KX1_PCIE_IOC_MAGIC 'p'  // /* Please use a different 8-bit number in your code */
/*See  /Documentation/ioctl-number.txt*/

/* S means "Set": thru a pointer
 * T means "Tell": directly with the argument value
 * G menas "Get": reply by setting thru a pointer
 * Q means "Qry": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */

/**********************************************************************
 *                         IOCTL FUNCTIONS                            *
 *********************************************************************/
#define KX1_PCIE_IOCT_INT_ENABLE   _IO(KX1_PCIE_IOC_MAGIC, 1)
#define KX1_PCIE_IOCT_INT_DISABLE  _IO(KX1_PCIE_IOC_MAGIC, 2)
#define KX1_PCIE_IOCG_SHAPI_VER        _IOR(KX1_PCIE_IOC_MAGIC, 3,  u_int32_t)
#define KX1_PCIE_IOCG_SHAPI_HW_ID      _IOR(KX1_PCIE_IOC_MAGIC, 4,  u_int32_t)
#define KX1_PCIE_IOCG_SHAPI_FW_ID      _IOR(KX1_PCIE_IOC_MAGIC, 5,  u_int32_t)
#define KX1_PCIE_IOCG_SHAPI_FW_VER     _IOR(KX1_PCIE_IOC_MAGIC, 6,  u_int32_t)
#define KX1_PCIE_IOCG_SHAPI_FW_TSTAMP  _IOR(KX1_PCIE_IOC_MAGIC, 7,  u_int32_t)
#define KX1_PCIE_IOCS_SHAPI_SCRATCH    _IOW(KX1_PCIE_IOC_MAGIC, 8,  u_int32_t)
#define KX1_PCIE_IOCG_SHAPI_SCRATCH    _IOR(KX1_PCIE_IOC_MAGIC, 9,  u_int32_t)
#define KX1_PCIE_IOCG_MOD_SHAPI_VER    _IOR(KX1_PCIE_IOC_MAGIC, 10,  u_int32_t)
#define KX1_PCIE_IOCG_MOD_FW_ID        _IOR(KX1_PCIE_IOC_MAGIC, 11,  u_int32_t)
#define KX1_PCIE_IOCG_MOD_VER          _IOR(KX1_PCIE_IOC_MAGIC, 12,  u_int32_t)
#define KX1_PCIE_IOCG_MOD_STATUS       _IOR(KX1_PCIE_IOC_MAGIC, 13,  u_int32_t)
#define KX1_PCIE_IOCS_MOD_CONTROL      _IOW(KX1_PCIE_IOC_MAGIC, 14,  u_int32_t)
#define KX1_PCIE_IOCG_MOD_CONTROL      _IOR(KX1_PCIE_IOC_MAGIC, 15,  u_int32_t)
#define KX1_PCIE_IOCS_TIMNG_CNTRL      _IOW(KX1_PCIE_IOC_MAGIC, 16,  u_int32_t)
#define KX1_PCIE_IOCG_TIMNG_CNTRL     _IOR(KX1_PCIE_IOC_MAGIC, 17,  u_int32_t)
#define KX1_PCIE_IOCG_TIMNG_COUNT     _IOR(KX1_PCIE_IOC_MAGIC, 18,  u_int32_t)
#define KX1_PCIE_IOCG_TIMNG_INPUT     _IOR(KX1_PCIE_IOC_MAGIC, 19,  u_int32_t)

#define KX1_PCIE_IOCG_TIMER_REGS      _IOR(KX1_PCIE_IOC_MAGIC, 20, TIMING_CHANNEL)
#define KX1_PCIE_IOCS_TIMER_REGS      _IOW(KX1_PCIE_IOC_MAGIC, 21, TIMING_CHANNEL)

#define KX1_PCIE_IOC_MAXNR   21

/* 
#define KX1_PCIE_IOCT_ACQ_ENABLE   _IO(KX1_PCIE_IOC_MAGIC, 3)
#define KX1_PCIE_IOCT_ACQ_DISABLE  _IO(KX1_PCIE_IOC_MAGIC, 4)
#define KX1_PCIE_IOCT_DMA_ENABLE   _IO(KX1_PCIE_IOC_MAGIC, 5)
#define KX1_PCIE_IOCT_DMA_DISABLE  _IO(KX1_PCIE_IOC_MAGIC, 6)
#define KX1_PCIE_IOCT_SOFT_TRIG    _IO(KX1_PCIE_IOC_MAGIC, 7)
#define KX1_PCIE_IOCS_RDTMOUT       _IOW(KX1_PCIE_IOC_MAGIC, 9,  u_int32_t)
#define KX1_PCIE_IOCS_DMA_SIZE      _IOW(KX1_PCIE_IOC_MAGIC, 10, u_int32_t)
#define KX1_PCIE_IOCG_DMA_SIZE      _IOR(KX1_PCIE_IOC_MAGIC, 11, u_int32_t)

#define KX1_PCIE_IOCS_TMRGATE       _IOW(KX1_PCIE_IOC_MAGIC, 12,  u_int32_t)
#define KX1_PCIE_IOCG_TMRGATE       _IOR(KX1_PCIE_IOC_MAGIC, 13,  u_int32_t)

#define KX1_PCIE_IOCS_TMR3CTRL      _IOW(KX1_PCIE_IOC_MAGIC, 26,  u_int32_t)
#define KX1_PCIE_IOCG_TMR3CTRL      _IOR(KX1_PCIE_IOC_MAGIC, 27,  u_int32_t)
#define KX1_PCIE_IOCS_TMR3COUNT     _IOW(KX1_PCIE_IOC_MAGIC, 28,  u_int32_t)
#define KX1_PCIE_IOCG_TMR3COUNT     _IOR(KX1_PCIE_IOC_MAGIC, 29,  u_int32_t)
#define KX1_PCIE_IOCS_CONFIG_ACQ _IOW(KX1_PCIE_IOC_MAGIC, 2, u_int32_t)
#define KX1_PCIE_IOCT_SOFT_TRIG _IO(KX1_PCIE_IOC_MAGIC, 8)
#define KX1_PCIE_IOCS_TMOUT      _IOW(KX1_PCIE_IOC_MAGIC, 11, u_int32_t)
#define KX1_PCIE_IOCS_CHAN       _IOW(PCIE_ADC_IOC_MAGIC, 12, u_int32_t)
*/
#endif /* _KX1_PCIE_IOCTL_H_ */
