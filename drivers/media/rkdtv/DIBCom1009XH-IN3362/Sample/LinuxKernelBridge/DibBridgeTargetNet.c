/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

/**************************************************************************************************
* @file "DibBridgeTargetNet.c"
* @brief Target specific implementation.
*
***************************************************************************************************/
#include "DibBridgeCommon.h"

/* This module's include */
#include "DibBridgeTargetNet.h"
#include "DibBridgeTarget.h"
#include <linux/crc32.h>
#include <linux/version.h>
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
#include <net/net_namespace.h>
#endif
#include <linux/netpoll.h>
#include <linux/workqueue.h>

/* Net private Data definition */
typedef struct 
{
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
   struct net_device_stats stats;
#endif
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
   struct napi_struct napi; 
#endif
   struct net_device *dev;

   struct work_struct DibNet;
} sPrivData;

uint8_t gLog = 0;

/* Function Prototypes */
static int32_t DibBridgeTargetNetXmit(struct sk_buff *skb, struct net_device *dev);

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
void DibBridgeTargetNetWork(struct work_struct *work) ;
#else
void DibBridgeTargetNetWork(void  *work) ;
#endif

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
static int DibBridgeTargetNetOpen(struct net_device *dev);
static int DibBridgeTargetNetClose(struct net_device *dev);
static int DibBridgeTargetNetPoll(struct napi_struct *napi, int budget);
#else
static int32_t DibBridgeTargetNetPoll(struct net_device *dev, int32_t *budget);
#endif

/* Error Print */
#if (DEBUG_MODE == DEBUG_PRINT)
#define NET_ERR( format... ) { printk(  "Net ERROR %s: ", __FILE__  ); printk(  format  ); printk(  CRB "( %s line %d )" CRA,__FUNCTION__,__LINE__  ); }
#define NET_LOG( format... ) { printk(  "Net Info %s: ", __FILE__  ); printk(  format  ); printk(  CRB "( %s line %d )" CRA,__FUNCTION__,__LINE__  ); }
#else
#define NET_ERR( format... ) { }
#define NET_LOG( format... ) { }
#endif

/*-----------------------------------------------------------------------
|  Globals , since One Net device for all available devices
-----------------------------------------------------------------------*/
struct net_device       *gpDibBridgeTargetNet = NULL;

struct workqueue_struct *gDibQueue            = NULL;
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38))
       DEFINE_RWLOCK(gQueueLock);
#else
       rwlock_t          gQueueLock           = RW_LOCK_UNLOCKED;
#endif
struct list_head         gRxQueue;
       int32_t           gRxQueued;
static uint32_t          gCnt                 = 0;



/*-----------------------------------------------------------------------
|  Free allocated structuers
-----------------------------------------------------------------------*/
void IntBridgeTargetFreeNet(struct net_device *dev)
{
   /* Delete Net Context */
   if(dev)
   {
#if 0
      if(dev->priv)
      {
         kfree(dev->priv);
         dev->priv = NULL;
      }

   kfree(dev);
   dev = NULL;
#else
	free_netdev(dev);
#endif
   }

   /* Delete work queue */
   if(gDibQueue)
   {
      flush_workqueue(gDibQueue);
      destroy_workqueue(gDibQueue);
      gDibQueue = NULL;
   }

   gLog = 0;
}

/*-----------------------------------------------------------------------
|  transmit packets !
-----------------------------------------------------------------------*/
static int32_t DibBridgeTargetNetXmit(struct sk_buff *skb, struct net_device *dev)
{
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
   struct net_device_stats *stats    = &dev->stats;
#else
   struct net_device_stats *stats    = &((sPrivData *) dev->priv)->stats;
#endif

   NET_ERR("discarding xmit Data: %d bytes", skb->len);

   stats->tx_bytes += skb->len;
   stats->tx_packets++;

   dev_kfree_skb(skb);
   return 0;

}

