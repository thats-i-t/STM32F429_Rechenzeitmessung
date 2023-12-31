#include "usb_dcd_int.h"
#include "usbd_core.h"

static uint32_t DCD_ReadDevInEP (USB_OTG_CORE_HANDLE *pdev, uint8_t epnum);

static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_HandleSof_ISR(USB_OTG_CORE_HANDLE *pdev);

static uint32_t DCD_HandleRxStatusQueueLevel_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_WriteEmptyTxFifo(USB_OTG_CORE_HANDLE *pdev , uint32_t epnum);

static uint32_t DCD_HandleUsbReset_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_HandleEnumDone_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_HandleResume_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_HandleUSBSuspend_ISR(USB_OTG_CORE_HANDLE *pdev);

static uint32_t DCD_IsoINIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev);
static uint32_t DCD_IsoOUTIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev);


uint32_t USBD_OTG_ISR_Handler(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_GINTSTS_TypeDef  gintr_status;
  uint32_t retval = 0;
  
  if (USB_OTG_IsDeviceMode(pdev)) /* ensure that we are in device mode */
  {
    gintr_status.d32 = USB_OTG_ReadCoreItr(pdev);
    if (!gintr_status.d32) /* avoid spurious interrupt */
    {
      return 0;
    }
    
    if (gintr_status.b.outepintr)
    {
      retval |= DCD_HandleOutEP_ISR(pdev);
    }    
    
    if (gintr_status.b.inepint)
    {
      retval |= DCD_HandleInEP_ISR(pdev);
    }
    
    if (gintr_status.b.modemismatch)
    {
      USB_OTG_GINTSTS_TypeDef  gintsts;
      
      /* Clear interrupt */
      gintsts.d32 = 0;
      gintsts.b.modemismatch = 1;
      USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);
    }
    
    if (gintr_status.b.wkupintr)
    {
      retval |= DCD_HandleResume_ISR(pdev);
    }    
    if (gintr_status.b.usbsuspend)
    {
      retval |= DCD_HandleUSBSuspend_ISR(pdev);
    }
    if (gintr_status.b.sofintr)
    {
      retval |= DCD_HandleSof_ISR(pdev);      
    }    
    if (gintr_status.b.rxstsqlvl)
    {
      retval |= DCD_HandleRxStatusQueueLevel_ISR(pdev);      
    }    
    if (gintr_status.b.usbreset)
    {
      retval |= DCD_HandleUsbReset_ISR(pdev);      
    }
    if (gintr_status.b.enumdone)
    {
      retval |= DCD_HandleEnumDone_ISR(pdev);
    }    
    if (gintr_status.b.incomplisoin)
    {
      retval |= DCD_IsoINIncomplete_ISR(pdev);
    }
    if (gintr_status.b.incomplisoout)
    {
      retval |= DCD_IsoOUTIncomplete_ISR(pdev);
    }    
  }
  return retval;
}

static uint32_t DCD_HandleResume_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_GINTSTS_TypeDef  gintsts;
  USB_OTG_DCTL_TypeDef     devctl;
  USB_OTG_PCGCCTL_TypeDef  power;
  
  if(pdev->cfg.low_power)
  {
    /* un-gate USB Core clock */
    power.d32 = USB_OTG_READ_REG32(pdev->regs.PCGCCTL);
    power.b.gatehclk = 0;
    power.b.stoppclk = 0;
    USB_OTG_WRITE_REG32(pdev->regs.PCGCCTL, power.d32);
  }
  
  /* Clear the Remote Wake-up Signaling */
  devctl.d32 = 0;
  devctl.b.rmtwkupsig = 1;
  USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DCTL, devctl.d32, 0);
  
  /* Inform upper layer by the Resume Event */
  USBD_Resume(pdev);
  
  /* Clear interrupt */
  gintsts.d32 = 0;
  gintsts.b.wkupintr = 1;
  USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GINTSTS, gintsts.d32);
  return 1;
}

