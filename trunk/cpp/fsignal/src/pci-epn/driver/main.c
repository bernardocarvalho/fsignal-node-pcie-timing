/*Device
  driver/main.c
  Driver for CFN/IST PCI DSP series Boards
*/
/* -*- linux-c -*- */
//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>/* kmalloc() */
#include <linux/fs.h>/* everything... */
#include <linux/errno.h>/* error codes */
#include <linux/types.h>/* size_t */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>

#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <asm/uaccess.h>/* copy_*_user */

#include <pci_epn.h> /* local definitions */
#include <pci_epn_lib.h>
#include <pci_epn_ioctl.h>

/**
 * Module Parameters
 */
//module_param(scull_nr_devs, int, S_IRUGO);
static int minor =  0;
module_param(minor, int, S_IRUGO);
static int major =  0;

#define PCIEPN_DEVICE_ID 0xA106 /* Device ID Todo change*/ 

#define SIGNAL_HSIZE 256 /*1 kB*/
static struct pci_device_id ids[] = { //0x10ee
  { PCI_DEVICE(PCI_VENDOR_ID_TI, PCIEPN_DEVICE_ID), },
  { 0, }
};
MODULE_DEVICE_TABLE(pci, ids);


/*
 * Prototypes for local functions
 */
static int probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void remove(struct pci_dev *pdev);

static struct pci_driver pci_driver = {
  .name = "pciepn_drv",
  .id_table = ids,
  .probe = probe,
  .remove = remove,
};

/*** Static functions ***/
/***
    set DSP Page Register
***/
static inline void set_dsp_page(pciepn_dev_t * devpriv, unsigned long dspAddress)
{
  /* only counts  the 10 MSBs */
  dspAddress >>= 22;
  outl(dspAddress,devpriv->portio_base + PCI_IO_DSPP);
};


/**
 *
 * @brief Interrupt Handler
 *
 * This function is the interrupt handler for the .
 *
 * @param [in]  irq a variable of type int which specifies the
 *		interrrupt line which caused the interrupt.
 *
 * @param [in]  dib a pointer to the device information block of
 *		the device interrupting
 *
 * @return	IRQ_HANDLED if this interrupt was serviced,
 *		otherwise IRQ_NONE is returned.
 *
 * @note	The interrupt line for the PCI-1200 is shared.
 *
 *
 todo
*/

/* On  kernel versions < 2.6.20 use third  argument: , struct pt_regs *regs*/
static irqreturn_t c641x_interrupt(int irq, void *dev_id)
{
  pciepn_dev_t *devpriv = (pciepn_dev_t *)dev_id; /* device information */
  irqreturn_t ret = IRQ_NONE;
  u32 int_hsr, int_dsp=0;

  /* Code clear the hardware interrupts */
  int_hsr= inl(devpriv->portio_base + PCI_IO_HSR);
  if((int_hsr & HSR_INTSRC)!=0) {
    int_hsr &= HSR_INTAM_MSK;
    int_hsr |= HSR_INTSRC;
    /* reset INTSRC bit */
    outl(int_hsr,devpriv->portio_base + PCI_IO_HSR);
    devpriv->interrupts++;
    ret = IRQ_HANDLED;

    outl(DSP_IRAM_PAGE, devpriv->portio_base + PCI_IO_DSPP); /* set Page 0*/
    int_dsp =  ioread32(devpriv->memio_base + DSPSTATUS);    /* get IRQ reasons from card */
    iowrite32(0, devpriv->memio_base + DSPSTATUS);        /* clear IRQ reason */
    if ( int_dsp == INT_PCIDMA){
      devpriv->dma_actbuf = (devpriv->dma_actbuf)? 0 : 1; /* Togle DMA buffer */
      iowrite32(devpriv->dmabuf_hw[devpriv->dma_actbuf],
		devpriv->mmapreg_base + PCI_MMAP_PCIMA);
    }
    else if (int_dsp == INT_ENDACQ) {
      if (waitqueue_active(&devpriv->rd_waitq)){
	PDEBUG("waitqueue_active.. waking queue  ");
	wake_up_interruptible(&devpriv->rd_waitq);
      }
    }
    PDEBUG(" interrupt: 0x%x, reas:=x%x, HSR:=x%x \n", devpriv->interrupts,
	   int_dsp, inl(devpriv->portio_base + PCI_IO_HSR));
  }
  return IRQ_RETVAL(ret);
}

