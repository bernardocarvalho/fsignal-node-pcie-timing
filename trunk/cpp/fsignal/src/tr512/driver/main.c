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

#include <cfn_c641x.h> /* local definitions */
#include <cfn_c641x_lib.h>
#include <cfn_c641x_ioctl.h>

/*
 * Module Parameters
 */
//module_param(scull_nr_devs, int, S_IRUGO);
static int minor =  0;
module_param(minor, int, S_IRUGO);

#define TR512_DEVICE_ID 0xA106 /* Device ID */

#define SIGNAL_HSIZE 256 /*1 kB*/
static struct pci_device_id ids[] = { //0x10ee
  { PCI_DEVICE(PCI_VENDOR_ID_TI, TR512_DEVICE_ID), },
  { 0, }
};
MODULE_DEVICE_TABLE(pci, ids);


/*
 * Prototypes for local functions
 */
static int probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void remove(struct pci_dev *pdev);

static struct pci_driver pci_driver = {
  .name = "cfn_c641x_drv",
  .id_table = ids,
  .probe = probe,
  .remove = remove,
};

/*** Static functions ***/
/***
    set DSP Page Register
***/
static inline void set_dsp_page(c641x_dev_t * devpriv, unsigned long dspAddress)
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
  c641x_dev_t *devpriv = (c641x_dev_t *)dev_id; /* device information */
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

  c641x_dev_t *devpriv; /* device information */
  config_port_t cPort;
  int rc ,i;

  devpriv = container_of(inode->i_cdev,  c641x_dev_t, cdev);
  if (down_interruptible(&devpriv->open_sem))
    return -ERESTARTSYS;

  if ( (filp->f_flags & O_ACCMODE) != O_WRONLY) {
  /* Check if Channel addresses are registred*/
    if(devpriv->chanAddr[0]==0){
      memcpy_fromio(&cPort, devpriv->memio_base + DSPCONFIG, sizeof(config_port_t));
      memcpy(devpriv->chanAddr, cPort.channelAdd, NR_CHANNELS * sizeof(int));
    }
  }

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
		     SA_INTERRUPT | SA_SHIRQ , "cfn_c641x", (void *)devpriv);
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
  c641x_dev_t *devpriv = (c641x_dev_t *)filp->private_data; /* device information */


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

  c641x_dev_t *devpriv = (c641x_dev_t *)filp->private_data; /* device information */
  ssize_t retval = 0;
  //  ssize_t bytes = 0;
  ssize_t bytes2read, read_offset, nbytes ;
  //  int read_buf;
  unsigned int memF,  dspChanSz; //dspChBlks
  status_port_t sPort;

  PDEBUGG("Module read deveno: 0x%x, %d, f_pos: %d \n", MINOR(devpriv->devno),count,(int) *f_pos );

  /*read after ext  trigger*/
  if (!(filp->f_flags & O_NONBLOCK )) {
    /*Blocking Read*/
    PDEBUG("read : Waiting..., cnt: %d\n", count);
    if(wait_event_interruptible_timeout(devpriv->rd_waitq,
	(memF=ioread32(devpriv->memio_base + DSPSTATUS + 12)) & 0x2, devpriv->wt_tmeout)==0){
      /*Clear status_port.MemoryFlags.AcqStop*/
      outl(DSP_IRAM_PAGE, devpriv->portio_base + PCI_IO_DSPP); // set Page 0
      iowrite32(memF & 0xFFFFFFFD, devpriv->memio_base + DSPSTATUS + 12);
      printk (KERN_ALERT "c641x: (read) timeout\n");
      return -ETIMEDOUT;
    }
  }
  outl(DSP_IRAM_PAGE, devpriv->portio_base + PCI_IO_DSPP); // set Page 0
  count = min_t(int, count, C641X_BAR0_PAGE_SIZE); 
  /*read dspstatus to get trigger block, block offset and circular buffer size*/
  memcpy_fromio(&sPort, devpriv->memio_base + DSPSTATUS, sizeof(status_port_t));
  /* validate the user's buffer address range */
  if (!(access_ok(VERIFY_WRITE, buf, count)))
    return -EFAULT;
  set_dsp_page(devpriv, devpriv->chanAddr[devpriv->act_chan]);

  bytes2read = count;
  //dspChBlks =  sPort.configWrd.circBuffSize;
  /*get trigger position to pos-trigger data*/
  //  read_offset = (sPort.triggerBlock  + (dspChBlks >>1)) %dspChBlks;
  read_offset =  sPort.triggerBlock * EDMA_BLK_SIZE ; /*sz in bytes*/
  read_offset += sPort.triggerOffset * sizeof(short);
  dspChanSz = sPort.configWrd.circBuffSize * EDMA_BLK_SIZE ;
  while(bytes2read > 0){
    nbytes = (bytes2read > devpriv->dmabuf_size)? devpriv->dmabuf_size : bytes2read;
    /*Check if buffer limit is reached*/
    nbytes = (nbytes < (dspChanSz - read_offset))? nbytes : (dspChanSz - read_offset);

    memcpy_fromio((void *)devpriv->dmabuf_virt[0], devpriv->memio_base + read_offset, nbytes);
    /* Transfering data to user space */
    if (copy_to_user(buf, (void *) devpriv->dmabuf_virt[0], nbytes)) {
      retval = -EFAULT;
      goto out;
    }
    read_offset = (read_offset + nbytes )%dspChanSz; /*modulo*/
    buf += nbytes;
    bytes2read -=nbytes;
  }
  PDEBUG("read : bytes:%d, chan:%d,  from:0x%x\n", count, devpriv->act_chan, 
	 (unsigned int)devpriv->chanAddr[devpriv->act_chan]);
    
  *f_pos += count;
  retval = count;
 out:
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

  c641x_dev_t *devpriv = (c641x_dev_t *)filp->private_data; /* device information */
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
/**
 * from scull_ioctl
 *
 */
