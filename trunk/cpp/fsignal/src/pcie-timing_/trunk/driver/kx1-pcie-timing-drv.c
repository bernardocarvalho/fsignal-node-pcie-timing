/**
 * Mercury KX1 Timing PCIe Vivado Project General   Linux driver 
 * Project Name:   
 * Design Name:    
 * Linux Device Driver
 * PCI Device Id: 70
 * FW Version 
 * working  with kernel 3.10.58 , NOT with 3.14.x
 *
 * SVN keywords
 * $Date: 2017-09-08 13:40:33 +0100 (Fri, 08 Sep 2017) $
 * $Revision: 10769 $
 * $URL: http://metis.ipfn.ist.utl.pt:8888/svn/cdaq/INSTRUMENTATION/1U_Data_Aquisition/Firmware/Vivado-16.4/mercury-kx1/Software/trunk/driver/kx1-pcie-timing-drv.c $
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

/* Configuration for the driver (what should be compiled in, module name, etc...) */
#include "config.h"

/* Internal definitions for all parts (includes, prototypes, data, macros) */
#include "common.h"

#include "ioctl.h"

/* Check macros and kernel version first */
#ifndef KERNEL_VERSION
#error "No KERNEL_VERSION macro! Stopping."
#endif

#ifndef LINUX_VERSION_CODE
#error "No LINUX_VERSION_CODE macro! Stopping."
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#error "This driver has been tested only for Kernel 3.10.0 or above."
#endif

/* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32) */
/* #ifndef init_MUTEX */
/* #define init_MUTEX(_m) sema_init(_m,1); */
/* #endif // #ifndef init_MUTEX#endif */
/* #endif */

/*
 * Global variables
 */

int device_major = 0;

static struct pci_device_id ids[] = {
  { PCI_DEVICE(PCI_VENDOR_ID_XILINX, PCI_DEVICE_ID_FPGA) },
  { 0, },
};

MODULE_DEVICE_TABLE(pci, ids);

struct class *kx1_pcie_class;

/*function prototypes*/
int _probe(struct pci_dev *pdev, const struct pci_device_id *id) ;
void _remove(struct pci_dev *pdev);

static struct pci_driver _pcie_pci = {
  .name = DRV_NAME,
  .id_table = ids,
  .probe = _probe,
  .remove = _remove
};

struct file_operations _fops;

/**
 * _pcie_open
 */
int _pcie_open(struct inode *inode, struct file *filp) {
  PCIE_DEV *pciDev;   /* device information */

  /** retrieve the device information  */

  pciDev = container_of(inode->i_cdev,  PCIE_DEV, cdev);

  if (down_interruptible(&pciDev->open_sem))
    return -ERESTARTSYS;
    
  filp->private_data = pciDev; //for other methods
  //atomic_set(&pciDev->rd_condition, 0); // prepare to read *****************************
  up(&pciDev->open_sem);    

  return 0;

}

/**
 * _pcie_release
 *  		called by close() sys call
 */
int _pcie_release(struct inode *inode, struct file *filp) {
  PCIE_DEV *pciDev;   /* device information */
  
  /**    retrieve the device information  */

  pciDev = container_of(inode->i_cdev,  PCIE_DEV, cdev);
  down(&pciDev->open_sem);
  filp->private_data = NULL;
  up(&pciDev->open_sem);

  return 0;
}

/**
 * _pcie_read
 */
ssize_t _pcie_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
  u32* data;
  int _ret;
  ssize_t retval = 0;
//  COMMAND_REG    cReg;

  PCIE_DEV *pciDev = (PCIE_DEV *)filp->private_data; /* device information */

  PDEBUG("_pcie_read  count %d \n", (int) count);

  /*check size ligned, if not return error, also alloc memory */
  if ((count > DMA_MAX_BYTES)) {
    retval=-1;
    goto out;
  }