static uint32_t DCD_HandleUSBSuspend_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_GINTSTS_TypeDef  gintsts;
  USB_OTG_PCGCCTL_TypeDef  power;
  USB_OTG_DSTS_TypeDef     dsts;
  __IO uint8_t prev_status = 0;
  
  prev_status = pdev->dev.device_status;
  USBD_Suspend(pdev);      
  
  dsts.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DSTS);
    
  /* Clear interrupt */
  gintsts.d32 = 0;
  gintsts.b.usbsuspend = 1;
  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);
  
  if((pdev->cfg.low_power) && (dsts.b.suspsts == 1)  && 
    (pdev->dev.connection_status == 1) && 
    (prev_status  == USB_OTG_CONFIGURED))
  {
	/*  switch-off the clocks */
    power.d32 = 0;
    power.b.stoppclk = 1;
    USB_OTG_MODIFY_REG32(pdev->regs.PCGCCTL, 0, power.d32);  
    
    power.b.gatehclk = 1;
    USB_OTG_MODIFY_REG32(pdev->regs.PCGCCTL, 0, power.d32);
    
    /* Request to enter Sleep mode after exit from current ISR */
    SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk);
  }
  return 1;
}

static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_DIEPINTn_TypeDef  diepint;
  
  uint32_t ep_intr;
  uint32_t epnum = 0;
  uint32_t fifoemptymsk;
  diepint.d32 = 0;
  ep_intr = USB_OTG_ReadDevAllInEPItr(pdev);
  
  while ( ep_intr )
  {
    if ((ep_intr & 0x1) == 0x01) /* In ITR */
    {
      diepint.d32 = DCD_ReadDevInEP(pdev , epnum); /* Get In ITR status */
      if ( diepint.b.xfercompl )
      {
        fifoemptymsk = 0x1 << epnum;
        USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DIEPEMPMSK, fifoemptymsk, 0);
        CLEAR_IN_EP_INTR(epnum, xfercompl);
        /* TX COMPLETE */
        USBD_DataInStage(pdev , epnum);
        
        if (pdev->cfg.dma_enable == 1)
        {
          if((epnum == 0) && (pdev->dev.device_state == USB_OTG_EP0_STATUS_IN))
          {
            /* prepare to rx more setup packets */
            USB_OTG_EP0_OutStart(pdev);
          }
        }           
      }
      if ( diepint.b.timeout )
      {
        CLEAR_IN_EP_INTR(epnum, timeout);
      }
      if (diepint.b.intktxfemp)
      {
        CLEAR_IN_EP_INTR(epnum, intktxfemp);
      }
      if (diepint.b.inepnakeff)
      {
        CLEAR_IN_EP_INTR(epnum, inepnakeff);
      }
      if ( diepint.b.epdisabled )
      {
        CLEAR_IN_EP_INTR(epnum, epdisabled);
      }       
      if (diepint.b.emptyintr)
      {
        DCD_WriteEmptyTxFifo(pdev , epnum);
      }
    }
    epnum++;
    ep_intr >>= 1;
  }
  
  return 1;
}

static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  uint32_t ep_intr;
  USB_OTG_DOEPINTn_TypeDef  doepint;
  USB_OTG_DEPXFRSIZ_TypeDef  deptsiz;
  uint32_t epnum = 0;
  
  doepint.d32 = 0;
  
  /* Read in the device interrupt bits */
  ep_intr = USB_OTG_ReadDevAllOutEp_itr(pdev);

  while(ep_intr)
  {
    if (ep_intr&0x1)
    {      
      doepint.d32 = USB_OTG_ReadDevOutEP_itr(pdev, epnum);
      
      /* Transfer complete */
      if ( doepint.b.xfercompl )
      {
        /* Clear the bit in DOEPINTn for this interrupt */
        CLEAR_OUT_EP_INTR(epnum, xfercompl);
        if (pdev->cfg.dma_enable == 1)
        {
          deptsiz.d32 = USB_OTG_READ_REG32(&(pdev->regs.OUTEP_REGS[epnum]->DOEPTSIZ));
          /*ToDo : handle more than one single MPS size packet */
          pdev->dev.out_ep[epnum].xfer_count = pdev->dev.out_ep[epnum].maxpacket - \
            deptsiz.b.xfersize;
        }
        /* Inform upper layer: data ready */
        /* RX COMPLETE */
        USBD_DataOutStage(pdev , epnum);
        
        if (pdev->cfg.dma_enable == 1)
        {
          if((epnum == 0) && (pdev->dev.device_state == USB_OTG_EP0_STATUS_OUT))
          {
            /* prepare to rx more setup packets */
            USB_OTG_EP0_OutStart(pdev);
          }
        }        
      }
      /* Endpoint disable  */
      if ( doepint.b.epdisabled )
      {
        /* Clear the bit in DOEPINTn for this interrupt */
        CLEAR_OUT_EP_INTR(epnum, epdisabled);
      }
      /* Setup Phase Done (control EPs) */
      if ( doepint.b.setup )
      {        
        /* inform the upper layer that a setup packet is available */
        /* SETUP COMPLETE */
        USBD_SetupStage(pdev);
        CLEAR_OUT_EP_INTR(epnum, setup);
      }
    }
    epnum++;
    ep_intr >>= 1;
  }
  return 1;
}

