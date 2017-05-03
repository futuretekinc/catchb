#include <string.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include "ftm_pcap.h"
#include "ftm_trace.h"
#include "ftm_mem.h"

#undef	__MODULE__
#define	__MODULE__	"pcap"

FTM_VOID	FTM_PCAP_callback
(
	u_char * pData, 
	const struct pcap_pkthdr * pPacketHeader, 
	const u_char *pPacket
);

FTM_RET	FTM_PCAP_create
(
	FTM_PCAP_PTR _PTR_ ppPCAP
)
{
	ASSERT(ppPCAP != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_PCAP_PTR	pPCAP;

	pPCAP = (FTM_PCAP_PTR)FTM_MEM_malloc(sizeof(FTM_PCAP));
	if (pPCAP == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create pcap!");
		goto finished;
	}

	*ppPCAP = pPCAP;

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pPCAP != NULL)
		{
			FTM_MEM_free(pPCAP);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_PCAP_destroy
(
	FTM_PCAP_PTR _PTR_ ppPCAP
)
{
	ASSERT(ppPCAP != NULL);

	if (*ppPCAP != NULL)
	{
		if ((*ppPCAP)->pPorts != NULL)
		{
			FTM_MEM_free((*ppPCAP)->pPorts);
		}

		FTM_MEM_free(*ppPCAP);
		*ppPCAP = NULL;
	}

	return	FTM_RET_OK;

}

FTM_RET	FTM_PCAP_open
(
	FTM_PCAP_PTR	pPCAP,
	FTM_CHAR_PTR	pIFName
)
{
	ASSERT(pPCAP != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_INT			nRet;

	if (pIFName != NULL)
	{
		strcpy(pPCAP->pDev, pIFName);
	}
	else if (strlen(pPCAP->pDev) == 0)
	{
		FTM_CHAR_PTR	pDev;

		pDev = pcap_lookupdev(pPCAP->pErrorBuff);
		if (pDev == NULL)
		{
			xRet = 	FTM_RET_OBJECT_NOT_FOUND;
			ERROR(xRet, "Failed to lookup device[%s]!", pPCAP->pErrorBuff);
			return	xRet;
		}

		strncpy(pPCAP->pDev, pDev, sizeof(pPCAP->pDev) - 1);
	}

	nRet = pcap_lookupnet(pPCAP->pDev, &pPCAP->xNet, &pPCAP->xMask, pPCAP->pErrorBuff);
	if (nRet == -1)
	{
		xRet = FTM_RET_PCAP_ERROR;
		ERROR(xRet, "Failed to lookup device[%s]!", pPCAP->pErrorBuff);
		return	xRet;	
	}

	pPCAP->pPCD = pcap_open_live(pPCAP->pDev, BUFSIZ, 0, -1, pPCAP->pErrorBuff);
	if (pPCAP->pPCD == NULL)
	{
		xRet = FTM_RET_PCAP_OPEN_FAILED;
		ERROR(xRet, "Failed to open live pcap[%s]!", pPCAP->pDev);
		return	xRet;
	}

	INFO("pPCD : %x", pPCAP->pPCD);
	return	xRet;
}

FTM_RET	FTM_PCAP_close
(
	FTM_PCAP_PTR	pPCAP
)
{
	ASSERT(pPCAP != NULL);

	INFO("PCAP closed!");
	if (pPCAP->pPCD != NULL)
	{
		pcap_close(pPCAP->pPCD);
		pPCAP->pPCD = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_PCAP_start
(
	FTM_PCAP_PTR	pPCAP,
	FTM_UINT32		ulCount
)
{
	ASSERT(pPCAP != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT32	i;

	INFO("PCAP Start!");
	if (pPCAP->ppCapturePackets != NULL)
	{
		for(i = 0 ; i < pPCAP->ulCaptureCount ; i++)
		{
			if (pPCAP->ppCapturePackets[i] != NULL)
			{
				FTM_MEM_free(pPCAP->ppCapturePackets[i]);
				pPCAP->ppCapturePackets[i] = NULL;
			}
		}

		pPCAP->ulCaptureCount = 0;
	}

	if (pPCAP->ulMaxCaptureCount != ulCount)
	{
		if (pPCAP->ppCapturePackets != NULL)
		{
			FTM_MEM_free(pPCAP->ppCapturePackets);
		}
		pPCAP->ulMaxCaptureCount = 0;

		pPCAP->ppCapturePackets = (FTM_UINT8_PTR _PTR_)FTM_MEM_calloc(sizeof(FTM_UINT8_PTR), ulCount);
		if (pPCAP->ppCapturePackets == NULL)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
			goto finished;
		}
		pPCAP->ulMaxCaptureCount = ulCount;
	}

	pPCAP->ulPacketCount = 0;
	pcap_loop(pPCAP->pPCD, ulCount, FTM_PCAP_callback, (u_char *)pPCAP);

finished:

	return	xRet;
}

FTM_RET	FTM_PCAP_stop
(
	FTM_PCAP_PTR	pPCAP
)
{
	ASSERT(pPCAP != NULL);

	if (pPCAP->pPCD != NULL)
	{
		pcap_breakloop(pPCAP->pPCD);
	}

	INFO("PCAP Stopped!");
	INFO("Number of observed packets : %d", pPCAP->ulPacketCount);

	return	FTM_RET_OK;
}

FTM_RET	FTM_PCAP_setIFName
(
	FTM_PCAP_PTR	pPCAP,
	FTM_CHAR_PTR	pIFName
)
{
	ASSERT(pPCAP != NULL);
	ASSERT(pIFName != NULL);

	memset(pPCAP->pDev, 0, sizeof(pPCAP->pDev));
	strncpy(pPCAP->pDev, pIFName, sizeof(pPCAP->pDev) - 1);

	return	FTM_RET_OK;
}

FTM_RET	FTM_PCAP_setFilterIP
(
	FTM_PCAP_PTR	pPCAP,
	FTM_UINT32		ulSIP,
	FTM_UINT32		ulDIP
)
{
	ASSERT(pPCAP != NULL);

	pPCAP->ulSourceIP 	= ulSIP;
	pPCAP->ulDestIP 	= ulDIP;

	return	FTM_RET_OK;
}

FTM_RET	FTM_PCAP_setFilterPorts
(
	FTM_PCAP_PTR	pPCAP,
	FTM_UINT16_PTR	pPorts,
	FTM_UINT32		ulCount
)
{
	ASSERT(pPCAP != NULL);

	if (pPCAP->pPorts != NULL)
	{
		if (pPCAP->ulPortCount != ulCount)
		{
			FTM_MEM_free(pPCAP->pPorts);		

			pPCAP->pPorts = FTM_MEM_malloc(sizeof(FTM_UINT16)*ulCount);
			if (pPCAP->pPorts == NULL)
			{
				pPCAP->ulPortCount = 0;
				return	FTM_RET_NOT_ENOUGH_MEMORY;	
			}
		}
	}
	else
	{
		pPCAP->pPorts = FTM_MEM_malloc(sizeof(FTM_UINT16)*ulCount);
		if (pPCAP->pPorts == NULL)
		{
			pPCAP->ulPortCount = 0;
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	}

	memcpy(pPCAP->pPorts, pPorts, sizeof(FTM_UINT16) * ulCount);
	pPCAP->ulPortCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_PCAP_setFilter
(
	FTM_PCAP_PTR	pPCAP,
	FTM_CHAR_PTR	pOptions
)
{
	ASSERT(pPCAP != NULL);
	
	FTM_RET	xRet = FTM_RET_PCAP_ERROR;
	FTM_INT	nRet;
	struct bpf_program	xFilter;

	if (pPCAP->pPCD == NULL)
	{
		xRet = FTM_RET_PCAP_NOT_OPENED;
		ERROR(xRet, "Failed to set filter because pcap not opened!");
		goto finished;
	}

	if (pOptions != NULL)
	{
		if (pPCAP->pOptions != NULL)
		{
			FTM_MEM_free(pPCAP->pOptions);
			pPCAP->pOptions = NULL;
		}

		pPCAP->pOptions = FTM_MEM_malloc(strlen(pOptions) + 1);
		if (pPCAP->pOptions == NULL)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			ERROR(xRet, "Failed to alloc options!");
			goto finished;
		}

		strcpy(pPCAP->pOptions, pOptions);
		INFO("PCAP Set option : %s", pOptions);
	}

	nRet = pcap_compile(pPCAP->pPCD, &xFilter, pPCAP->pOptions, 0, pPCAP->xNet);
	if (nRet == -1)
	{
		xRet= FTM_RET_PCAP_ERROR;
		ERROR(xRet, "Failed to compile pcap options!");
		goto finished;
	}

	nRet = pcap_setfilter(pPCAP->pPCD, &xFilter);
	if (nRet == -1)
	{
		xRet = FTM_RET_PCAP_ERROR;
		ERROR(xRet, "Failed to set filter!");
	}

finished:
	return	xRet;	
}

FTM_VOID	FTM_PCAP_callback
(
	u_char *pData, 
	const struct pcap_pkthdr * pPacketHeader, 
	const u_char *pPacket
)
{
	ASSERT(pData != NULL);

	FTM_PCAP_PTR	pPCAP = (FTM_PCAP_PTR)pData;
	FTM_UINT8_PTR	pCapturedPacket;
	FTM_CHAR		pSrcIP[64];
	FTM_CHAR		pDestIP[64];
	struct iphdr _PTR_ pIPH;

	pPCAP->ulPacketCount++;
	if (pPCAP->ulCaptureCount >= pPCAP->ulMaxCaptureCount)
	{
		ERROR(FTM_RET_PCAP_ERROR, "Failed to capture packet!");
		return;
	}

	struct ether_header _PTR_ pEthHdr;
	pEthHdr = (struct ether_header *)pPacket;

	if (ntohs(pEthHdr->ether_type) != ETHERTYPE_IP)
	{
		INFO("Not IP!");
		return;
	}

	pIPH = (struct iphdr *)(pPacket + sizeof(struct ether_header));

	strcpy(pSrcIP, inet_ntoa(*(struct in_addr*)&pIPH->saddr));
	strcpy(pDestIP, inet_ntoa(*(struct in_addr*)&pIPH->daddr));

	if (pIPH->protocol == 6)
	{
		struct tcphdr _PTR_ pTCPH;

		pTCPH = (struct tcphdr *)(pPacket + sizeof(struct ether_header) + sizeof(struct iphdr));

		INFO("%2d : %s:%d -> %s:%d", pIPH->protocol, pSrcIP, ntohs(pTCPH->source), pDestIP, ntohs(pTCPH->dest));
	}
	else
	{
		INFO("%2d : %s -> %s", pIPH->protocol, pSrcIP, pDestIP);
	}

	pCapturedPacket  = (FTM_UINT8_PTR)FTM_MEM_malloc(sizeof(struct ether_header) + ntohs(pIPH->tot_len));
	if (pCapturedPacket == NULL)
	{
		return;	
	}

	memcpy(pCapturedPacket, pPacket, sizeof(struct ether_header) + ntohs(pIPH->tot_len));

	
	pPCAP->ppCapturePackets[pPCAP->ulCaptureCount++] = pCapturedPacket;
}    


