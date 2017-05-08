#include <string.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include "ftm_score.h"
#include "ftm_trace.h"

#undef	__MODULE__
#define	__MODULE__ "analyzer"


void FTM_SCORE_processIP
(
	FTM_SCORE_PTR	pScoreInfo,
	FTM_UINT8_PTR	pPacket,
	FTM_UINT32		ulLen
)
{
	struct iphdr* pIPHdr 	= ((struct iphdr _PTR_)(pPacket + 14));
	//FTM_INT	nIPHdrLen = pIPHdr->ihl * 4;  // obtain the length of header in bytes to check for options

	if (pScoreInfo->xOSMatrix.bTTL == FTM_FALSE)
	{    
		if ((pIPHdr->ttl > 0) && (pIPHdr->ttl <= 32))
		{    
			pScoreInfo->fValue +=0.1;
		}    
		else if ((pIPHdr->ttl > 32) && (pIPHdr->ttl <= 64))
		{    
			pScoreInfo->fValue +=0.2;
		}    
		else if ((pIPHdr->ttl > 64) && (pIPHdr->ttl <= 128))
		{    
			pScoreInfo->fValue +=0.3;
		}    
		else if ((pIPHdr->ttl > 128) && (pIPHdr->ttl <= 255))
		{    
			pScoreInfo->fValue +=0.4;
		}    

		pScoreInfo->xOSMatrix.bTTL = FTM_TRUE;
	}    


	if (pScoreInfo->xOSMatrix.bDF == FTM_FALSE)
	{    
		if ((htons(pIPHdr->frag_off) & 0x4000) == 0x0000)      //DF = 0
		{    
			pScoreInfo->fValue +=0.1;
		}    
		else if ((htons(pIPHdr->frag_off) & 0x4000) == 0x4000)     //DF = 1
		{    
			pScoreInfo->fValue +=0.2;
		}    

		pScoreInfo->xOSMatrix.bDF = FTM_TRUE;
	}    


}//end Process_IP