/**
 *
 * @brief Open Method.
 *
 * This function is called to initialize and obtain access to the
 * PCI1200 device functions.  The DMA buffers used for collecting AD
 * samples are allocated and mapped into I/O memory, unless the
 * WRITE-ONLY access mode is specified.
 *
 * Only one user may open the PCI1200 module for reading (O_RDONLY)
 * or reading and writing (O_RDWR) at any one time.  Multiple opens
 * may occur for write-only users (O_WRONLY), and write-only users are
 * allowed to coexist with an active read or read-write user.
 *
 * A user opens the module in read-only mode for the purpose of collecting
 * real-time AD samples from the board, and nothing more.  A read-write
 * user has access to the IOCTL commands to perform digital I/O, write to
 * the DACS, and access the timer/counter functions, as well as collecting
 * real-time AD samples.  Write-only users may perform all the functions
 * of a read-write user, with the exception of collecting AD samples.
 *
 * @note  The PCI1200 device is reset to a known state on an unopened module.
 */
static int open(struct inode *inode, struct file *filp) {

  pciepn_dev_t *devpriv; /* device information */
      //config_port_t cPort;
  int rc ,i;

  devpriv = container_of(inode->i_cdev,  pciepn_dev_t, cdev);
  if (down_interruptible(&devpriv->open_sem))
    return -ERESTARTSYS;

/*   if ( (filp->f_flags & O_ACCMODE) != O_WRONLY) { */
/*   /\* Check if Channel addresses are registred*\/ */
/*     if(devpriv->chanAddr[0]==0){ */
/*       memcpy_fromio(&cPort, devpriv->memio_base + DSPCONFIG, sizeof(config_port_t)); */
/*       memcpy(devpriv->chanAddr, cPort.channelAdd, NR_CHANNELS * sizeof(int)); */
/*     } */
/*   } */

  /* on first open, reset PCI regs and setup IRQ */
  if (!(devpriv->open_count)) {
    devpriv->dmagfporder=0;
    devpriv->dmabuf_size=PAGE_SIZE * (1 << devpriv->dmagfporder);
    for (i=0; i<2; i++) {
      if ((devpriv->dmabuf_virt[i]=__get_free_pages(GFP_KERNEL,devpriv->dmagfporder ))){

      }
      else{
		printk(KERN_ERR "c641x: In function open:"
			 "__get_free_pages() failed "
			 "attempting to allocate and map DMA Buffer\n");
		up(&devpriv->open_sem);
		return -ENOMEM;
      }
    }

    /* request IRQ */
    /*Physically disable the hardware interrupts */
    outl(5, devpriv->portio_base + PCI_IO_HSR);
    /* Disable Master DMA */
    iowrite32(0, devpriv->mmapreg_base + PCI_MMAP_PCIMC);
    rc = request_irq((unsigned int)devpriv->irq, c641x_interrupt,
		     IRQF_SHARED , "pci_epn", (void *)devpriv);
    if (rc) {
      up(&devpriv->open_sem);
      return rc;
    }
  }

  /* bump the open count */
  devpriv->open_count++;

  filp->private_data = devpriv; /* for other methods */
  up(&devpriv->open_sem);
  /* Success */
  return 0;
}

/**
 * called by close
 */
static int release(struct inode *inode, struct file *filp) {
  int i;
  pciepn_dev_t *devpriv = (pciepn_dev_t *)filp->private_data; /* device information */


  down(&devpriv->open_sem);
  devpriv->open_count--;

  /* reset and free IRQ on final release */
  if (!devpriv->open_count) {
    for (i=0; i<2; i++) {
      if (devpriv->dmabuf_virt[i]){
      free_pages(devpriv->dmabuf_virt[i], devpriv->dmagfporder); //, devpriv->dmagfporder);
      devpriv->dmabuf_virt[i] = 0;
      }
    }
    /*Physically disable the hardware interrupts */
    outl(5,devpriv->portio_base + PCI_IO_HSR);
    /* Disable Master DMA */
    iowrite32(0, devpriv->mmapreg_base + PCI_MMAP_PCIMC);
    udelay(1000); /* wait for DMA in progress to finish */
    free_irq((unsigned int)devpriv->irq, (void *)devpriv);
  }
  PDEBUG("Module release\n");
  filp->private_data = NULL;
  up(&devpriv->open_sem);
  /* Success */
  return 0;
}
/**
 *
 **/