static uint32_t DCD_HandleSof_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_GINTSTS_TypeDef  GINTSTS;
  USBD_SOF(pdev);
  /* Clear interrupt */
  GINTSTS.d32 = 0;
  GINTSTS.b.sofintr = 1;
  USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GINTSTS, GINTSTS.d32);  
  return 1;
}

static uint32_t DCD_HandleRxStatusQueueLevel_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_GINTMSK_TypeDef  int_mask;
  USB_OTG_DRXSTS_TypeDef   status;
  USB_OTG_EP *ep;
  
  /* Disable the Rx Status Queue Level interrupt */
  int_mask.d32 = 0;
  int_mask.b.rxstsqlvl = 1;
  USB_OTG_MODIFY_REG32( &pdev->regs.GREGS->GINTMSK, int_mask.d32, 0);
  
  /* Get the Status from the top of the FIFO */
  status.d32 = USB_OTG_READ_REG32( &pdev->regs.GREGS->GRXSTSP );
  
  ep = &pdev->dev.out_ep[status.b.epnum];
  
  switch (status.b.pktsts)
  {
  case STS_GOUT_NAK:
    break;
  case STS_DATA_UPDT:
    if (status.b.bcnt)
    {
      USB_OTG_ReadPacket(pdev,ep->xfer_buff, status.b.bcnt);
      ep->xfer_buff += status.b.bcnt;
      ep->xfer_count += status.b.bcnt;
    }
    break;
  case STS_XFER_COMP:
    break;
  case STS_SETUP_COMP:
    break;
  case STS_SETUP_UPDT:
    /* Copy the setup packet received in FIFO into the setup buffer in RAM */
    USB_OTG_ReadPacket(pdev , pdev->dev.setup_packet, 8);
    ep->xfer_count += status.b.bcnt;
    break;
  default:
    break;
  }
  
  /* Enable the Rx Status Queue Level interrupt */
  USB_OTG_MODIFY_REG32( &pdev->regs.GREGS->GINTMSK, 0, int_mask.d32);
  
  return 1;
}

static uint32_t DCD_WriteEmptyTxFifo(USB_OTG_CORE_HANDLE *pdev, uint32_t epnum)
{
  USB_OTG_DTXFSTSn_TypeDef  txstatus;
  USB_OTG_EP *ep;
  uint32_t len = 0;
  uint32_t len32b;
  txstatus.d32 = 0;
  uint32_t fifoemptymsk;
  
  ep = &pdev->dev.in_ep[epnum];    
  
  len = ep->xfer_len - ep->xfer_count;
  
  if (len > ep->maxpacket)
  {
    len = ep->maxpacket;
  }
  
  len32b = (len + 3) / 4;
  txstatus.d32 = USB_OTG_READ_REG32( &pdev->regs.INEP_REGS[epnum]->DTXFSTS);
  
  while  (txstatus.b.txfspcavail > len32b &&
          ep->xfer_count < ep->xfer_len &&
            ep->xfer_len != 0)
  {
    /* Write the FIFO */
    len = ep->xfer_len - ep->xfer_count;
    
    if (len > ep->maxpacket)
    {
      len = ep->maxpacket;
    }
    len32b = (len + 3) / 4;
    
    USB_OTG_WritePacket (pdev , ep->xfer_buff, epnum, len);
    
    ep->xfer_buff  += len;
    ep->xfer_count += len;
    
    txstatus.d32 = USB_OTG_READ_REG32(&pdev->regs.INEP_REGS[epnum]->DTXFSTS);
    
    /* Mask the TxFIFOEmpty interrupt  */
    if (ep->xfer_len == ep->xfer_count)
    {
      fifoemptymsk = 0x1 << ep->num;  
      USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DIEPEMPMSK, 
                           fifoemptymsk, 0); 
    }
  }
  
  return 1;
}

