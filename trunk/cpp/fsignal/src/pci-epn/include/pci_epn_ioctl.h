#ifndef _PCI_EPN_IOCTL_H_
#define _PCI_EPN_IOCTL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IOCTL definitions
 */
#define PCI_EPN_IOC_MAGIC 'l'  // /* Please use a different 8-bit number in your code */
/*See  /Documentation/ioctl-number.txt*/

/**
 * S means "Set": thru a pointer
 * T means "Tell": directly with the argument value
 * G menas "Get": reply by setting thru a pointer
 * Q means "Qry": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */

/** ********************************************************************
 *                         IOCTL FUNCTIONS                            *
 *********************************************************************/
#define PCI_EPN_IOCT_FPGA_INIT   _IO(PCI_EPN_IOC_MAGIC, 1)
#define PCI_EPN_IOCS_CONFIG_ACQ  _IOW(PCI_EPN_IOC_MAGIC, 2, u_int32_t)
#define PCI_EPN_IOCT_INT_ENABLE  _IO(PCI_EPN_IOC_MAGIC, 3)
#define PCI_EPN_IOCT_INT_DISABLE _IO(PCI_EPN_IOC_MAGIC, 4)
#define PCI_EPN_IOCS_SEND_EVENT  _IOW(PCI_EPN_IOC_MAGIC, 5, dual64_t) // todo
#define PCI_EPN_IOCT_READ_EVENT  _IO(PCI_EPN_IOC_MAGIC, 6)
#define PCI_EPN_IOCT_RESET_CNT   _IO(PCI_EPN_IOC_MAGIC, 7)
#define PCI_EPN_IOCS_SET_TRG     _IOW(PCI_EPN_IOC_MAGIC, 8, dual64_t) 
#define PCI_EPN_IOCS_SET_WIDTH   _IOW(PCI_EPN_IOC_MAGIC, 9, u_int32_t)
//#define PCI_EPN_IOCS_SET_WIDTH   _IOW(PCI_EPN_IOC_MAGIC, 9, dual64_t)

#define PCI_EPN_IOCT_PING         _IO(PCI_EPN_IOC_MAGIC, 10)


//#define PCI_EPN_IOCT_DMA_ENABLE _IO(PCI_EPN_IOC_MAGIC, 9)
//#define PCI_EPN_IOCT_DMA_DISABLE _IO(PCI_EPN_IOC_MAGIC, 10)
#define PCI_EPN_IOCS_TMOUT         _IOW(PCI_EPN_IOC_MAGIC, 11, u_int32_t)
#define PCI_EPN_IOCS_SET_TRG_CHAN  _IOW(PCI_EPN_IOC_MAGIC, 12, u_int32_t)
#define PCI_EPN_IOCS_SET_TIME     _IOW(PCI_EPN_IOC_MAGIC, 13, dual64_t) 


#define PCI_EPN_IOCT_DSP_HOLD    _IO(PCI_EPN_IOC_MAGIC, 20)
#define PCI_EPN_IOCT_DSP_FREE    _IO(PCI_EPN_IOC_MAGIC, 21)
#define PCI_EPN_IOCT_DSP_RESET    _IO(PCI_EPN_IOC_MAGIC, 22)
#define PCI_EPN_IOCG_STATUS      _IOR(PCI_EPN_IOC_MAGIC, 23, status_port_t)

#define PCI_EPN_IOC_MAXNR   23

//#define PCI_EPN_IOC_RESET _IO(PCI_EPN_IOC_MAGIC, 0)

//#define PCI_EPN_IOCG_BRD_INFO  _IOR(PCI_EPN_IOC_MAGIC, 1, BOARD_INFO)

//#define PCI_EPN_IOCS_CR1 _IOW(PCI_EPN_IOC_MAGIC, 2, u_int8_t)
//#define PCI_EPN_IOCG_CR1 _IOR(PCI_EPN_IOC_MAGIC, 3, u_int8_t)

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* _PCI_EPN_IOCTL_H_ */