static ssize_t read(struct file *filp, char *buf,
		    size_t count, loff_t *f_pos) {

        //pciepn_dev_t *devpriv = (pciepn_dev_t *)filp->private_data; /* device information */
  ssize_t retval = 0;
  //  ssize_t bytes = 0;
  //ssize_t bytes2read, read_offset, nbytes ;
  //  int read_buf;
  //unsigned int memF,  dspChanSz; //dspChBlks
  //status_port_t sPort;

    
  *f_pos += count;
  retval = count;
      //out:
  //  up(&dev->sem);
  return retval;
}

/**
 *
 **/
static ssize_t write( struct file *filp, const char __user *buf,
		      size_t count, loff_t *f_pos) {
  ssize_t retval = 0;
  int i;
  union {
    u_int32_t reg32;
    u_int16_t reg16[2];
    u_int8_t reg8[4];
  } u;

  pciepn_dev_t *devpriv = (pciepn_dev_t *)filp->private_data; /* device information */
  // PDEBUGG("write, fpos: 0x%x, data 0x%x\n", (int) *f_pos , cval );
  outl(DSP_IRAM_PAGE, devpriv->portio_base + PCI_IO_DSPP); // set Page 0
  for(i=0; i < count; i++) {
    if(__get_user(u.reg8[3], (u_int8_t *) buf++)){
      retval= -EFAULT;
      goto out;
    }
    iowrite8(u.reg8[3], devpriv->memio_base + *f_pos);
    (*f_pos)++;
  }
  retval = i;
 out:
  return retval;
}

int send_comand(pciepn_dev_t *devpriv,   unsigned int dsp_cmd)
{
  int  retval = 0;
  int tmp;
  
  iowrite32(dsp_cmd, devpriv->memio_base + HOST2DSP);
          
  outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
  udelay(TIME_OUT_DLY); // 10us NOT works .... must test
  if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) {
      PDEBUG("ioctl, Timeout: HOST2DSP: %#x\n", tmp );
      return -ETIMEDOUT;
  }
  udelay(TIME_OUT_DLY);
  if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != dsp_cmd) {
      PDEBUG("ioctl, Timeout: DSPSTATUS +4: %#x\n", tmp );
      return -ETIMEDOUT;
  }
  return retval;
}

/**
 * from ioctl
 *
 */