/*  cReg.reg32 = ioread32( (void*) &pciDev->pHregs->command);
  cReg.cmdFlds.DmaE=1;
  cReg.cmdFlds.AcqE=1;
  //
  iowrite32(cReg.reg32, (void*) &pciDev->pHregs->command);
*/
  if(atomic_read(&pciDev->rd_condition) == 0)
    {
      if(wait_event_interruptible_timeout(pciDev->rd_q, atomic_read(&pciDev->rd_condition) !=0, 
					  pciDev->wt_tmout)==0)
	{
	  //printk(KERN_ALERT "_pcie read: wait_q timeout\n");
	  goto out;
	}
    }

  /* Copy buffer in buf with copy_to_user */
  data = pciDev->dmaIO.buf[pciDev->curr_buf].addr_v;
  _ret = copy_to_user(buf, data, count);
  if (_ret) {
    printk(KERN_ERR "_pcie_read: copy_to_user error:%d\n", _ret);
    return -EFAULT;
  }

  *f_pos += count;
  retval = count;

  atomic_dec(&pciDev->rd_condition);
  if (pciDev->curr_buf == (DMA_BUFFS-1))
    pciDev->curr_buf = 0;
  else
    pciDev->curr_buf++;
 out: 
  return retval;
}

/**
 * _pcie_write
 */
ssize_t _pcie_write(struct file *file, const char *buf, size_t count, loff_t * ppos) {
  printk(KERN_WARNING "_pcie_write: not implemented\n");
  return 0;
}

struct file_operations _fops = {
 owner:   THIS_MODULE,
 read:    _pcie_read,
 write:   _pcie_write,
 // ioctl:   pcieAdc_ioctl,
 .unlocked_ioctl  = _unlocked_ioctl,
 open:    _pcie_open,
 release: _pcie_release,
};

/**
 * _irq_handler
 */
static irqreturn_t _irq_handler(int irq, void* dev_id) { 
  PCIE_DEV      *pciDev;
  unsigned long  flags;
  int tmp;
  irqreturn_t    _ret = IRQ_HANDLED;    
  int buffer_number=0;
  int dma_channel_active;
  //STATUS_REG  sReg;
  //COMMAND_REG    cReg;
 
  pciDev = (PCIE_DEV *) pci_get_drvdata(dev_id);
  spin_lock_irqsave(&pciDev->irq_lock, flags);
/*
  //Ack  IRQ
  sReg.reg32 = ioread32( (void*) &pciDev->pHregs->status);
  if(sReg.statFlds.DmaC==1)  {
    //    sReg.statFlds.DmaC==1
    iowrite32(sReg.reg32, (void*) &pciDev->pHregs->status);
   }

  cReg.reg32 = ioread32( (void*) &pciDev->pHregs->command);
  cReg.cmdFlds.DmaE=0;
  //AcqE
  iowrite32(cReg.reg32, (void*) &pciDev->pHregs->command);

  buffer_number = ioread32((void*) &pciDev->pHregs->dmaCurrBuff);
*/  PDEBUG("_irq_handler dmaCurrBuff 0x%X \n", buffer_number);
  dma_channel_active = (0x8 & buffer_number) >> 3; // dma channel active
  buffer_number = (0x00000007 & buffer_number);
  pciDev->curr_buf=buffer_number;

  tmp = atomic_read(&pciDev->rd_condition);
  if (tmp > pciDev->max_buffer_count)
    pciDev->max_buffer_count = tmp;
  if (tmp != 0)
    pciDev->mismatches++;

  atomic_inc(&pciDev->rd_condition);

  if (waitqueue_active(&pciDev->rd_q)){
    wake_up_interruptible(&pciDev->rd_q);
  }

  spin_unlock_irqrestore(&pciDev->irq_lock, flags);
  return _ret;
}

/**
 *
 * DMA management functions *
 */
int enableDMAonboard(struct pci_dev *pdev) {
  int _ret =0;

  _ret=pci_dma_supported(pdev,  DMA_BIT_MASK(32));
  if (! _ret ){
    printk(KERN_ERR "_pcie_probe DMA not supported. EXIT\n");
    return _ret;
  }
  /* enabling DMA transfers */
  _ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
  if (_ret) {
    printk(KERN_ERR "_pcie_probe pci_set_dma_mask error(%d). EXIT\n", _ret);
    return _ret;
  }
  _ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
  if (_ret) {
    printk(KERN_ERR "_pcie_probe pci_set_consistent_dma_mask error(%d). EXIT\n", _ret);
    return _ret;
  }
  /* setting DMA mastering mode */
  (void) pci_set_master(pdev);
  /*  _ret = pci_set_mwi(pdev);
      if (_ret) {
      printk(KERN_ERR "_pcie_probe pci_set_mwi error(%d). EXIT\n", _ret);
      return _ret;
      }
  */
  return _ret;

}

