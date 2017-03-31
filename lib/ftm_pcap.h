#ifndef	FTM_PCAP_H_
#define	FTM_PCAP_H_

#include "ftm_types.h"
#include <pcap/pcap.h>

typedef	struct FTM_PCAP_STRUCT
{
	pcap_t _PTR_	pPCD;		// packet capture descriptor	

	FTM_CHAR_PTR	pDev;
	bpf_u_int32		xNet;
	bpf_u_int32		xMask;
	FTM_CHAR_PTR	pOptions;
	FTM_CHAR		pErrorBuff[PCAP_ERRBUF_SIZE];

	FTM_UINT32		ulSourceIP;
	FTM_UINT32		ulDestIP;
	FTM_UINT16_PTR	pPorts;
	FTM_UINT32		ulPortCount;

	FTM_UINT32		ulCaptureCount;
	FTM_UINT32		ulMaxCaptureCount;
	FTM_UINT8_PTR _PTR_	ppCapturePackets;
}	FTM_PCAP, _PTR_ FTM_PCAP_PTR;

FTM_RET	FTM_PCAP_create
(
	FTM_PCAP_PTR _PTR_ ppPCAP
);

FTM_RET	FTM_PCAP_destroy
(
	FTM_PCAP_PTR _PTR_ ppPCAP
);

FTM_RET	FTM_PCAP_open
(
	FTM_PCAP_PTR	pPCAP
);

FTM_RET	FTM_PCAP_close
(
	FTM_PCAP_PTR	pPCAP
);

FTM_RET	FTM_PCAP_start
(
	FTM_PCAP_PTR	pPCAP,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_PCAP_stop
(
	FTM_PCAP_PTR	pPCAP
);

FTM_RET	FTM_PCAP_setFilter
(
	FTM_PCAP_PTR	pPCAP,
	FTM_CHAR_PTR	pOptions
);

FTM_RET	FTM_PCAP_setFilterIP
(
	FTM_PCAP_PTR	pPCAP,
	FTM_UINT32		ulSIP,
	FTM_UINT32		ulDIP
);

FTM_RET	FTM_PCAP_setFilterPorts
(
	FTM_PCAP_PTR	pPCAP,
	FTM_UINT16_PTR	pPorts,
	FTM_UINT32		ulCount
);

#endif