int ioctl(struct inode *inode, struct file *filp,
	  unsigned int cmd, unsigned long arg)
{
  int err = 0, retval = 0;
  int  tmp;//, cfgWrd;
  u64  var64;
  u32 * pInt;
 
      //ParamWordFields * pcfgWrd;
  status_port_t sPort;
  pciepn_dev_t *devpriv = (pciepn_dev_t *)filp->private_data; /* device information */
  //  config_port_t cPort;

  PDEBUG("c641x Module ioctl, deveno: 0x%x, cmd:0x%x\n", MINOR(devpriv->devno), cmd );

  /*
   * extract the type and number bitfields, and don't decode
   * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
   */
  if (_IOC_TYPE(cmd) != PCI_EPN_IOC_MAGIC) return -ENOTTY;
  if (_IOC_NR(cmd) > PCI_EPN_IOC_MAXNR) return -ENOTTY;

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

  outl(DSP_IRAM_PAGE, devpriv->portio_base + PCI_IO_DSPP); // set Page 0

  switch(cmd) {
      
      case PCI_EPN_IOCT_FPGA_INIT:
          iowrite32(INIT, devpriv->memio_base + HOST2DSP);
          outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
          udelay(TIME_OUT_DLY); 
          if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) 
              return -ETIMEDOUT;
          udelay(TIME_OUT_DLY);
          if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != INIT) {
              return -ETIMEDOUT;
          }
          break;
          
      case PCI_EPN_IOCS_SEND_EVENT:
          if(copy_from_user(&var64, (void __user *)arg, sizeof(u64 )))
            return -EFAULT;

          pInt = (u32 *) &var64;
          iowrite32(pInt[0], devpriv->memio_base + HOST2DSP +4);
          iowrite32(pInt[1], devpriv->memio_base + HOST2DSP +8);
          iowrite32(SEND_EVENT, devpriv->memio_base + HOST2DSP);
          
          outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
          udelay(TIME_OUT_DLY); // 10us NOT works .... must test
          if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) {
              PDEBUG("ioctl, Timeout: HOST2DSP: %#x\n", tmp );
              return -ETIMEDOUT;
          }
          udelay(TIME_OUT_DLY);
          if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != SEND_EVENT) {
              PDEBUG("ioctl, Timeout: DSPSTATUS +4: %#x\n", tmp );
              return -ETIMEDOUT;
              
          }
          break;
      case PCI_EPN_IOCT_READ_EVENT:
          iowrite32(READ_EVENT, devpriv->memio_base + HOST2DSP);
          outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
          udelay(TIME_OUT_DLY); 
          if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) 
              return -ETIMEDOUT;
          udelay(TIME_OUT_DLY);
          if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != READ_EVENT) {
              return -ETIMEDOUT;
          }
          break;
      case PCI_EPN_IOCT_RESET_CNT:
          iowrite32(RESET_CNT, devpriv->memio_base + HOST2DSP);
          outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
          udelay(TIME_OUT_DLY); 
          if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) 
              return -ETIMEDOUT;
          udelay(TIME_OUT_DLY);
          if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != RESET_CNT) {
              return -ETIMEDOUT;
          }
          break;
      case PCI_EPN_IOCS_SET_TRG:
          if(copy_from_user(&var64, (void __user *)arg, sizeof(u64 )))
            return -EFAULT;

          iowrite32(devpriv->trigChan, devpriv->memio_base + HOST2DSP +4);
          pInt = (u32 *) &var64;

          iowrite32(pInt[0], devpriv->memio_base + HOST2DSP +8);
          PDEBUG("ioctl, SET_TRG0: %#x\n", pInt[0] );
          PDEBUG("ioctl, SET_TRG1: %#x\n", pInt[1] ); // dataHigh
          iowrite32(pInt[1], devpriv->memio_base + HOST2DSP +12);
          if(send_comand(devpriv, SET_TRIG)){
              return -ETIMEDOUT;
          }
          break;
      case PCI_EPN_IOCS_SET_WIDTH:
          retval = __get_user(tmp, (int __user *)arg);
          if (retval)
            return -EFAULT;

          iowrite32(devpriv->trigChan, devpriv->memio_base + HOST2DSP +4);