int configurePCI(PCIE_DEV *pcieDev) {
  u16 reg16 = 0;
  int i = 0;
  int _ret = 0;

  //set PCI command register
  pci_read_config_word(pcieDev->pdev, PCI_COMMAND, &reg16);
  reg16 &= ~PCI_COMMAND_IO; // disable IO port access
  reg16 |= PCI_COMMAND_PARITY; // enable parity error hangs
  reg16 |= PCI_COMMAND_SERR; // enable addr parity error
  pci_write_config_word(pcieDev->pdev, PCI_COMMAND, reg16);

  //PCI reading IO memory spaces and set virtual addresses
  for (i = 0; i < NUM_BARS; i++) {
    pcieDev->memIO[i].start = pci_resource_start(pcieDev->pdev, i);
    pcieDev->memIO[i].end = pci_resource_end(pcieDev->pdev, i);
    pcieDev->memIO[i].len = pci_resource_len(pcieDev->pdev, i);
    pcieDev->memIO[i].flags = pci_resource_flags(pcieDev->pdev, i);
    // virtual addr
    pcieDev->memIO[i].vaddr = ioremap_nocache(pcieDev->memIO[i].start,
					      pcieDev->memIO[i].len);
    PDEBUG("_pcie_probe BAR %d: start 0x%X, end 0X%X, len 0x%X, flags 0x%X \n", i,
	   (int) pcieDev->memIO[i].start,
	   (int) pcieDev->memIO[i].end,
	   (int) pcieDev->memIO[i].len,
	   (int) pcieDev->memIO[i].flags);

    if (!pcieDev->memIO[i].vaddr) {
      printk(KERN_ERR "_pcie: error in ioremap_nocache [%d]. Aborting.\n", _ret);
      return -ENOMEM;
    }
  }
  //virtual pointer to board registers
//  pcieDev->pHregs = (PCIE_HREGS *) pcieDev->memIO[1].vaddr;
  pcieDev->pShapiHregs = (PCIE_SHAPI_HREGS *) pcieDev->memIO[1].vaddr;
  pcieDev->pTimingHregs = (SHAPI_MOD_TIMING_HREGS *) (pcieDev->memIO[1].vaddr + 
	 ioread32((void*) &pcieDev->pShapiHregs->firstModAddress));
  //PDEBUG("pS 0x%lx, pT 0x%lx\n", pcieDev->pShapiHregs,  pcieDev->pTimingHregs);

  return _ret;
}

int setupDMA(PCIE_DEV *pcieDev) {
  int i = 0;
  /**
     setting DMA regions */
  pcieDev->dmaIO.buf_size = DMA_MAX_BYTES;//PAGE_SIZE * (1 << GFPORDER);// 
//  iowrite32(pcieDev->dmaIO.buf_size, (void*) &pcieDev->pHregs->dmaNbytes);  // write the buffer size to the FPGA

  //PDEBUG("setupDMA MAX  0x%08x, DMA_SIZE 0x%08x\n", DMA_MAX_BYTES,
//	 ioread32((void*) &pcieDev->pHregs->dmaNbytes));

  /* set up a coherent mapping through PCI subsystem */
  for( i=0; i<DMA_BUFFS; i++){  

    pcieDev->dmaIO.buf[i].addr_v = pci_alloc_consistent(pcieDev->pdev, pcieDev->dmaIO.buf_size,
							&(pcieDev->dmaIO.buf[i].addr_hw));
    if (!pcieDev->dmaIO.buf[i].addr_v || !pcieDev->dmaIO.buf[i].addr_hw) {
      printk(KERN_ERR "_pcie_probe pci_alloc_consistent error(v:%p hw:%p). EXIT\n",
	     (void*)pcieDev->dmaIO.buf[i].addr_v, (void*)pcieDev->dmaIO.buf[i].addr_hw);
      return -ENOMEM;
    }
    memset((void*)(pcieDev->dmaIO.buf[i].addr_v), 0, pcieDev->dmaIO.buf_size);
  }

  //for( i=0; i < DMA_BUFFS; i++) // WRITE pci MA registers
  //  iowrite32(pcieDev->dmaIO.buf[i].addr_hw, (void*) &pcieDev->pHregs->HwDma1Addr[i]);

  pcieDev->dmaIO.buf_actv=0;
  pcieDev->dmaIO.hw_actv=pcieDev->dmaIO.buf[0].addr_hw;

  return 0;
}