FTM_VOID	FTM_SCORE_processTCP
(
	FTM_SCORE_PTR	pScoreInfo,
	FTM_UINT8_PTR	pPacket, 
	FTM_UINT32		ulLen
)
{
	struct iphdr* pIPHdr 	= ((struct iphdr*)(pPacket + 14));
	FTM_UINT32	ulIPHdrLen 	= pIPHdr->ihl * 4;  // obtain the length of header in bytes to check for options

	if (pIPHdr->protocol != 6)
	{
		return;
	}

	struct tcphdr* pTCPHdr = ((struct tcphdr*)(pPacket + 14 + ulIPHdrLen));
	FTM_UINT32	ulTCPHdrLen = pTCPHdr->doff * 4;  // obtain the length of header in bytes to check for options
	FTM_UINT32	ulTCPOptLen = ulTCPHdrLen - 20;

	FTM_UINT16	ulTCPMSS = 0;
	FTM_INT32	nWindowScale = -1;

	if (pTCPHdr->syn)
	{
		FTM_INT32	nOptionIndex = 0;
		FTM_BOOL	bEndOfOptions = FTM_FALSE;

		while((bEndOfOptions == FTM_FALSE) && (nOptionIndex < ulTCPOptLen))
		{
			switch(pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex])
			{
				
			case 0:
			default:
				bEndOfOptions = FTM_TRUE;
				break;

			case 1/*TCPOPT_NOP*/:
				nOptionIndex++;
				break;

			case 2/*TCPOPT_MAXSEG*/:
				ulTCPMSS = (pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 2] * 256) + pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 3];
				nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
				break;

			case 3/*TCPOPT_WSCALE*/:
				nWindowScale = pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 2];
				nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
				break;

			case 4/*TCPOPT_SACKOK*/:
				nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
				break;

			case 8/*TCPOPT_TIMESTAMP*/:
				nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
				break;
			}
		}


		if(pScoreInfo->xOSMatrix.bTCPMSS == FTM_FALSE)
		{
			if (ulTCPMSS == 1380)
			{
				pScoreInfo->fValue+=1;      // MacOS 9.1
			}

			pScoreInfo->xOSMatrix.bTCPMSS = FTM_TRUE;
		}


		if(pScoreInfo->xOSMatrix.bWindowScale == FTM_FALSE)
		{
			if (nWindowScale == -1)     //no window scale appears in tcp options
			{
				pScoreInfo->fValue+=2;       // Linux 1.2


			}
			else if (nWindowScale == 0)
			{
				pScoreInfo->fValue+=3;       // Linux 1.2
			}
			else if (nWindowScale == 1)
			{
				pScoreInfo->fValue+=4;       // Linux 1.2
			}
			else if (nWindowScale == 2)
			{
				pScoreInfo->fValue+=5;       // Linux 1.2
			}


			else if ((nWindowScale == 5) | (nWindowScale == 6) | (nWindowScale == 7))
			{
				pScoreInfo->fValue+=6;       // Linux 1.2
			}
			else if (nWindowScale == 8)
			{
				pScoreInfo->fValue+=7;       // Linux 1.2
			}

			pScoreInfo->xOSMatrix.bWindowScale = FTM_TRUE;
		}
		if(pScoreInfo->xOSMatrix.bWindowSize == FTM_FALSE)
		{
			int temp_window = htons(pTCPHdr->window);

			if ((temp_window <= (ulTCPMSS + 50)) & (temp_window >= (ulTCPMSS - 50)))
			{
				pScoreInfo->fValue+=10;       // Linux 1.2
			}

			if ((temp_window <= (32736 + 70)) & (temp_window >= (32736 - 70)))
			{
				pScoreInfo->fValue+=20;       // Linux 1.2
			}

			if (((temp_window <= (512 + 50)) & (temp_window >= (512 - 50))) |
					((temp_window <= (16384 + 70)) & (temp_window >= (16384 - 70))))
			{
				pScoreInfo->fValue+=30;       // Linux 1.2
			}


			if (((temp_window <= (ulTCPMSS * 11) + 70) & (temp_window >= (ulTCPMSS * 11) - 70 )) |
					((temp_window <= (ulTCPMSS * 20) + 70) & (temp_window >= (ulTCPMSS * 20) - 70 )))
			{
				pScoreInfo->fValue+=40;       // Linux 1.2
			}

			if (((temp_window <= (ulTCPMSS * 2) + 70) & (temp_window >= (ulTCPMSS * 2) - 70 )) |
					((temp_window <= (ulTCPMSS * 3) + 70) & (temp_window >= (ulTCPMSS * 3) - 70 )) |
					((temp_window <= (ulTCPMSS * 4) + 70) & (temp_window >= (ulTCPMSS * 4) - 70 )))
			{
				pScoreInfo->fValue+=50;       // Linux 1.2
			}

			if ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70)))
			{
				pScoreInfo->fValue+=60;       // Linux 1.2
			}

			if (((temp_window <= (ulTCPMSS * 44) + 70) & (temp_window >= (ulTCPMSS * 44) - 70 )))
			{
				pScoreInfo->fValue+=70;       // Linux 1.2
			}

			if ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70)))
			{

				pScoreInfo->fValue+=80;       // Linux 1.2
			}

			if (((temp_window <= 65535) && (temp_window >= (65535 - 70))) ||
				((temp_window <= (8192 + 70)) && (temp_window >= (8192 - 70))) ||
				((temp_window <= (32767 + 70)) && (temp_window >= (32767 - 70))) ||
				((temp_window <= (37300 + 70)) && (temp_window >= (37300 - 70))) ||
				((temp_window <= (46080 + 70)) && (temp_window >= (46080 - 70))) ||
				((temp_window <= (60352 + 70)) && (temp_window >= (60352 - 70))) ||
				((temp_window <= (ulTCPMSS * 44) + 70) && (temp_window >= (ulTCPMSS * 44) - 70 )) ||
				((temp_window <= (ulTCPMSS * 4) + 70) && (temp_window >= (ulTCPMSS * 4) - 70 )) ||
				((temp_window <= (ulTCPMSS * 6) + 70) && (temp_window >= (ulTCPMSS * 6) - 70 )) ||
				((temp_window <= (ulTCPMSS * 12) + 70) && (temp_window >= (ulTCPMSS * 12) - 70 )) ||
				((temp_window <= (ulTCPMSS * 16) + 70) && (temp_window >= (ulTCPMSS * 16) - 70 )) ||
				((temp_window <= (ulTCPMSS * 26) + 70) && (temp_window >= (ulTCPMSS * 26) - 70 )))
			{
				pScoreInfo->fValue+=90;       // Linux 1.2
			}

			if ((temp_window <= (44620 + 70)) && (temp_window >= (44620 - 70)))
			{
				pScoreInfo->fValue+=100;       // Linux 1.2
			}

			if ((temp_window <= (64512 + 70)) && (temp_window >= (64512 - 70)))
			{
				pScoreInfo->fValue+=110;       // Linux 1.2
			}

			if ((temp_window <= (64512 + 70)) && (temp_window >= (64512 - 70)))
			{
				pScoreInfo->fValue+=120;       // Linux 1.2
			}

			if (((temp_window <= (8192 + 70)) && (temp_window >= (8192 - 70))) ||
				(((temp_window <= (ulTCPMSS * 6) + 70) && (temp_window >= (ulTCPMSS * 6) - 70 ))))
			{
				pScoreInfo->fValue+=130;       // Linux 1.2
			}

			if (((temp_window <= (64512 + 70)) && (temp_window >= (64512 - 70))) ||
				((temp_window <= (ulTCPMSS * 44) + 70) && (temp_window >= (ulTCPMSS * 44) - 70 )))
			{
				pScoreInfo->fValue+=140;       // Linux 1.2
			}

			if (((temp_window <= 65535) && (temp_window >= (65535 - 70))) ||
				((temp_window <= (40320 + 70)) && (temp_window >= (40320 - 70))) ||
				((temp_window <= (32767 + 70)) && (temp_window >= (32767 - 70))) ||
				((temp_window <= (ulTCPMSS * 45) + 70) && (temp_window >= (ulTCPMSS * 45) - 70 )))
			{
				pScoreInfo->fValue+=150;       // Linux 1.2
			}

			if (((temp_window <= 65535) && (temp_window >= (65535 - 70))) ||
				((temp_window <= (8192 + 70)) && (temp_window >= (8192 - 70))) ||
				((temp_window <= (64512 + 70)) && (temp_window >= (64512 - 70))) ||
				((temp_window <= (32767 + 70)) && (temp_window >= (32767 - 70))) ||
				((temp_window <= (ulTCPMSS * 45) + 70) && (temp_window >= (ulTCPMSS * 45) - 70 )) ||
				((temp_window <= (ulTCPMSS * 44) + 70) && (temp_window >= (ulTCPMSS * 44) - 70 )) ||
				((temp_window <= (ulTCPMSS * 12) + 70) && (temp_window >= (ulTCPMSS * 12) - 70 )))
			{
				pScoreInfo->fValue+=160;       // Linux 1.2
			}

			if (((temp_window <= 65535) && (temp_window >= (65535 - 70))) ||
				((temp_window <= (32768 + 70)) && (temp_window >= (32768 - 70))) ||
				((temp_window <= (16384 + 70)) && (temp_window >= (16384 - 70))))
			{
				pScoreInfo->fValue+=170;       // Linux 1.2
			}

			if ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70)))
			{
				pScoreInfo->fValue+=180;       // Linux 1.2
			}


			if ((temp_window <= (16616 + 70)) & (temp_window >= (16616 - 70)))
			{
				pScoreInfo->fValue+=190;       // Linux 1.2
			}

			if (((temp_window <= (ulTCPMSS * 2) + 70) & (temp_window >= (ulTCPMSS * 2) - 70 )))
			{
				pScoreInfo->fValue+=200;       // Linux 1.2
			}

			if ((temp_window <= (32768 + 70)) & (temp_window >= (32768 - 70)))
			{
				pScoreInfo->fValue+=210;       // Linux 1.2
			}

			if (((temp_window <= (32768 + 70)) & (temp_window >= (32768 - 70))) |
					((temp_window <= 65535) & (temp_window >= (65535 - 70))))
			{
				pScoreInfo->fValue+=220;       // Linux 1.2
			}

			if ((temp_window <= (33304 + 70)) & (temp_window >= (33304 - 70)))
			{
				pScoreInfo->fValue+=230;       // Linux 1.2
			}

			pScoreInfo->xOSMatrix.bWindowSize = FTM_TRUE;
		}


		if(pScoreInfo->xOSMatrix.bSynPacketSize == FTM_FALSE)
		{
			if (htons(pIPHdr->tot_len) == 44)
			{
				pScoreInfo->fValue+=100;       // Linux 1.2
			}
			else if (htons(pIPHdr->tot_len) == 48)
			{
				pScoreInfo->fValue+=200;       // Linux 1.2
			}
			else if (htons(pIPHdr->tot_len) == 52)
			{
				pScoreInfo->fValue+=300;       // Linux 1.2
			}
			else if (htons(pIPHdr->tot_len) == 60)
			{
				pScoreInfo->fValue+=400;       // Linux 1.2
			}

			pScoreInfo->xOSMatrix.bSynPacketSize = FTM_TRUE;
		}


		if(pScoreInfo->xOSMatrix.bOptionsOrder == FTM_FALSE)
		{
			nOptionIndex = 0;
			bEndOfOptions = FTM_FALSE;

			FTM_CHAR	pOptions[20];

			memset(pOptions, 0, sizeof(pOptions));

			while((bEndOfOptions == FTM_FALSE) && (nOptionIndex < ulTCPOptLen))
			{
				switch(pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex])
				{
				case 0:
				default:
					bEndOfOptions = FTM_TRUE;
					break;

				case 1/*TCPOPT_NOP*/:
					strcat(pOptions, "N");
					nOptionIndex++;
					break;

				case 2/*TCPOPT_MAXSEG*/:
					strcat(pOptions, "M");
					nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
					break;

				case 3/*TCPOPT_WSCALE*/:
					strcat(pOptions, "W");
					nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
					break;

				case 4/*TCPOPT_SACKOK*/:
					strcat(pOptions, "S");
					nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
					break;

				case 8/*TCPOPT_TIMESTAMP*/:
					strcat(pOptions, "T");
					nOptionIndex += pPacket[ETH_HLEN + ulIPHdrLen + 20 + nOptionIndex + 1];
					break;
				}
			}

			if (strcmp(pOptions, "M") == 0)
			{
				pScoreInfo->fValue+=1000;       // Linux 1.2
			}
			else if (strcmp(pOptions, "MSTNW") == 0)
			{
				pScoreInfo->fValue+=2000;       // Linux 1.2
			}

			if (strcmp(pOptions, "MNWNNTNNS") == 0)
			{
				pScoreInfo->fValue+=3000;       // Linux 1.2
			}

			if ((strcmp(pOptions, "MNNS") == 0) | (strcmp(pOptions, "MNWNNTNNS") == 0) | (strcmp(pOptions, "MNWNNS") == 0))
			{
				pScoreInfo->fValue+=4000;       // Linux 1.2
			}

			if (strcmp(pOptions, "MNNS") == 0)
			{
				pScoreInfo->fValue+=5000;       // Linux 1.2
			}
			if ((strcmp(pOptions, "MNWNNS") == 0) | (strcmp(pOptions, "MNNS") == 0))
			{
				pScoreInfo->fValue+=6000;       // Linux 1.2
			}

			if (strcmp(pOptions, "MWNNNS") == 0)
			{
				pScoreInfo->fValue+=7000;       // Linux 1.2
			}


			if (strcmp(pOptions, "MW") == 0)
			{
				pScoreInfo->fValue+=8000;       // Linux 1.2
			}

			if (strcmp(pOptions, "MNNN") == 0)
			{
				pScoreInfo->fValue+=9000;       // Linux 1.2
			}

			if (strcmp(pOptions, "MWN") == 0)
			{
				pScoreInfo->fValue+=10000;       // Linux 1.2
			}

			if (strcmp(pOptions, "MNNNN") == 0)
			{
				pScoreInfo->fValue+=11000;       // Linux 1.2
			}

			if (strcmp(pOptions, "MNWNNT") == 0)
			{
				pScoreInfo->fValue+=12000;       // Linux 1.2
			}

			pScoreInfo->xOSMatrix.bOptionsOrder = FTM_TRUE;
		}


		if (pScoreInfo->xOSMatrix.bIPID == FTM_FALSE)
		{
			if (htons(pIPHdr->id) != 0)       //IPID != 0
			{
				pScoreInfo->fValue+=10000;       // Linux 1.2
			}
			else if (htons(pIPHdr->id) == 0)  //IPID = 0
			{
				pScoreInfo->fValue+=20000;       // Linux 1.2
			}

			pScoreInfo->xOSMatrix.bIPID = FTM_TRUE;
		}
	}

}//end processing_tCP