//          pInt = (u32 *) &var64;
//          PDEBUG("ioctl, SET_W0: %#x\n", pInt[0] );
//          PDEBUG("ioctl, SET_W1: %#x\n", pInt[1] );

          iowrite32(tmp, devpriv->memio_base + HOST2DSP +8);
          iowrite32(0, devpriv->memio_base + HOST2DSP +12);
          iowrite32(SET_WIDTH, devpriv->memio_base + HOST2DSP);
          
          outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
          udelay(TIME_OUT_DLY); // 10us NOT works .... must test
          if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) {
              PDEBUG("ioctl, Timeout: HOST2DSP: %#x\n", tmp );
              return -ETIMEDOUT;
          }
          udelay(TIME_OUT_DLY);
          if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != SET_WIDTH) {
              PDEBUG("ioctl, Timeout: DSPSTATUS +4: %#x\n", tmp );
              return -ETIMEDOUT;
              
          }
          break;
      case PCI_EPN_IOCS_SET_TIME:
          if(copy_from_user(&var64, (void __user *)arg, sizeof(u64 )))
            return -EFAULT;

          pInt = (u32 *) &var64;
          PDEBUG("ioctl, SET_TIME0: %#x\n", pInt[0] );
          PDEBUG("ioctl, SET_TIME1: %#x\n", pInt[1] ); // dataHigh

          iowrite32(pInt[0], devpriv->memio_base + HOST2DSP +8);
          iowrite32(pInt[1], devpriv->memio_base + HOST2DSP +12);
          
          if(send_comand(devpriv, SET_TIME)){
              return -ETIMEDOUT;
          }
          
          break;
      case PCI_EPN_IOCS_SET_TRG_CHAN:
          retval = __get_user(tmp, (int __user *)arg);
          if (!retval)
              devpriv->trigChan = tmp ; 
          break;
      case PCI_EPN_IOCT_INT_ENABLE:
              //Physically enable the hardware interrupts
          outl(5,devpriv->portio_base + PCI_IO_HSR); // reset INTSRC bit
          outl(1,devpriv->portio_base + PCI_IO_HSR);
          break;
      case PCI_EPN_IOCT_INT_DISABLE:
              //Physically disable the hardware interrupts
          outl(5,devpriv->portio_base + PCI_IO_HSR); // reset INTSRC bit
          break;
          
      case PCI_EPN_IOCT_PING:
          iowrite32(PING, devpriv->memio_base + HOST2DSP);
          outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
          udelay(TIME_OUT_DLY); // 10us NOT works .... must test
          if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) 
              return -ETIMEDOUT;
          udelay(TIME_OUT_DLY);
          if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != PING) {
              return -ETIMEDOUT;
          }
          break;

      case  PCI_EPN_IOCS_TMOUT:
          retval = __get_user(tmp, (int __user *)arg);
          if (!retval)
              devpriv->wt_tmeout = tmp * HZ; /*time out in sec*/
          break;
      case PCI_EPN_IOCT_DSP_HOLD:
              /*send WARM reset  to dsp... */
    outl(HDCR_WARMRESET_BIT, devpriv->portio_base + PCI_IO_HDCR);
        /*Must wait at least 16 DSP clock cycles before accessing the DSP again */
    udelay(1000);
    break;
      case PCI_EPN_IOCT_DSP_RESET:
          outl(HDCR_WARMRESET_BIT, devpriv->portio_base + PCI_IO_HDCR);
          udelay(1000);
      case PCI_EPN_IOCT_DSP_FREE:
              /* Release the DSP from reset */
          outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR);
              //    msleep(100);
          
          break;
  case PCI_EPN_IOCG_STATUS:
//    PDEBUG("ioctl firmWR=0x%x\n", ioread32(devpriv->memio_base+ DSPCONFIG));
      
    memcpy_fromio(&sPort, devpriv->memio_base + DSPSTATUS, sizeof(status_port_t));
    if(copy_to_user((void __user *)arg, &sPort, sizeof(status_port_t)))
      return -EFAULT;
    break;

/*   case PCI_EPN_IOCG_IRQ_CNT: /\* write Get: arg is pointer to result *\/ */
/*     retval = __put_user(devpriv->interrupts, (int __user *)arg); */
/*     PDEBUGG("ioctl irq cnt retval:%d,\n", retval); */
    
/*     break; */

  default:  /* redundant, as cmd was checked against MAXNR */
    return -ENOTTY;
  }
  PDEBUG("ioctl HSR:0x%x, HDCR:0x%x, DSPP:0x%x\n",
	 inl(devpriv->portio_base + PCI_IO_HSR),
	 inl(devpriv->portio_base + PCI_IO_HDCR),
	 inl(devpriv->portio_base + PCI_IO_DSPP));

  return retval;

}
/**
 * pci_epn file operations.
 *
 */
static struct file_operations pci_epn_fops = {
  owner: THIS_MODULE,
  read:  read,
  write: write,
  ioctl: ioctl,
  open:  open,
  release: release
};



/**
 *
 */