/*-----------------------------------------------------------------------
|  Retrieve device drivers statistics
-----------------------------------------------------------------------*/
static struct net_device_stats *GetStats(struct net_device *dev)
{
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
   return &dev->stats;
#else
   return &((sPrivData *) dev->priv)->stats;
#endif
}

/*-----------------------------------------------------------------------
|  Multicast handling
-----------------------------------------------------------------------*/
void DibBridgeTargetNetSetMulticastList(struct net_device *dev)
{
}

/*-----------------------------------------------------------------------
|  Register device driver
-----------------------------------------------------------------------*/
int32_t DibBridgeTargetNetRegisteredInit(struct net_device *dev)
{
   sPrivData *priv = netdev_priv(dev);
   
   uint8_t dev_add[6] = { 0x00, 0x10, 0xb5, 0xe1, 0x55, 0x56 };

   ether_setup(dev);

   dev->flags          |= IFF_NOARP;
   dev->hard_header_len = ETH_HLEN;     /* 14 */
   dev->addr_len        = ETH_ALEN;     /* 6  */

   memcpy(dev->dev_addr, dev_add, 6);

   /* see http://www.uwsg.iu.edu/hypermail/linux/net/0207.1/0008.html */
   dev->features = NETIF_F_HW_CSUM | NETIF_F_IP_CSUM; 
   dev->if_port  = IF_PORT_100BASET; 

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
   netif_napi_add(dev, &priv->napi, DibBridgeTargetNetPoll, 64);
#else
   dev->poll                = DibBridgeTargetNetPoll;   
   dev->weight              = 64;   
#endif 

 #if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
   INIT_WORK(&priv->DibNet, DibBridgeTargetNetWork);
 #else
   INIT_WORK(&priv->DibNet, DibBridgeTargetNetWork, dev);
 #endif

   return 0;
}

/*-----------------------------------------------------------------------
|  Remove device driver
-----------------------------------------------------------------------*/
void DibBridgeTargetNetCleanup(void)
{
   struct net_device *dev = gpDibBridgeTargetNet;
   struct list_head  *p, *n;
   struct my_sk_buff *my_skb;

   DIB_ASSERT(gCnt >= 1);

   /* Since we have a single net device for all caracter devices */
   gCnt--;
   if(gCnt > 0)
      return;

   /* Last one ? Unresgiter */
   if(dev)
      unregister_netdev(dev);

   list_for_each_safe(p, n, &gRxQueue) 
   {
      my_skb = list_entry(p, struct my_sk_buff, list);
      list_del_init(&my_skb->list);
      gRxQueued--;
      dev_kfree_skb(my_skb->skb);
      kfree(my_skb);
   }

   IntBridgeTargetFreeNet(dev);

   NET_LOG(CRB "Removed Target Net Device" CRA);
}

/**
 *  Linux module init
 */
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
static const struct net_device_ops DibBridgeTargetOps = {
    .ndo_init               = DibBridgeTargetNetRegisteredInit,
    .ndo_open               = DibBridgeTargetNetOpen,
    .ndo_stop               = DibBridgeTargetNetClose,
    .ndo_start_xmit         = DibBridgeTargetNetXmit,
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
    .ndo_set_rx_mode = DibBridgeTargetNetSetMulticastList,
#else
    .ndo_set_multicast_list = DibBridgeTargetNetSetMulticastList,
#endif
    .ndo_get_stats          = GetStats,
};
#endif

