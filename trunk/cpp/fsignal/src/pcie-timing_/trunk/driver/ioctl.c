/**
 *
 * @file ioctl.c
 * @author Bernardo Carvalho
 * @date 2014-06-27
 * @brief Contains the functions handling the different ioctl calls.
 *
 * SVN keywords
 * $Author: bernardo $
 * $Date: 2017-09-08 13:40:33 +0100 (Fri, 08 Sep 2017) $
 * $Revision: 10769 $
 * $URL: http://metis.ipfn.ist.utl.pt:8888/svn/cdaq/INSTRUMENTATION/1U_Data_Aquisition/Firmware/Vivado-16.4/mercury-kx1/Software/trunk/driver/ioctl.c $
 *
* Copyright 2017 IPFN-Instituto Superior Tecnico, Portugal
*  
* Licensed under the EUPL, Version 1.2 or - as soon they
will be approved by the European Commission - subsequent
versions of the EUPL (the "Licence");
* You may not use this work except in compliance with the
Licence.
* You may obtain a copy of the Licence at:
*  
*
https://joinup.ec.europa.eu/software/page/eupl
*  
* Unless required by applicable law or agreed to in
writing, software distributed under the Licence is
distributed on an "AS IS" basis,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
express or implied.
* See the Licence for the specific language governing
permissions and limitations under the Licence.
*/ 


/* Configuration for the driver (what should be compiled in, module name, etc...) */
#include "config.h"

/* Internal definitions for all parts (includes, prototypes, data, macros) */
#include "common.h"

#include "../include/kx1-pcie-timing-ioctl.h"


/**
 * _unlocked_ioctl
 */
long _unlocked_ioctl(struct file *filp,  unsigned int cmd, unsigned long arg){

  int err = 0, retval = 0;
  unsigned long  flags  = 0;
  u32 tmp; 
  TIMING_CHANNEL    tReg;  
  PCIE_DEV *pciDev; /* for device information */
  //STATUS_REG sReg;

  /* retrieve the device information  */
  pciDev = (PCIE_DEV *)filp->private_data;

 // sReg.reg32 = ioread32( (void*) &pciDev->pHregs->status);
  //if(sReg.reg32 == 0xFFFFFFFF)
    //PDEBUG("ioctl status Reg:0x%X, cmd: 0x%X\n", sReg.reg32, cmd);
  
  /**
   * extract the type and number bitfields, and don't decode
   * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
   */
  if (_IOC_TYPE(cmd) != KX1_PCIE_IOC_MAGIC) return -ENOTTY;
  if (_IOC_NR(cmd) > KX1_PCIE_IOC_MAXNR) return -ENOTTY;

  /*
   * the direction is a bitmask, and VERIFY_WRITE catches R/W
   * transfers. `Type' is user-oriented, while
   * access_ok is kernel-oriented, so the concept of "read" and
   * "write" is reversed
   */
  if (_IOC_DIR(cmd) & _IOC_READ)
    err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
  else if (_IOC_DIR(cmd) & _IOC_WRITE)
    err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
  if (err) return -EFAULT;
  switch(cmd) {

  case KX1_PCIE_IOCG_SHAPI_VER:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    //  ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    tmp = PCIE_READ32((void*) &pciDev->pShapiHregs->shapiVersion); 
    //  ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);

    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case KX1_PCIE_IOCG_SHAPI_HW_ID:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pShapiHregs->hwIDhwVendorID);
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCG_SHAPI_FW_ID:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pShapiHregs->devFwIDdevVendorID);
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCG_SHAPI_FW_VER:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pShapiHregs->fwVersion);
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCG_SHAPI_FW_TSTAMP:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pShapiHregs->fwTimeStamp);
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case KX1_PCIE_IOCS_SHAPI_SCRATCH:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- -----
      iowrite32(tmp, (void*) &pciDev->pShapiHregs->scratchReg);  // write to the FPGA
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- -----
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KX1_PCIE_IOCG_SHAPI_SCRATCH:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pShapiHregs->scratchReg);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case KX1_PCIE_IOCG_MOD_SHAPI_VER:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pTimingHregs->shapiVersion);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCG_MOD_FW_ID:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pTimingHregs->modFwIDmodVendorID);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCG_MOD_VER:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pTimingHregs->modVersion);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCS_TIMNG_CNTRL:
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- -----
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pTimingHregs->modTimingControl);  // write the to the FPGA
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- -----
    break;
  case KX1_PCIE_IOCG_TIMNG_CNTRL:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pTimingHregs->modTimingControl);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCG_TIMNG_INPUT:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pTimingHregs->modTimingInput);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCG_TIMNG_COUNT:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pTimingHregs->modTimingCount);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
  case KX1_PCIE_IOCS_TIMER_REGS:
    if(copy_from_user(&tReg, (void __user *)arg, sizeof(tReg)))
      return -EFAULT;
    if( tReg.channel<0)
	return -EINVAL;   
    if( tReg.channel >= NUM_TIMERS)
	return -EINVAL;
    spin_lock_irqsave(&pciDev->irq_lock, flags);    
	iowrite32(tReg.evnt_regs.delay, (void*) &pciDev->pTimingHregs->timingRegs[tReg.channel].delay);  
	iowrite32(tReg.evnt_regs.period, (void*) &pciDev->pTimingHregs->timingRegs[tReg.channel].period);  
	iowrite32(tReg.evnt_regs.config, (void*) &pciDev->pTimingHregs->timingRegs[tReg.channel].config);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
   
    PDEBUG("ioctl tReg:%d, delay: %u, 0x%08X, 0x%08X\n", tReg.channel, tReg.evnt_regs.delay, 
	tReg.evnt_regs.period,tReg.evnt_regs.config);
    //PDEBUG("ioctl ptReg:0x%lx\n", (void*) &pciDev->pTimingHregs->timingRegs[tReg.channel].delay);
    break;