static uint32_t DCD_HandleUsbReset_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_DAINT_TypeDef    daintmsk;
  USB_OTG_DOEPMSK_TypeDef  doepmsk;
  USB_OTG_DIEPMSK_TypeDef  diepmsk;
  USB_OTG_DCFG_TypeDef     dcfg;
  USB_OTG_DCTL_TypeDef     dctl;
  USB_OTG_GINTSTS_TypeDef  gintsts;
  uint32_t i;
  
  dctl.d32 = 0;
  daintmsk.d32 = 0;
  doepmsk.d32 = 0;
  diepmsk.d32 = 0;
  dcfg.d32 = 0;
  gintsts.d32 = 0;
  
  /* Clear the Remote Wake-up Signaling */
  dctl.b.rmtwkupsig = 1;
  USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DCTL, dctl.d32, 0 );
  
  /* Flush the Tx FIFO */
  USB_OTG_FlushTxFifo(pdev ,  0 );
  
  for (i = 0; i < pdev->cfg.dev_endpoints ; i++)
  {
    USB_OTG_WRITE_REG32( &pdev->regs.INEP_REGS[i]->DIEPINT, 0xFF);
    USB_OTG_WRITE_REG32( &pdev->regs.OUTEP_REGS[i]->DOEPINT, 0xFF);
  }
  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DAINT, 0xFFFFFFFF );
  
  daintmsk.ep.in = 1;
  daintmsk.ep.out = 1;
  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DAINTMSK, daintmsk.d32 );
  
  doepmsk.b.setup = 1;
  doepmsk.b.xfercompl = 1;
  doepmsk.b.epdisabled = 1;
  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DOEPMSK, doepmsk.d32 );
  diepmsk.b.xfercompl = 1;
  diepmsk.b.timeout = 1;
  diepmsk.b.epdisabled = 1;

  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DIEPMSK, diepmsk.d32 );
  /* Reset Device Address */
  dcfg.d32 = USB_OTG_READ_REG32( &pdev->regs.DREGS->DCFG);
  dcfg.b.devaddr = 0;
  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DCFG, dcfg.d32);
  
  
  /* setup EP0 to receive SETUP packets */
  USB_OTG_EP0_OutStart(pdev);
  
  /* Clear interrupt */
  gintsts.d32 = 0;
  gintsts.b.usbreset = 1;
  USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GINTSTS, gintsts.d32);
  
  /*Reset internal state machine */
  USBD_Reset(pdev);
  return 1;
}