/*
 * probe
 */
int _probe(struct pci_dev *pdev, const struct pci_device_id *id) {
  int _ret;
  PCIE_DEV * pciDev = NULL;
  u32 name[4];	
  u32 scrth;
  //STATUS_REG  sReg;
  //COMMAND_REG    cReg;
  u32   _minor;

  /* allocate the device instance block */
  pciDev = kzalloc(sizeof(PCIE_DEV), GFP_KERNEL);
  if (!pciDev) {
    return -ENOMEM;
  }
  pciDev->pdev = pdev;
  pciDev->wt_tmout = 5 * HZ; /*time out in sec*/
  pci_set_drvdata(pdev, pciDev);

  //  init_MUTEX(&pciDev->open_sem);
  sema_init(&pciDev->open_sem, 1);
  /* enabling PCI board */
  _ret = pci_enable_device(pdev);
  if (_ret) {
    printk(KERN_ERR "_pcie_probe pci_enable_device error(%d). EXIT\n", _ret);
    return _ret;
  }
 
  /*enable DMA transfers */
  _ret = enableDMAonboard(pdev);
  if (_ret != 0) {
    printk(KERN_ERR "_pcie: error in DMA initialization. Aborting.\n");
    return _ret;
  }

  /* configure PCI and remap I/O */
  _ret = configurePCI(pciDev);
  if (_ret != 0) {
    printk("KERN_ERR _pcie: error in PCI configuration. Aborting.\n");
    return _ret;
  }
  /*Set up DMA
  _ret = setupDMA(pciDev);
  if (_ret != 0) {
    printk("KERN_ERR _pcie: error in DMA setup. Aborting.\n");
    return _ret;
  }
*/
  PDEBUG("shapiVersion Reg 0x%08x\n",
	 ioread32((void*) &pciDev->pShapiHregs->shapiVersion));
  PDEBUG("firstModAddress Reg 0x%08x\n",
	 ioread32((void*) &pciDev->pShapiHregs->firstModAddress));
  PDEBUG("shapiVersion Time Stamp 0x%08x\n",
	 ioread32((void*) &pciDev->pShapiHregs->fwTimeStamp));
  name[0] = ioread32((void*) &pciDev->pShapiHregs->fwName[0]);
  name[1] = ioread32((void*) &pciDev->pShapiHregs->fwName[1]);
  name[2] = ioread32((void*) &pciDev->pShapiHregs->fwName[2]);
  name[3] = 0;
  PDEBUG("fwName: %s\n", (char *) name);	
  scrth = ioread32((void*) &pciDev->pShapiHregs->scratchReg);
  PDEBUG("scratchReg Reg 0x%08x\n", scrth);	
  iowrite32(scrth + 1, (void*) &pciDev->pShapiHregs->scratchReg);
  PDEBUG("scratchReg Reg 0x%08x\n",
	 ioread32((void*) &pciDev->pShapiHregs->scratchReg));
  //PDEBUG("ModVersion Reg 0x%08x\n",
//	 ioread32((void*) (&pciDev->pShapiHregs->shapiVersion + 0x10)));
  PDEBUG("Mod shapiVersion Reg 0x%08x\n",
	 ioread32((void*) &pciDev->pTimingHregs->shapiVersion));
  PDEBUG("Mod fwID Reg 0x%08x\n",
	 ioread32((void*) &pciDev->pTimingHregs->modFwIDmodVendorID));
  name[0] = ioread32((void*) &pciDev->pTimingHregs->modName[0]);
  name[1] = ioread32((void*) &pciDev->pTimingHregs->modName[1]);
  name[2] = 0;
  PDEBUG("Mod Name: %s\n", (char *) name);	

  iowrite32(0x0, (void*) &pciDev->pTimingHregs->modTimingControl);
  PDEBUG("modTimingCount  0x%08x\n",
	 ioread32((void*) &pciDev->pTimingHregs->modTimingCount));
  iowrite32(0x0000000F, (void*) &pciDev->pTimingHregs->modTimingControl);
  PDEBUG("modTimingControl  0x%08x\n",
	 ioread32((void*) &pciDev->pTimingHregs->modTimingControl));
  PDEBUG("modTimingCount  0x%08x\n",
	 ioread32((void*) &pciDev->pTimingHregs->modTimingCount));
	 //ioread32((void*) &pciDev->pTimingHregs->modVersion));
  PDEBUG("pS 0x%lx, pT 0x%lx\n", (void*) &pciDev->pShapiHregs->shapiVersion,
  (void*) &pciDev->pTimingHregs->modVersion);
/*
  iowrite32(0x1, (void*) &pciDev->pHregs->timersGate);
  PDEBUG("_pcie_probe Command Reg 0x%08x\n",
	 ioread32((void*) &pciDev->pHregs->command));
  PDEBUG("_pcie_probe  DMA_SIZE 0x%08x\n",
	 ioread32((void*) &pciDev->pHregs->dmaNbytes));

  PDEBUG("_pcie_probe Bar DMA_CURR_BUFF 0x%08x\n",
	 ioread32((void*) &pciDev->pHregs->dmaCurrBuff));

  PDEBUG("_pcie_probe Bar 1 triggerDelay 0x%08x\n",
      ioread32((void*) &pciDev->pHregs->triggerDelay));

  */
  PDEBUG("_pcie_probe Bar 0 address 0 0x%08x\n",
	 ioread32(pciDev->memIO[0].vaddr));
	
  // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
  /* Install board IRQ */

  _ret=pci_enable_msi(pdev);  

  if (_ret) {
    printk(KERN_ERR "pci_enable_msi %d error[%d]\n", pdev->irq, _ret);
    return _ret;
  }
  pciDev->irq = pdev->irq;    

  // _ret = request_irq(pdev->irq, _irq_handler, IRQF_SHARED, DRV_NAME, (void*) pdev);
  _ret = request_irq(pdev->irq, _irq_handler, 0, DRV_NAME, (void*) pdev);

  if (_ret) {
    printk(KERN_ERR "request_linux_irq irq %d error[%d]\n", pdev->irq, _ret);
    return _ret;
  }
  PDEBUG("_pcie irq %d handler installed\n", pdev->irq);
    
  spin_lock_init(&pciDev->irq_lock);
  init_waitqueue_head(&pciDev->rd_q);
  _minor = 1; //TODO Change
  pciDev->devno = MKDEV(device_major, _minor);

  cdev_init(&pciDev->cdev, &_fops);
  pciDev->cdev.owner = THIS_MODULE;
  pciDev->cdev.ops = &_fops;

  _ret = cdev_add(&pciDev->cdev,pciDev->devno,1);
  //_ret=0;
  if (_ret ) {
    printk(KERN_ERR "Error %d adding _pcie device", _ret);
    return -EIO;
  }

  pciDev->dev= device_create(kx1_pcie_class, NULL,
			     pciDev->devno, NULL, NODENAMEFMT, _minor);
  /*
    cReg.cmdFlds.ACQE=0;
    cReg.cmdFlds.STREAME=0;
    cReg.cmdFlds.DAC_SHIFT=0xC; // 4 bit field 

    PCIE_WRITE32(cReg.reg32, (void*) &pciDev->pHregs->command);
/  
  cReg.reg32 = ioread32( (void*) &pciDev->pHregs->command);
  sReg.reg32 = ioread32( (void*) &pciDev->pHregs->status);
*/
  printk(KERN_NOTICE "%s installed, major:%d, shapiVersion :0x%8X, Tstamp :0x%8X\n", 
	 DRV_NAME, device_major, ioread32((void*) &pciDev->pShapiHregs->shapiVersion),  ioread32((void*) &pciDev->pShapiHregs->fwTimeStamp));

  return 0;
}