/*  
  case KX1_PCIE_IOCS_TIMER_REGS:
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- -----
  //\dd  retval = __get_user(tReg, (TIMING_CHANNEL __user *)arg);
    //if(copy_from_user(&tReg, (void __user *)arg, sizeof(tReg)))
    //  return -EFAULT;

//   if (!retval){
  //    spin_lock_irqsave(&pciDev->irq_lock, flags);    
    PDEBUG("ioctl Reg:0x%X, cmd: 0x%X\n", tReg.channel, tReg.regs.delay);
//      iowrite32(tmp, (void*) &pciDev->pTimingHregs->modTimingControl);  // write the to the FPGA
    //  spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- -----
    break;
 case  KC705_PCIE_IOCS_RDTMOUT:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval)
      pciDev->wt_tmout = tmp * HZ;
    break;
 *
 *  #define KX1_PCIE_IOCT_INT_ENABLE   _IO(KX1_PCIE_IOC_MAGIC, 1)
 *  #define KX1_PCIE_IOCT_INT_DISABLE  _IO(KX1_PCIE_IOC_MAGIC, 2)
 *  #define KX1_PCIE_IOCG_SHAPI_VER        _IOR(KX1_PCIE_IOC_MAGIC, 3,  u_int32_t)
 *  #define KX1_PCIE_IOCS_SHAPI_SCRATCH    _IOW(KX1_PCIE_IOC_MAGIC, 4,  u_int32_t)
 *  #define KX1_PCIE_IOCG_SHAPI_SCRATCH    _IOR(KX1_PCIE_IOC_MAGIC, 5,  u_int32_t)
 *  #define KX1_PCIE_IOCS_TIMNG_CNTRL      _IOW(KX1_PCIE_IOC_MAGIC, 6,  u_int32_t)
 *  #define KX1_PCIE_IOCG_TIMNG_CNTRL     _IOR(KX1_PCIE_IOC_MAGIC, 7,  u_int32_t)
 *  #define KX1_PCIE_IOCG_TIMNG_COUNT     _IOR(KX1_PCIE_IOC_MAGIC, 8,  u_int32_t)
 *
 *  #define KX1_PCIE_IOCS_TMR0_DELAY      _IOW(KX1_PCIE_IOC_MAGIC, 9,  u_int32_t)
 *  #define KX1_PCIE_IOCG_TMR0_DELAY      _IOR(KX1_PCIE_IOC_MAGIC, 10,  u_int32_t)
 *  #define KX1_PCIE_IOCS_TMR0_REG1      _IOW(KX1_PCIE_IOC_MAGIC, 11,  u_int32_t)
 *  #define KX1_PCIE_IOCG_TMR0_REG1      _IOR(KX1_PCIE_IOC_MAGIC, 12,  u_int32_t)
 *  #define KX1_PCIE_IOCS_TMR0_REG2      _IOW(KX1_PCIE_IOC_MAGIC, 13,  u_int32_t)
 *  #define KX1_PCIE_IOCG_TMR0_REG2      _IOR(KX1_PCIE_IOC_MAGIC, 14,  u_int32_t)
 *
 *  case KC705_PCIE_IOCG_STATUS:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    //  ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    tmp = PCIE_READ32((void*) &pciDev->pHregs->status); 
    //  ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);

    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case KC705_PCIE_IOCT_INT_ENABLE:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    cReg.reg32=PCIE_READ32((void*) &pciDev->pHregs->command);
    //    cReg.cmdFlds.ACQ/iE=1;
    cReg.cmdFlds.DmaIntE=1;
    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    break;

  case KC705_PCIE_IOCT_INT_DISABLE:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    cReg.reg32=PCIE_READ32((void*) &pciDev->pHregs->command);
    //    cReg.cmdFlds.A1~E=0;
    cReg.cmdFlds.DmaIntE=0;
    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    break;

  case KC705_PCIE_IOCT_ACQ_ENABLE:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    cReg.reg32=PCIE_READ32((void*) &pciDev->pHregs->command);
    pciDev->mismatches=0;
    pciDev->curr_buf=0;
    pciDev->max_buffer_count=0;
    atomic_set(&pciDev->rd_condition, 0);
    //    cReg.cmdFlds.ACQE=1;
    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    break;

  case KC705_PCIE_IOCT_ACQ_DISABLE : 
    retval = pciDev->max_buffer_count;
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    cReg.reg32=PCIE_READ32((void*) &pciDev->pHregs->command);
    cReg.cmdFlds.AcqE=0;
    //    cReg.cmdFlds.STRG=0;
    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
	
    break;
  case KC705_PCIE_IOCT_DMA_ENABLE:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    cReg.reg32=PCIE_READ32((void*) &pciDev->pHregs->command);
    cReg.cmdFlds.DmaE=1;
    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);

    break;
  case KC705_PCIE_IOCT_DMA_DISABLE : 
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    cReg.reg32=PCIE_READ32((void*) &pciDev->pHregs->command);
    cReg.cmdFlds.DmaE=0;
    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
	
    break;
*/
    /*
  case KC705_PCIE_IOCG_COUNTER:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    tmp = PCIE_READ32((void*) &pciDev->pHregs->hwcounter); 
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
    */
    /**
     ** Not used yet in this Board 

  case KC705_PCIE_IOCT_SOFT_TRIG:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    cReg.reg32=PCIE_READ32((void*) &pciDev->pHregs->command);
    cReg.cmdFlds.STRG=1;
    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
    // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    break;
     */