int ioctl(struct inode *inode, struct file *filp,
	  unsigned int cmd, unsigned long arg)
{

  int err = 0, retval = 0;
  int i, tmp, cfgWrd;
  ParamWordFields * pcfgWrd;
  status_port_t sPort;
  c641x_dev_t *devpriv = (c641x_dev_t *)filp->private_data; /* device information */
  //  config_port_t cPort;

  PDEBUGG("c641x Module ioctl, deveno: 0x%x, cmd:0x%x\n", MINOR(devpriv->devno), cmd );

  /*
   * extract the type and number bitfields, and don't decode
   * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
   */
  if (_IOC_TYPE(cmd) != CFN_C641X_IOC_MAGIC) return -ENOTTY;
  if (_IOC_NR(cmd) > CFN_C641X_IOC_MAXNR) return -ENOTTY;

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
  case CFN_C641X_IOCS_CONFIG_ACQ:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval){
      cfgWrd = tmp;
      pcfgWrd = (ParamWordFields *) &cfgWrd; ;

      iowrite32(cfgWrd, devpriv->memio_base + HOST2DSP +4);
      iowrite32(CONFIG_ACQ, devpriv->memio_base + HOST2DSP);
      PDEBUG("ioctl, tmp: 0x%x, pcfgWrd->streamAcq:%d\n", tmp, pcfgWrd->streamAcq );
      //      printk("cfn_c641x_ai_cinsn called: uval = 0x%0X\n", uval);

      outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
      udelay(TIME_OUT_DLY); // 10us NOT works .... must test
      if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) {
	PDEBUG("ioctl, Timeout: HOST2DSP: %#x\n", tmp );
	return -ETIMEDOUT;
      }
      udelay(TIME_OUT_DLY);
      if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != CONFIG_ACQ) {
	PDEBUG("ioctl, Timeout: DSPSTATUS +4: %#x\n", tmp );
	return -ETIMEDOUT;
      }
    }
    break;
  case CFN_C641X_IOCT_INT_ENABLE:
    //Physically enable the hardware interrupts
    outl(5,devpriv->portio_base + PCI_IO_HSR); // reset INTSRC bit
    outl(1,devpriv->portio_base + PCI_IO_HSR);
    break;
  case CFN_C641X_IOCT_INT_DISABLE:
    //Physically disable the hardware interrupts
    outl(5,devpriv->portio_base + PCI_IO_HSR); // reset INTSRC bit
    break;
  case CFN_C641X_IOCT_SOFT_TRIG:
      iowrite32(SOFT_TRIG, devpriv->memio_base + HOST2DSP);
      outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR); // Generates a host interrupt to the DSP.
      udelay(TIME_OUT_DLY); // 10us NOT works .... must test
      if((tmp=ioread32(devpriv->memio_base + HOST2DSP)) != NOTHING) {
	return -ETIMEDOUT;
      }
      udelay(TIME_OUT_DLY);
      if((tmp=ioread32(devpriv->memio_base + DSPSTATUS +4)) != SOFT_TRIG) {
	//PDEBUG("ioctl, Timeout: DSPSTATUS +4: %#x\n", tmp );
	return -ETIMEDOUT;
      }
    break;
  case CFN_C641X_IOCT_DMA_ENABLE:
    //      if((pcfgWrd->streamAcq) !=0){
    for (i=0; i<2; i++) {
      devpriv->dmabuf_hw[i]=dma_map_single(&devpriv->pdev->dev, (void *)devpriv->dmabuf_virt[i],
					   devpriv->dmabuf_size, DMA_FROM_DEVICE);
    }