int32_t DibBridgeTargetNetInit(void)
{
	int32_t err = -1;
	struct net_device *dev;
	sPrivData *priv;
   

	/* check if already opened */
	if(gpDibBridgeTargetNet)
	{
		dev = gpDibBridgeTargetNet;
		NET_LOG(CRB "Target Net Device Already Open" CRA);
		err = DIBSTATUS_SUCCESS;
		goto End;
	}

	dev = alloc_etherdev(sizeof(sPrivData));
	priv = netdev_priv(dev);

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
    dev->netdev_ops = &DibBridgeTargetOps;
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
	dev->init               = DibBridgeTargetNetRegisteredInit;
    dev->open               = DibBridgeTargetNetOpen;
    dev->stop               = DibBridgeTargetNetClose;
    dev->hard_start_xmit    = DibBridgeTargetNetXmit;
    dev->set_multicast_list = DibBridgeTargetNetSetMulticastList;
    dev->get_stats          = GetStats;
#else
	dev->init               = DibBridgeTargetNetRegisteredInit;
    dev->hard_start_xmit    = DibBridgeTargetNetXmit;
    dev->set_multicast_list = DibBridgeTargetNetSetMulticastList;
    dev->get_stats          = GetStats;
#endif

	strcpy(dev->name,"dvbh%d");

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
	memset(&dev->stats, 0, sizeof(struct net_device_stats));
#else
	memset(&priv->stats, 0, sizeof(struct net_device_stats));
#endif
	priv->dev = dev;

	/* Create workqueue */
	gDibQueue = create_workqueue("DibQueue");
	if(gDibQueue == NULL)
	{
		NET_ERR(CRB "Insufficient memory" CRA);
		err = -ENOMEM;
		goto End;
	}

	/* Init Rx Queue */
	INIT_LIST_HEAD(&gRxQueue); 
	gRxQueued = 0;

	gpDibBridgeTargetNet = dev;
   
	err = register_netdev(dev);
	if(err < 0) 
	{
		NET_ERR("couldn't register device, Err = %d", err);
	}
	else
	{
		NET_LOG(CRB "Target Net Device Registered " CRA);
	}

End:
	if(err < 0)
	{
		/* Free allocated memory */
		IntBridgeTargetFreeNet(dev);
		gpDibBridgeTargetNet = NULL;
	}
	else
	{
		gCnt ++;
	}

	return err;
}

/**
 *  Receive IP Data
 */
void DibBridgeTargetNetSubmitPacket(int32_t len, uint8_t *buf)
{
   struct net_device *dev = gpDibBridgeTargetNet;
   sPrivData *priv = netdev_priv(dev);
   struct my_sk_buff *my_skb;

   uint8_t ipv4Header[14] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x00,
                              /* 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, */
                              0x00, 0x10, 0xb5, 0xe1, 0x5c, 0x5d,
                              0x08, 0x00 };

   uint8_t ipv6Header[14] = { /*0x00, 0x10, 0xb5, 0xe1, 0x55, 0x56, */
                              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                              0x00, 0x10, 0xb5, 0xe1, 0x5c, 0x5d,
                              0x86, 0xDD };
   struct sk_buff *skb;

   skb = dev_alloc_skb(len + 2 + 14);
   if(!skb) 
   {
      NET_ERR(CRB "rx: low on mem - packet dropped" CRA);
      return;
   }
   skb_reserve(skb, 2);

   if((buf[0] & 0xf0) >> 4 == 0x4) 
   {     
      /* IPV4 */
      /* Multicast address mapping */
      memcpy(ipv4Header + 3, buf + 17, 3);
      memcpy(skb_put(skb, 14), ipv4Header, 14);
   }
   else if((buf[0] & 0xf0) >> 4 == 0x6) 
   {        
      /* IPV6 */
      memcpy(skb_put(skb, 14), ipv6Header, 14);
   }
   else 
   {
      NET_ERR("IP version isn't 4 or 6, discarding");
      dev_kfree_skb(skb);
      return;
   }

   memcpy(skb_put(skb, len), buf, len);

   /* Write metadata, and then pass to the receive level */
   skb->dev       = dev;
   skb->protocol  = eth_type_trans(skb, dev);
   skb->ip_summed = CHECKSUM_UNNECESSARY;        /* don't check it */
   skb->pkt_type  = PACKET_HOST; 
   dev->last_rx   = jiffies;

   if(!(dev->flags & IFF_UP) || (gRxQueued > DIB_MAX_QUEUED)) 
   {
      if(gLog == 0)
      {
      /* Inform only once */
  	      NET_LOG(CRB "%s => Warning too many packets queued OR interface not UP" CRA,__func__);
  	      gLog = 1;
  	   }

      dev_kfree_skb(skb);
      queue_work(gDibQueue, &priv->DibNet);
      return;
   }
   my_skb      = kmalloc(sizeof(struct my_sk_buff), GFP_ATOMIC); 
   my_skb->skb = skb; 

   write_lock(&gQueueLock);
   
   list_add_tail(&my_skb->list, &gRxQueue); 
   gRxQueued++;
   
   write_unlock(&gQueueLock);
   
   queue_work(gDibQueue, &priv->DibNet);
}