static int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    int rc, i;
  //	void 	*base_phys;  		//access to 4M-byte prefetchable maps to all of DSP memory
  void * mmapreg_base = NULL; // access to DSP’s memory-mapped registers
  void * memio_base = NULL;  		//access to 4M-byte prefetchable maps to all of DSP memory
  unsigned long memio_base_phys, base_phys, flags;
  unsigned short 	portio_base;
  config_port_t cPort;
  u8 irq, brd_rev;

  pciepn_dev_t * devpriv = NULL;


  /* Do probing type stuff here.
   * Like calling request_region();
   */
  /* request the memory regions */
  rc = pci_request_regions(pdev, pci_driver.name);
  if (rc) {
    printk(KERN_ERR "pci_epn: In function probe:"
	   "pci_request_regions() reports PCI Regions unavailable\n");
    return rc;
  }

  memio_base_phys = pci_resource_start(pdev, C641X_MEM_BAR0_INDEX);
  if (!memio_base_phys) {
    printk(KERN_ERR "c641x: In function probe:"
	   "pci_resource_start() failure - PCI Region %d unavailable\n", C641X_MEM_BAR0_INDEX);
    rc = -EIO;
    goto out;
  }
  flags=pci_resource_flags(pdev, C641X_MEM_BAR0_INDEX);
  PDEBUG("c641x:probe %d region flags 0x%x\n", C641X_MEM_BAR0_INDEX,(int) flags);

  memio_base = ioremap_nocache(memio_base_phys, pci_resource_len(pdev, C641X_MEM_BAR0_INDEX));
  if (!memio_base) {
    printk(KERN_ERR "In function pcie_fpgaendpoint.probe:"
	   "ioremap() failed to map PCI Region %d\n", C641X_MEM_BAR0_INDEX);
    rc = -ENOMEM;
    goto out;
  }

  base_phys = pci_resource_start(pdev, C641X_MEM_BAR1_INDEX);
  if (!memio_base_phys) {
    printk(KERN_ERR "c641x: In function probe:"
	   "pci_resource_start() failure - PCI Region: %d unavailable\n", C641X_MEM_BAR1_INDEX);
    rc = -EIO;
    goto out;
  }
  mmapreg_base = ioremap_nocache(base_phys, pci_resource_len(pdev, C641X_MEM_BAR1_INDEX));

  if (!mmapreg_base) {
    printk(KERN_ERR "In function pcie_fpgaendpoint.probe:"
	   "ioremap() failed to map PCI Region: %d\n", C641X_MEM_BAR1_INDEX);
    rc = -ENOMEM;
    goto out;
  }
  flags=pci_resource_flags(pdev, C641X_MEM_BAR1_INDEX);
  PDEBUG("c641x:probe %d region flags 0x%x\n", C641X_MEM_BAR1_INDEX,(int) flags);

  portio_base = pci_resource_start(pdev, C641X_IO_BARINDEX);
  if (!portio_base) {
    printk(KERN_ERR "c641x: In function probe:"
	   "pci_resource_start() failure - PCI Region %d unavailable\n", C641X_IO_BARINDEX);
    rc = -EIO;
    goto out;
  }

  flags=pci_resource_flags(pdev, C641X_IO_BARINDEX);
  PDEBUG("probe  region %d flags 0x%x\n", C641X_IO_BARINDEX, (int) flags);

  if (pci_enable_device(pdev)){
    rc = -ENODEV;
    goto out;
  }

  rc=pci_read_config_byte(pdev, PCI_REVISION_ID, &brd_rev);
  if (rc) {
    printk(KERN_ERR ":  probe:"
	   "error pci_read_config_byte(PCI_REVISION_ID)\n");
    rc = -EIO;
    goto out;
  }
  /**
   * not used
  rc=pci_read_config_byte(pdev, PCI_INTERRUPT_LINE, &irq);
  if (rc) {
    printk(KERN_ERR ":  probe:"
	   " error pci_read_config_byte(PCI_INTERRUPT_LINE)\n");
    rc = -EIO;
    goto out;
  }
  */
  /*Only works after pci_enable_device()*/
  irq= pdev->irq;
  PDEBUG("pciepn probe: Revision:%d, irq:%d\n", (int)brd_rev,(int) irq);

  outl(FLASH_PAGE, portio_base + PCI_IO_DSPP); // set Page
//  PDEBUG(" Magic: 0x%x, Count: 0x%x, Number: %d \n", ioread32(memio_base),
//	 ioread32(memio_base + 4), ioread32(memio_base + 8) );

  outl(DSP_IRAM_PAGE, portio_base + PCI_IO_DSPP); // set Page 0
  i=inl(portio_base + PCI_IO_DSPP); 
  mb();
  PDEBUG("probe HSR:0x%x, HDCR:0x%x, DSPP:0x%x\n",
         inl(portio_base + PCI_IO_HSR),
         inl(portio_base + PCI_IO_HDCR),
         inl(portio_base + PCI_IO_DSPP));
  
  ioread32(memio_base);
  ioread32(memio_base+ DSPCONFIG);
  
  memcpy_fromio(&cPort, memio_base + DSPCONFIG, sizeof(config_port_t));
  if(cPort.fwLoaded!=0x42424330){
    PDEBUG("pci_epn Board FirmW NOT loaded: 0x%0X\n", cPort.fwLoaded );
/*     for(i=0; i < NR_CHANNELS ; i++) */
/*       cPort.channelAdd[i] =  0; // for now */
  }

  /* allocate the device instance block */
  devpriv = kzalloc(sizeof(pciepn_dev_t), GFP_KERNEL);
  if (!devpriv) {
    rc = -ENOMEM;
    goto out;  /* Make this more graceful */
  }
  /*Check if have already major number*/
  if(major){ 
      devpriv->devno=MKDEV(major, minor);
      register_chrdev_region(devpriv->devno, 1, pci_driver.name);
  }
  else{
          /* allocate  the device  number*/
      rc = alloc_chrdev_region(&devpriv->devno, minor, 1, pci_driver.name);
      major=MAJOR(devpriv->devno);
  }
  
  /* allocate  the device  number*/