/* 	PDEBUG("ioctl: CDSPA: 0x%X, CPCIA: 0x%X, CCNT: 0x%X\n", */
/* 	       ioread32(devpriv->mmapreg_base + PCI_MMAP_CDSPA), */
/* 	       ioread32(devpriv->mmapreg_base + PCI_MMAP_CPCIA), */
/* 	       ioread32(devpriv->mmapreg_base + PCI_MMAP_CCNT) */
/* 	       ); */
	devpriv->dma_actbuf = 0; // Start on buffer 0
    iowrite32(devpriv->dmabuf_hw[devpriv->dma_actbuf], devpriv->mmapreg_base + PCI_MMAP_PCIMA);
    iowrite32((devpriv->dmabuf_size  <<16) , devpriv->mmapreg_base + PCI_MMAP_PCIMC); // DMA bytes CNT
	PDEBUGG("ioctl, streamAcq DSPMA: %#x, PCIMA  addr: %#x, CNT:%#x\n",
	       ioread32(devpriv->mmapreg_base + PCI_MMAP_DSPMA),
	       ioread32(devpriv->mmapreg_base + PCI_MMAP_PCIMA),
	       ioread32(devpriv->mmapreg_base + PCI_MMAP_PCIMC));
	PDEBUGG("ioctl: CDSPA: 0x%X, CPCIA: 0x%X, CCNT: 0x%X\n",
	       ioread32(devpriv->mmapreg_base + PCI_MMAP_CDSPA),
	       ioread32(devpriv->mmapreg_base + PCI_MMAP_CPCIA),
	       ioread32(devpriv->mmapreg_base + PCI_MMAP_CCNT)
	       );
	//}

    break;
  case CFN_C641X_IOCT_DMA_DISABLE :

    iowrite32(0, devpriv->mmapreg_base + PCI_MMAP_PCIMC); // DMA bytes CNT = 0
    udelay(1000); // wait for DMA in progress to finish
    for (i=0; i<2; i++) {
      dma_unmap_single(&devpriv->pdev->dev, devpriv->dmabuf_hw[i],
		       devpriv->dmabuf_size, DMA_FROM_DEVICE);
    }
    break;

  case  CFN_C641X_IOCS_TMOUT:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval)
      devpriv->wt_tmeout = tmp * HZ; /*time out in sec*/
    break;
  case  CFN_C641X_IOCS_CHAN:
    retval = __get_user(tmp, (int __user *)arg);
    if (!retval)
      if(tmp <0 ) tmp =0;
      if(tmp >= NR_CHANNELS ) tmp =NR_CHANNELS -1;
      devpriv->act_chan = tmp ; /* actual channel to read*/
    break;
  case CFN_C641X_IOCT_DSP_HOLD:
    /*send WARM reset  to dsp... */
    outl(HDCR_WARMRESET_BIT, devpriv->portio_base + PCI_IO_HDCR);
    /*Must wait at least 16 DSP clock cycles before accessing the DSP again */
    udelay(1000);
    break;
  case CFN_C641X_IOCT_DSP_RESET:
    outl(HDCR_WARMRESET_BIT, devpriv->portio_base + PCI_IO_HDCR);
    udelay(1000);
  case CFN_C641X_IOCT_DSP_FREE:
    /* Release the DSP from reset */
    outl(HDCR_DSPINT_BIT,devpriv->portio_base + PCI_IO_HDCR);
    //    msleep(100);

    break;
  case CFN_C641X_IOCG_STATUS:
    PDEBUG("ioctl firmWR=0x%x\n", ioread32(devpriv->memio_base+ DSPCONFIG));

    memcpy_fromio(&sPort, devpriv->memio_base + DSPSTATUS, sizeof(status_port_t));
    if(copy_to_user((void __user *)arg, &sPort, sizeof(status_port_t)))
      return -EFAULT;
    break;

  case CFN_C641X_IOCG_IRQ_CNT: /* write Get: arg is pointer to result */
    retval = __put_user(devpriv->interrupts, (int __user *)arg);
    PDEBUGG("ioctl irq cnt retval:%d,\n", retval);

    break;

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
 * cfn_c641x file operations.
 *
 */
