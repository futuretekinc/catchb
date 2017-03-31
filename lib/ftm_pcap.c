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

FTM_VOID	FTM_PCAP_callback
(
	u_char * pData, 
	const struct pcap_pkthdr * pPacketHeader, 
	const u_char *pPacket
);

FTM_RET	FTM_PCAP_create
(
	FTM_PCAP_PTR _PTR_ ppPCap
)
{
	ASSERT(ppPCap != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_PCAP_PTR	pPCap;

	pPCap = (FTM_PCAP_PTR)FTM_MEM_malloc(sizeof(FTM_PCAP));
	if (pPCap == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create pcap!");
		goto finished;
	}

	*ppPCap = pPCap;

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pPCap != NULL)
		{
			FTM_MEM_free(pPCap);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_PCAP_destroy
(
	FTM_PCAP_PTR _PTR_ ppPCap
)
{
	ASSERT(ppPCap != NULL);

	if (*ppPCap != NULL)
	{
		if ((*ppPCap)->pPorts != NULL)
		{
			FTM_MEM_free((*ppPCap)->pPorts);
		}

		FTM_MEM_free(*ppPCap);
		*ppPCap = NULL;
	}

	return	FTM_RET_OK;

}

FTM_RET	FTM_PCAP_open
(
	FTM_PCAP_PTR	pPCap
)
{
	ASSERT(pPCap != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_INT			nRet;
	FTM_CHAR_PTR	pDev;

	pDev = pcap_lookupdev(pPCap->pErrorBuff);
	if (pDev == NULL)
	{
		xRet = 	FTM_RET_OBJECT_NOT_FOUND;
		ERROR(xRet, "Failed to lookup device[%s]!", pPCap->pErrorBuff);
		return	xRet;
	}

	nRet = pcap_lookupnet(pDev, &pPCap->xNet, &pPCap->xMask, pPCap->pErrorBuff);
	if (nRet == -1)
	{
		xRet = FTM_RET_PCAP_ERROR;
		return	xRet;	
	}

	pPCap->pPCD = pcap_open_live(pDev, BUFSIZ, 0, -1, pPCap->pErrorBuff);
	if (pPCap->pPCD == NULL)
	{
		xRet = FTM_RET_PCAP_OPEN_FAILED;
		ERROR(xRet, "Failed to open live pcap!");
		return	xRet;
	}

	return	xRet;
}

FTM_RET	FTM_PCAP_close
(
	FTM_PCAP_PTR	pPCap
)
{
	ASSERT(pPCap != NULL);

	if (pPCap->pPCD != NULL)
	{
		pcap_close(pPCap->pPCD);
		pPCap->pPCD = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_PCAP_start
(
	FTM_PCAP_PTR	pPCap,
	FTM_UINT32		ulCount
)
{
	ASSERT(pPCap != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT32	i;

	if (pPCap->ppCapturePackets != NULL)
	{
		for(i = 0 ; i < pPCap->ulCaptureCount ; i++)
		{
			if (pPCap->ppCapturePackets[i] != NULL)
			{
				FTM_MEM_free(pPCap->ppCapturePackets[i]);
				pPCap->ppCapturePackets[i] = NULL;
			}
		}

		pPCap->ulCaptureCount = 0;
	}

	if (pPCap->ulMaxCaptureCount != ulCount)
	{
		if (pPCap->ppCapturePackets != NULL)
		{
			FTM_MEM_free(pPCap->ppCapturePackets);
		}
		pPCap->ulMaxCaptureCount = 0;

		pPCap->ppCapturePackets = (FTM_UINT8_PTR _PTR_)FTM_MEM_calloc(sizeof(FTM_UINT8_PTR), ulCount);
		if (pPCap->ppCapturePackets == NULL)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
			goto finished;
		}
		pPCap->ulMaxCaptureCount = ulCount;
	}

	pcap_loop(pPCap->pPCD, ulCount, FTM_PCAP_callback, (u_char *)pPCap);

finished:

	return	xRet;
}

FTM_RET	FTM_PCAP_stop
(
	FTM_PCAP_PTR	pPCap
)
{
	ASSERT(pPCap != NULL);

	if (pPCap->pPCD != NULL)
	{
		pcap_breakloop(pPCap->pPCD);
	}

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
	FTM_PCAP_PTR	pPCap,
	FTM_CHAR_PTR	pOptions
)
{
	ASSERT(pPCap != NULL);
	
	FTM_RET	xRet = FTM_RET_PCAP_ERROR;
	FTM_INT	nRet;
	struct bpf_program	xFilter;

	if (pPCap->pPCD == NULL);
	{
		xRet = FTM_RET_PCAP_NOT_OPENED;
		goto finished;
	}

	if (pOptions != NULL)
	{
		if (pPCap->pOptions != NULL)
		{
			FTM_MEM_free(pPCap->pOptions);
			pPCap->pOptions = NULL;
		}

		pPCap->pOptions = FTM_MEM_malloc(strlen(pOptions) + 1);
		if (pPCap->pOptions == NULL)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			goto finished;
		}

		strcpy(pPCap->pOptions, pOptions);
	}

	nRet = pcap_compile(pPCap->pPCD, &xFilter, pPCap->pOptions, 0, pPCap->xNet);
	if (nRet == -1)
	{
		xRet= FTM_RET_PCAP_ERROR;
		ERROR(xRet, "Failed to compile pcap options!");
		goto finished;
	}

	nRet = pcap_setfilter(pPCap->pPCD, &xFilter);
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

	FTM_PCAP_PTR	pPCap = (FTM_PCAP_PTR)pData;
	FTM_UINT8_PTR	pCapturedPacket;
	FTM_INT32		i;
	struct iphdr _PTR_ pIPH;
	struct tcphdr _PTR_ pTCPH;

	if (pPCap->ulCaptureCount >= pPCap->ulMaxCaptureCount)
	{
		ERROR(FTM_RET_PCAP_ERROR, "Failed to capture packet!");
		return;
	}

	struct ether_header _PTR_ pEthHdr;
	pEthHdr = (struct ether_header *)pPacket;

	if (ntohs(pEthHdr->ether_type) != ETHERTYPE_IP)
	{
		return;
	}

	pIPH = (struct iphdr *)(pPacket + sizeof(struct ether_header));

	if ((pIPH->saddr != pPCap->ulSourceIP) || (pIPH->daddr != pPCap->ulDestIP))
	{
		return;
	}

	pTCPH = (struct tcphdr *)(pIPH + 1);
	for(i = 0 ; i < pPCap->ulPortCount ; i++)
	{
		if (pPCap->pPorts[i] == pTCPH->source)
		{
			break;
		}
	}

	if (i == pPCap->ulPortCount)
	{
		return;	
	}

#if 1
	INFO("%8s : %d", "Version", pIPH->version);
	INFO("%8s : %d", "Length", ntohs(pIPH->tot_len));
    INFO("%8s : %d.%d.%d.%d:%d", "SA", ((pIPH->saddr >> 0) & 0xFF), ((pIPH->saddr >> 8) & 0xFF), ((pIPH->saddr >> 16) & 0xFF), ((pIPH->saddr >> 24) & 0xFF), pTCPH->source);
	INFO("%8s : %d.%d.%d.%d:%d", "DA", ((pIPH->daddr >> 0) & 0xFF), ((pIPH->daddr >> 8) & 0xFF), ((pIPH->daddr >> 16) & 0xFF), ((pIPH->daddr >> 24) & 0xFF), pTCPH->dest);
#endif

	pCapturedPacket  = (FTM_UINT8_PTR)FTM_MEM_malloc(sizeof(struct ether_header) + ntohs(pIPH->tot_len));
	if (pCapturedPacket == NULL)
	{
		return;	
	}

	memcpy(pCapturedPacket, pPacket, sizeof(struct ether_header) + ntohs(pIPH->tot_len));

	
	pPCap->ppCapturePackets[pPCap->ulCaptureCount++] = pCapturedPacket;
}    