static uint16_t IntChecksum(void *buf, uint32_t count)
{
   uint32_t  sum = 0;
   uint16_t *Addr = buf;

   while(count > 1)
   {
      /*  This is the inner loop */
      sum   += ntohs(* (uint16_t *) Addr);
      Addr++;
      count -= 2;
   }

   /*  Add left-over byte, if any */
   if(count > 0)
      sum += ntohs(* (uint8_t *) Addr);

   /*  Fold 32-bit sum to 16 bits */
   while(sum >> 16)
      sum = (sum & 0xffff) + (sum >> 16);

   return htons(~sum);
}

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
void DibBridgeTargetNetWork(struct work_struct  *Data)
{
	sPrivData *priv = container_of(Data, sPrivData, DibNet);

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
    napi_schedule(&priv->napi);
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
    netif_rx_schedule(&priv->napi);
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
   	netif_rx_schedule(priv->dev, &priv->napi);
#else
	netif_rx_schedule(priv->dev);
#endif
}
#else
void DibBridgeTargetNetWork(void  *Data)
{
   netif_rx_schedule(gpDibBridgeTargetNet);
}
#endif

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
static int DibBridgeTargetNetOpen(struct net_device *dev)
{
	sPrivData *priv = netdev_priv(dev);
	struct napi_struct *napi    = &priv->napi;
	napi_enable(napi);
	return 0;
}

static int DibBridgeTargetNetClose(struct net_device *dev)
{
	sPrivData *priv = netdev_priv(dev);
	struct napi_struct *napi    = &priv->napi;
	napi_disable(napi);
	return 0;
}