static struct file_operations cfn_c641x_fops = {
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

  c641x_dev_t * devpriv = NULL;


  /* Do probing type stuff here.
   * Like calling request_region();
   */
  /* request the memory regions */
  rc = pci_request_regions(pdev, pci_driver.name);
  if (rc) {
    printk(KERN_ERR "cfn_c641x: In function probe:"
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
  PDEBUG("c641x:probe %d region flags 0x%x\n", C641X_IO_BARINDEX, (int) flags);

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
  /** not used
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
  PDEBUG("c641x probe: Revision:%d, irq:%d\n", (int)brd_rev,(int) irq);

  outl(FLASH_PAGE, portio_base + PCI_IO_DSPP); // set Page
  PDEBUG(" Magic: 0x%x, Count: 0x%x, Number: %d \n", ioread32(memio_base),
	 ioread32(memio_base + 4), ioread32(memio_base + 8) );

  outl(DSP_IRAM_PAGE, portio_base + PCI_IO_DSPP); // set Page 0

  memcpy_fromio(&cPort, memio_base + DSPCONFIG, sizeof(config_port_t));
  if(cPort.fwLoaded!=0x42424330){
    PDEBUG("cfn_c641x Board FirmW NOT loaded: 0x%0X\n", cPort.fwLoaded );
    for(i=0; i < NR_CHANNELS ; i++)
      cPort.channelAdd[i] =  0; // for now
  }

  /* allocate the device instance block */
  devpriv = kzalloc(sizeof(c641x_dev_t), GFP_KERNEL);
  if (!devpriv) {
    rc = -ENOMEM;
    goto out;  /* Make this more graceful */
  }

  /* allocate  the device  number*/
  rc = alloc_chrdev_region(& devpriv->devno, minor, 1, pci_driver.name);
  if (rc) {
    printk(KERN_ERR "cfn_c641x:In function probe:"
	   "alloc_chrdev_region() failure - "
	   "unable to allocate char devices\n");
    goto out;
  }

  /* Initialize device. */
    devpriv->memio_base_phys = memio_base_phys;
    devpriv->memio_base = memio_base;
    devpriv->mmapreg_base = mmapreg_base;
    devpriv->portio_base = portio_base;
    memcpy(cPort.channelAdd, devpriv->chanAddr, NR_CHANNELS * sizeof(int));
    devpriv->act_chan = 0;
    devpriv->irq= irq;

    devpriv->pdev = pdev;
    init_MUTEX(&devpriv->open_sem);
    init_waitqueue_head(&devpriv->rd_waitq);
    cdev_init(&devpriv->cdev, &cfn_c641x_fops);
    devpriv->cdev.owner = THIS_MODULE;
    devpriv->cdev.ops = &cfn_c641x_fops;
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
  } else {
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
  c641x_dev_t * devpriv;

  /* get the device information data */
  devpriv = (c641x_dev_t *) pci_get_drvdata(pdev);
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
MODULE_DESCRIPTION("Driver for PCI cfn_c641x");
MODULE_AUTHOR("Bernardo Carvalho");