/*
 * remove
 */
void _remove(struct pci_dev *pdev) {
  unsigned long flags;
  int i;
  PCIE_DEV * pciDev;

  /* get the device information daa */
  pciDev = (PCIE_DEV *) pci_get_drvdata(pdev);
  if(pciDev->irq) {
    /* disable registered IRQ */
    //deregistering OS ISR and restore MSI
    free_irq(pciDev->irq, pdev);
    pci_disable_msi(pdev);
  }
  spin_lock_irqsave(&pciDev->irq_lock, flags);
  // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
  /*  Reset  Device */
  //PCIE_WRITE32(0, (void*) &pciDev->pHregs->command );
  //  PCIE_WRITE32(0, (void*) &pciDev->pHregs1~->dmaReg );	/* WRITE pci MA register */
  // ----- ----- ----- ----- ----- ----- DEVICE SPECIFIC CODE ----- ----- ----- ----- ----- -----
  spin_unlock_irqrestore(&pciDev->irq_lock, flags);
  cdev_del(&pciDev->cdev);
  //  unregister_chrdev_region(pciDev->devno,1);
  device_destroy(kx1_pcie_class, pciDev->devno);

  /* deregistering DMAable areas and virtual addresses for the board */
/*  for( i=0; i<DMA_BUFFS; i++)
    pci_free_consistent(pdev, pciDev->dmaIO.buf_size,
			pciDev->dmaIO.buf[i].addr_v,
			pciDev->dmaIO.buf[i].addr_hw);
*/
for( i=0; i<2; i++)
    iounmap(pciDev->memIO[i].vaddr);
  
  /* disable PCI board */
  kfree(pciDev);
  pci_set_drvdata(pdev, NULL);
  pci_clear_mwi(pdev);
  pci_disable_device(pdev);
  printk(KERN_NOTICE "%s removed. \n", DRV_NAME);
  return;
}