static uint32_t DCD_HandleEnumDone_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  uint32_t hclk = 168000000;
  
  USB_OTG_GINTSTS_TypeDef  gintsts;
  USB_OTG_GUSBCFG_TypeDef  gusbcfg;
  RCC_ClocksTypeDef RCC_Clocks;
  USB_OTG_EP0Activate(pdev);
  
  /* Get HCLK frequency */
  RCC_GetClocksFreq(&RCC_Clocks);
  hclk = RCC_Clocks.HCLK_Frequency;

  /* Clear default TRDT value and Set USB turn-around time based on device speed and PHY interface. */
  gusbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);
  gusbcfg.b.usbtrdtim = 0;
  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, gusbcfg.d32);
  
  /* Full or High speed */
  if ( USB_OTG_GetDeviceSpeed(pdev) == USB_SPEED_HIGH)
  {
    pdev->cfg.speed            = USB_OTG_SPEED_HIGH;
    pdev->cfg.mps              = USB_OTG_HS_MAX_PACKET_SIZE ;    
    
    /*USBTRD min For HS device*/
    gusbcfg.b.usbtrdtim = 9;
  }
  else
  {
    pdev->cfg.speed            = USB_OTG_SPEED_FULL;
    pdev->cfg.mps              = USB_OTG_FS_MAX_PACKET_SIZE ; 
    
    /* The USBTRD is configured according to the tables below, depending on AHB frequency 
    used by application. In the low AHB frequency range it is used to stretch enough the USB response 
    time to IN tokens, the USB turnaround time, so to compensate for the longer AHB read access 
    latency to the Data FIFO */
    
    if((hclk >= 15000000)&&(hclk < 16000000))
    {
      /* hclk Clock Range between 15-16 MHz */
      gusbcfg.b.usbtrdtim = 0xE;
    }
    
    else if((hclk >= 16000000)&&(hclk < 17100000))
    {
      /* hclk Clock Range between 16-17.1 MHz */
      gusbcfg.b.usbtrdtim = 0xD;
    }
    
    else if((hclk >= 17100000)&&(hclk < 18400000))
    {
      /* hclk Clock Range between 17-18.4 MHz */
      gusbcfg.b.usbtrdtim = 0xC;
    }
    
    else if((hclk >= 18400000)&&(hclk < 20000000))
    {
      /* hclk Clock Range between 18.4-20 MHz */
      gusbcfg.b.usbtrdtim = 0xB;
    }
    
    else if((hclk >= 20000000)&&(hclk < 21800000))
    {
      /* hclk Clock Range between 20-21.8 MHz */
      gusbcfg.b.usbtrdtim = 0xA;
    }
    
    else if((hclk >= 21800000)&&(hclk < 24000000))
    {
      /* hclk Clock Range between 21.8-24 MHz */
      gusbcfg.b.usbtrdtim = 0x9;
    }
    
    else if((hclk >= 24000000)&&(hclk < 26600000))
    {
      /* hclk Clock Range between 24-26.6 MHz */
      gusbcfg.b.usbtrdtim = 0x8;
    }
    
    else if((hclk >= 26600000)&&(hclk < 30000000))
    {
      /* hclk Clock Range between 26.6-30 MHz */
      gusbcfg.b.usbtrdtim = 0x7;
    }
    
    else if((hclk >= 30000000)&&(hclk < 34300000))
    {
      /* hclk Clock Range between 30-34.3 MHz */
      gusbcfg.b.usbtrdtim= 0x6;
    } 
    
    else /* if(hclk >= 34300000) */
    {
      /* hclk Clock Range between 34.3-168 MHz */
      gusbcfg.b.usbtrdtim = 0x5;
    }
  }

  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, gusbcfg.d32);
  
  /* Clear interrupt */
  gintsts.d32 = 0;
  gintsts.b.enumdone = 1;
  USB_OTG_WRITE_REG32( &pdev->regs.GREGS->GINTSTS, gintsts.d32 );
  return 1;
}

static uint32_t DCD_IsoINIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_GINTSTS_TypeDef gintsts;  
  
  gintsts.d32 = 0;
  
  /* Clear interrupt */
  gintsts.b.incomplisoin = 1;
  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);
  
  return 1;
}

static uint32_t DCD_IsoOUTIncomplete_ISR(USB_OTG_CORE_HANDLE *pdev)
{
  USB_OTG_GINTSTS_TypeDef gintsts;  
  
  gintsts.d32 = 0;

  /* Clear interrupt */
  gintsts.b.incomplisoout = 1;
  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GINTSTS, gintsts.d32);
  return 1;
}

static uint32_t DCD_ReadDevInEP (USB_OTG_CORE_HANDLE *pdev, uint8_t epnum)
{
  uint32_t v, msk, emp;
  msk = USB_OTG_READ_REG32(&pdev->regs.DREGS->DIEPMSK);
  emp = USB_OTG_READ_REG32(&pdev->regs.DREGS->DIEPEMPMSK);
  msk |= ((emp >> epnum) & 0x1) << 7;
  v = USB_OTG_READ_REG32(&pdev->regs.INEP_REGS[epnum]->DIEPINT) & msk;
  return v;
}