static int DibBridgeTargetNetPoll(struct napi_struct *napi, int budget)
{
   sPrivData *privData;
   struct net_device_stats *stats;
   int32_t                  n;
   struct my_sk_buff       *my_skb;

   privData = container_of(napi, sPrivData, napi);
   stats    = &napi->dev->stats;
   n        = 0;

   if(gRxQueued > DIB_MAX_QUEUED) 
   {
      NET_LOG(CRB "%s => budget = %d" CRA, __FUNCTION__, budget);
   }

   read_lock(&gQueueLock);
   for(n = 0; n < budget; n++) 
   {
      if(list_empty(&gRxQueue)) 
      {
         if(gRxQueued) 
         { 
            INIT_LIST_HEAD(&gRxQueue); 
            gRxQueued = 0;
         }
         goto rx_complete;
      } 
      else 
      {
         my_skb = list_entry(gRxQueue.next, struct my_sk_buff, list);
         list_del_init(&my_skb->list);
         gRxQueued--;
         netif_receive_skb(my_skb->skb);
         stats->rx_bytes += my_skb->skb->len;
         stats->rx_packets++;
         kfree(my_skb);
      }
   }

rx_complete:
   read_unlock(&gQueueLock);
   if (n < budget)
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
    napi_complete(napi);
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
    netif_rx_schedule(napi);
#elif(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
   	netif_rx_complete(privData->dev, napi);
#else
   	netif_rx_complete(privData->dev);
#endif

   return n;
}
#else
static int32_t DibBridgeTargetNetPoll(struct net_device *dev, int32_t *budget) 
{
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
   struct net_device_stats *stats    = &dev->stats;
#else
   struct net_device_stats *stats    = &((sPrivData *) dev->priv)->stats;
#endif
   int32_t                  quota    = min(dev->quota, *budget);
   int32_t                  n        = 0;
   struct my_sk_buff       *my_skb;

   if(gRxQueued > DIB_MAX_QUEUED) 
   {
      NET_ERR(CRB "%s => quota = %d" CRA,__func__, quota);
   }

   read_lock(&gQueueLock);
   for(n = 0; n < quota; n++) 
   {
      if(list_empty(&gRxQueue)) 
      {
         if(gRxQueued) 
         { 
            INIT_LIST_HEAD(&gRxQueue); 
            gRxQueued = 0;
         }
         goto rx_complete;
      } 
      else 
      {
         my_skb = list_entry(gRxQueue.next, struct my_sk_buff, list);
         list_del_init(&my_skb->list);
         gRxQueued--;
         netif_receive_skb(my_skb->skb);
         stats->rx_bytes += my_skb->skb->len;
         stats->rx_packets++;
         kfree(my_skb);
      }
   }

   dev->quota -= n;
   *budget    -= n;
   read_unlock(&gQueueLock);
   
   return 1;

rx_complete:

   read_unlock(&gQueueLock);
   dev->quota -= n;
   *budget    -= n;
   netif_rx_complete(dev);

   return 0;
}
#endif

/**
 *  Encapsulate RAW TS in IP packets and send them to the IP stack
 */


#define NB_TS_PACKETS       43
#define TP_SIZE             188
#define DATA_PAYLOAD_LEN    (NB_TS_PACKETS*TP_SIZE)
#define UDP_HEADER_LEN      8
#define UDP_PACKET_LEN      (DATA_PAYLOAD_LEN + UDP_HEADER_LEN)
#define IP_HEADER_LEN       20
#define IP_PACKET_LEN       (UDP_PACKET_LEN + IP_HEADER_LEN)


uint8_t TmpBuf[IP_PACKET_LEN]; /* tmp buf to hold Data to fill compltly an skb */
int32_t TmpLen = 0;