/*
 * _pcie_init
 */
static int __init _pcie_init(void) {
  int _ret;
  dev_t devno = 0;

  devno = MKDEV(0, 0);
  _ret = alloc_chrdev_region(&devno, 0, MINOR_NUMBERS, DRV_NAME);
  if (_ret) {
    printk (KERN_ERR "Failed to register device %s with error %d\n", DRV_NAME, _ret);
    goto fail;
  }
  device_major = MAJOR(devno);
  PDEBUG("_init: device_num:%d\n", device_major);

  kx1_pcie_class = class_create(THIS_MODULE, DRV_NAME);
  if (IS_ERR(kx1_pcie_class)) {
    printk(KERN_ERR "Unable to allocate class\n");
    _ret = PTR_ERR(kx1_pcie_class);

    goto unreg_chrdev;
  }

  /* registering the board */
  _ret = pci_register_driver(&_pcie_pci);
  if (_ret) {
    printk(KERN_ERR "pcieAdc_init pci_register_driver error(%d).\n", _ret);
    goto unreg_class;
    //    return _ret;
  }
  return _ret;
 unreg_class:
  class_unregister(kx1_pcie_class);
  class_destroy(kx1_pcie_class);
 unreg_chrdev:
  unregister_chrdev_region(MKDEV(device_major,0), MINOR_NUMBERS);
 fail:
  return _ret;
  
}

/*
 * _pcie_exit
 */
static void _pcie_exit(void) {
  /* unregistering the board */
  pci_unregister_driver(&_pcie_pci);
  class_unregister(kx1_pcie_class);
  unregister_chrdev_region(MKDEV(device_major,0), MINOR_NUMBERS);
}

module_init(_pcie_init);
module_exit(_pcie_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Module for PCIe KINTEX FPGA ");
MODULE_AUTHOR("Bernardo Carvalho/IST-IPFN");

