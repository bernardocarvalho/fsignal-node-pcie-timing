#ifndef _CFN_C641X_IOCTL_H_
#define _CFN_C641X_IOCTL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IOCTL definitions
 */
#define CFN_C641X_IOC_MAGIC 'k'  // /* Please use a different 8-bit number in your code */
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
//#define CFN_C641X_IOCT_DSP_HOLD _IO(CFN_C641X_IOC_MAGIC, 1)
#define CFN_C641X_IOCS_CONFIG_ACQ _IOW(CFN_C641X_IOC_MAGIC, 2, u_int32_t)
#define CFN_C641X_IOCT_INT_ENABLE _IO(CFN_C641X_IOC_MAGIC, 3)
#define CFN_C641X_IOCT_INT_DISABLE _IO(CFN_C641X_IOC_MAGIC, 4)
//#define CFN_C641X_IOCS_BAR1_WREG _IOW(CFN_C641X_IOC_MAGIC, 5, uint32_t)
//#define CFN_C641X_IOCG_BAR1_RREG _IOR(CFN_C641X_IOC_MAGIC, 6, uint32_t)
#define CFN_C641X_IOCG_IRQ_CNT _IOR(CFN_C641X_IOC_MAGIC, 7, u_int32_t)
#define CFN_C641X_IOCT_SOFT_TRIG _IO(CFN_C641X_IOC_MAGIC, 8)
#define CFN_C641X_IOCT_DMA_ENABLE _IO(CFN_C641X_IOC_MAGIC, 9)
#define CFN_C641X_IOCT_DMA_DISABLE _IO(CFN_C641X_IOC_MAGIC, 10)
#define CFN_C641X_IOCS_TMOUT      _IOW(CFN_C641X_IOC_MAGIC, 11, u_int32_t)
#define CFN_C641X_IOCS_CHAN       _IOW(CFN_C641X_IOC_MAGIC, 12, u_int32_t)

#define CFN_C641X_IOCT_DSP_HOLD    _IO(CFN_C641X_IOC_MAGIC, 20)
#define CFN_C641X_IOCT_DSP_FREE    _IO(CFN_C641X_IOC_MAGIC, 21)
#define CFN_C641X_IOCT_DSP_RESET    _IO(CFN_C641X_IOC_MAGIC, 22)
#define CFN_C641X_IOCG_STATUS      _IOR(CFN_C641X_IOC_MAGIC, 23, status_port_t)

#define CFN_C641X_IOC_MAXNR   23

//#define CFN_C641X_IOC_RESET _IO(CFN_C641X_IOC_MAGIC, 0)

//#define CFN_C641X_IOCG_BRD_INFO  _IOR(CFN_C641X_IOC_MAGIC, 1, BOARD_INFO)

//#define CFN_C641X_IOCS_CR1 _IOW(CFN_C641X_IOC_MAGIC, 2, u_int8_t)
//#define CFN_C641X_IOCG_CR1 _IOR(CFN_C641X_IOC_MAGIC, 3, u_int8_t)

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* _CFN_C641X_IOCTL_H_ */