/*
  case  KC705_PCIE_IOCS_DMA_SIZE:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->dmaNbytes);  // write the buffer size to the FPGA
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_DMA_SIZE:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->dmaNbytes);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMRGATE:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timersGate);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMRGATE:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timersGate);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR0CTRL:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer0Control);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR0CTRL:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer0Control);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR0COUNT:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer0Count);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR0COUNT:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer0Count);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR1CTRL:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer1Control);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR1CTRL:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer1Control);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR1COUNT:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer1Count);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR1COUNT:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer1Count);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR2CTRL:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer2Control);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR2CTRL:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer2Control);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR2COUNT:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer2Count);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR2COUNT:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer2Count);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR3CTRL:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer3Control);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR3CTRL:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer3Control);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;

  case  KC705_PCIE_IOCS_TMR3COUNT:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      spin_lock_irqsave(&pciDev->irq_lock, flags);    
      iowrite32(tmp, (void*) &pciDev->pHregs->timer3Count);  
      spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    }
    break;

  case KC705_PCIE_IOCG_TMR3COUNT:
    spin_lock_irqsave(&pciDev->irq_lock, flags);
      tmp = ioread32((void*) &pciDev->pHregs->timer3Count);  
    spin_unlock_irqrestore(&pciDev->irq_lock, flags);
    if(copy_to_user((void __user *)arg, &tmp, sizeof(u32)))
      return -EFAULT;
    break;
*/
  default:  /* redundant, as cmd was checked against MAXNR */
    return -ENOTTY;
  }
  return retval;
}