//  rc = alloc_chrdev_region(& devpriv->devno, minor, 1, pci_driver.name);
  if (rc) {
    printk(KERN_ERR "pci_epn:In function probe:"
	   "alloc_chrdev_region() failure - "
	   "unable to allocate char devices\n");
    goto out;
  }

  /* Initialize device. */
    devpriv->memio_base_phys = memio_base_phys;
    devpriv->memio_base = memio_base;
    devpriv->mmapreg_base = mmapreg_base;
    devpriv->portio_base = portio_base;
    devpriv->trigChan = 0;
//    memcpy(cPort.channelAdd, devpriv->chanAddr, NR_CHANNELS * sizeof(int));
        //devpriv->act_chan = 0;
    devpriv->irq= irq;

    
    devpriv->pdev = pdev;
    init_MUTEX(&devpriv->open_sem); 
/*     init_waitqueue_head(&devpriv->rd_waitq); */
    cdev_init(&devpriv->cdev, &pci_epn_fops);
    devpriv->cdev.owner = THIS_MODULE;
    devpriv->cdev.ops = &pci_epn_fops;
    rc = cdev_add (&devpriv->cdev, devpriv->devno, 1);

  /* clean up if error */
 out:if (rc) {
    if (memio_base)
      iounmap(memio_base);
    if (mmapreg_base)
      iounmap(mmapreg_base);
    pci_release_regions(pdev);
    if (devpriv->devno)
      	unregister_chrdev_region(devpriv->devno, 1);
    if (devpriv)
	 	kfree(devpriv);
    pci_disable_device(devpriv->pdev);
  }
    else {
        printk(KERN_NOTICE "pcie_probe cdev_add OK, major: %d, minor:%d, IRQ:%d \n",
               MAJOR(devpriv->devno), minor, devpriv->irq);
        minor+=1;
        pci_set_drvdata(pdev, devpriv);
    }
  return rc;
}

/**
 *
 */
static void remove(struct pci_dev *pdev)
{
  /* clean up any allocated resources and stuff here.
   * like call release_region();
   */
  pciepn_dev_t * devpriv;

  /* get the device information data */
  devpriv = (pciepn_dev_t *) pci_get_drvdata(pdev);
  PDEBUG("c641x Module  Remove 0x%x\n", devpriv->devno );

  iounmap(devpriv->memio_base);
  iounmap(devpriv->mmapreg_base);
  cdev_del(&devpriv->cdev);
  unregister_chrdev_region(devpriv->devno, 1);
  kfree(devpriv);
  pci_release_regions(pdev);
  pci_set_drvdata(pdev, (void *)NULL);
  pci_disable_device(pdev);

  return;
}

/**
 *
 */
static void __exit c641x_cleanup(void)
{
  pci_unregister_driver(&pci_driver);
  PDEBUG("Module c641x exit\n");
}

/**
 *
 */
static int __init c641x_init(void)
{
  int result;

  result= pci_register_driver(&pci_driver);
  if(result){
    goto fail;
  }
  printk (KERN_NOTICE "pci_c641x_init OK!\n");

  return 0; /* succeed */

 fail:
  c641x_cleanup();
  return result;
}


module_init(c641x_init);
module_exit(c641x_cleanup);

// PDEBUG("Here I am: %s:%i\n", __FILE__, __LINE__);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Driver for PCI pci_epn");
MODULE_AUTHOR("Bernardo Carvalho");