void DibBridgeTargetNetSubmitRawTsPacket(uint8_t *buf, int32_t len)
{
	struct net_device	*dev = gpDibBridgeTargetNet;
	sPrivData		*priv = netdev_priv(dev);
	struct sk_buff  	*skb;
	struct my_sk_buff	*my_skb;
	uint16_t		 crc16;
	uint32_t		 Offset = 0;

	/* Defines the differents headers needed to encapsulate the Data */
	uint8_t ethHeader[14] = {0x01, 0x00, 0x05, 0x00, 0x00, 0x02, /* Eth Dst Address */
				 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Eth Src Address */
				 0x08, 0x00			     /* Eth Type => 0x800 = IP */
				};

	uint8_t ipv4Header[20] = {0x45, 			     /* => IPv4 , 5 words long  			  */
				  0x00, 
				  (IP_PACKET_LEN >> 8) & 0xFF, IP_PACKET_LEN & 0xFF,		        /* Total len : for UDP with 7 TS = 1324 + 20 = 1344  */
				  0x01, 0x01,			     /* Id : to be incremented  			  */
				  0x00, 0x00, 0x05, 0x11,	     /* some stuff and UDP (11) 			  */
				  0x07, 0x00,			     /* header cksum : to be computed ?!		  */
				  0xC0, 0xA8, 0xC8, 0x03,	     /* Src address => we don't care (192.168.200.3) !    */
				  0x0A, 0x2A, 0x00, 0x01	                        /* Dst address => 10.42.0.1		  */
				 };

	uint8_t udpHeader[8] = {0x80, 0x35,			    /* Src port => same as MPE frames in my case (to test) ! */
				0x04, 0xD2,			    /* Dst port => 1234..................................... */
				(UDP_PACKET_LEN >> 8) & 0xFF, UDP_PACKET_LEN & 0xFF,			    /* Total len => for 7 TS : 7*188 + 8 = 1324 	     */
				0x00, 0x00			    /* cksum => to be computed ?!			     */
			       };

	uint16_t       *Id  = (uint16_t*)&ipv4Header[4];
	static uint16_t SId = 0;

	if(len > RAWTS_BUFFER_SIZE) 
	{
		NET_LOG(CRB "%s:%d => %s : bad TS len (%d)" CRA,__FILE__,__LINE__,__func__, len); 
		return;
	}

	while(1)
	{
		int size = len > DATA_PAYLOAD_LEN-TmpLen ? DATA_PAYLOAD_LEN-TmpLen : len;
	   
		memcpy(&TmpBuf[TmpLen], &buf[Offset], size);
	   
		TmpLen += size;
		Offset += size;
		len -= size;
	   
		if (TmpLen < DATA_PAYLOAD_LEN)
		{
			break;
		}
	   
		/* Prepare the headers according to the Buffer passed if needed */
		/* Increment Id counter */
		*Id = SId++; 

		skb = dev_alloc_skb(IP_PACKET_LEN + 2);  /* 1344 + 14 = 1358 + 2 bytes to align IP header on a 16 byte boundary */
		skb_reserve(skb, 2);			      /* Align IP header on 16 bytes boundary */

		if(!skb) 
		{
			NET_ERR(CRB "rx: low on mem - packet dropped" CRA);
			return;
		}

		/* Compute cksum : Mandatory */
		ipv4Header[10] = 0;
		ipv4Header[11] = 0;
		crc16 = IntChecksum(ipv4Header, IP_HEADER_LEN);
		ipv4Header[10] = (crc16 & 0x00FF);
		ipv4Header[11] = (crc16 & 0xFF00)>>8;

		/* Copy the headers into the skb */
		memcpy(skb_put(skb, 14), ethHeader, 14); 
		memcpy(skb_put(skb, IP_HEADER_LEN), ipv4Header, IP_HEADER_LEN);
		memcpy(skb_put(skb, UDP_HEADER_LEN), udpHeader, UDP_HEADER_LEN);

		/* Copy 7 RAW TS into the skb */
		/* or copy the saved Data from previous round */
		memcpy(skb_put(skb, TmpLen), TmpBuf, TmpLen);
		memset(TmpBuf, 0xff, DATA_PAYLOAD_LEN);
		TmpLen = 0;

		/* Write metadata, and then pass to the receive level */
		skb->dev	  = dev;
		skb->ip_summed = CHECKSUM_UNNECESSARY;	 /* don't check it */
		/*	 skb->pkt_type = PACKET_MULTICAST;    */
		dev->last_rx   = jiffies;
		skb->protocol  = eth_type_trans(skb, dev);

		/* link in a list the new skb */
		/* Only if interface is up and we have just a few packets queued */ 
		if(!(dev->flags & IFF_UP) || (gRxQueued > DIB_MAX_QUEUED)) 
		{
         if(gLog == 0)
         {
         /* Inform only once */
  	         NET_LOG(CRB "%s => Warning too many packets queued OR interface not UP" CRA,__func__);
  	         gLog = 1;
  	      }
         dev_kfree_skb(skb);
			queue_work(gDibQueue, &priv->DibNet);
			return;
		
		}
		my_skb      = kmalloc(sizeof(struct my_sk_buff), GFP_ATOMIC);
		my_skb->skb = skb; 

		write_lock(&gQueueLock);
		list_add_tail(&my_skb->list, &gRxQueue); 
		gRxQueued++;
		write_unlock(&gQueueLock);
	   
		queue_work(gDibQueue, &priv->DibNet);

	}
}
