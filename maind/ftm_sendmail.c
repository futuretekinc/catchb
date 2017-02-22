#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_time.h"

/* This is a simple example showing how to send mail using libcurl's SMTP
 * capabilities. For an example of using the multi interface please see
 * smtp-multi.c.
 *
 * Note that this example requires libcurl 7.20.0 or above.
 */ 

typedef	struct FTM_SMTP_PARAMS_STRUCT
{
	FTM_UINT32		ulPayload;
	FTM_UINT32		ulSentPayload;
	FTM_CHAR		pPayload[];
}	FTM_SMTP_PARAMS, _PTR_ FTM_SMTP_PARAMS_PTR;

FTM_RET	FTM_SMTP_makePayload
(
	FTM_CHAR_PTR	pTo,
	FTM_CHAR_PTR	pFrom,
	FTM_CHAR_PTR	pSubject,
	FTM_CHAR_PTR	pMessage,
	FTM_SMTP_PARAMS_PTR _PTR_ ppParams
)
{
	ASSERT(pTo != NULL);
	ASSERT(pFrom != NULL);
	ASSERT(pSubject != NULL);
	ASSERT(pMessage != NULL);
	ASSERT(ppParams != NULL);

	FTM_RET	xRet;
	FTM_SMTP_PARAMS_PTR	pParams;
	FTM_UINT32			ulMaxPayload;

	ulMaxPayload = sizeof(FTM_SMTP_PARAMS) + 1024 + strlen(pTo) + strlen(pFrom) + strlen(pSubject) + strlen(pMessage);
	pParams = (FTM_SMTP_PARAMS_PTR)FTM_MEM_malloc(ulMaxPayload);
	if (pParams == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to alloc buffer!");
		return	xRet;
	}

	pParams->ulPayload += snprintf(&pParams->pPayload[pParams->ulPayload], ulMaxPayload - pParams->ulPayload - 1, "Date : %s\r\n", FTM_TIME_printfCurrent(NULL));
	pParams->ulPayload += snprintf(&pParams->pPayload[pParams->ulPayload], ulMaxPayload - pParams->ulPayload - 1, "To : %s\r\n", pTo);
	pParams->ulPayload += snprintf(&pParams->pPayload[pParams->ulPayload], ulMaxPayload - pParams->ulPayload - 1, "From : %s\r\n", pFrom);
	pParams->ulPayload += snprintf(&pParams->pPayload[pParams->ulPayload], ulMaxPayload - pParams->ulPayload - 1, "Subject : %s\r\n", pSubject);
	pParams->ulPayload += snprintf(&pParams->pPayload[pParams->ulPayload], ulMaxPayload - pParams->ulPayload - 1, "\r\n");
	pParams->ulPayload += snprintf(&pParams->pPayload[pParams->ulPayload], ulMaxPayload - pParams->ulPayload - 1, "%s\r\n", pMessage);
	//pParams->ulPayload += snprintf(&pParams->pPayload[pParams->ulPayload], ulMaxPayload - pParams->ulPayload - 1, "It could be a lot of lines, could be MIME encoded, whatever.\r\nCheck RFC5322.\r\n");

	*ppParams = pParams;

	return	FTM_RET_OK;
}

static 
size_t FTM_SMTP_payload
(
	FTM_VOID_PTR	pBuff, 
	size_t 			nSize, 
	size_t 			nMemb, 
	FTM_VOID_PTR	pData
)
{
	FTM_SMTP_PARAMS_PTR	pParams = (FTM_SMTP_PARAMS_PTR)pData;

	if((nSize == 0) || (nMemb == 0) || ((nSize*nMemb) < 1)) 
	{
		return 0;
	}

	if (pParams->ulSentPayload < pParams->ulPayload)
	{
		if (pParams->ulSentPayload + nSize > pParams->ulPayload)
		{
			nSize = pParams->ulPayload - pParams->ulSentPayload;
		}

		memcpy(pBuff, &pParams->pPayload[pParams->ulSentPayload], nSize);
		pParams->ulSentPayload += nSize;

		return nSize;
	}

	return 0;
}

FTM_RET	FTM_SMTP_send
(
	FTM_CHAR_PTR	pServer,
	FTM_CHAR_PTR	pTo,
	FTM_CHAR_PTR	pFrom,
	FTM_CHAR_PTR	pSubject,
	FTM_CHAR_PTR	pMessage
)
{
	CURL *pCURL;
	CURLcode res = CURLE_OK;
	struct curl_slist *recipients = NULL;
	FTM_SMTP_PARAMS_PTR	pParams;
	FTM_RET	xRet;
	FTM_CHAR	pBuff[1024];


	xRet = FTM_SMTP_makePayload(pTo, pFrom, pSubject, pMessage, &pParams);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to make payload!");
		return	xRet;	
	}

	pCURL = curl_easy_init();
	if(pCURL == NULL) 
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to initialize curl easy!");
		goto finished;
	}

	/* This is the URL for your mailserver */ 
	sprintf(pBuff, "smtp://%s", pServer);
	curl_easy_setopt(pCURL, CURLOPT_URL, pBuff);

	/* Note that this option isn't strictly required, omitting it will result
	 * in libcurl sending the MAIL FROM command with empty sender data. All
	 * autoresponses should have an empty reverse-path, and should be directed
	 * to the address in the reverse-path which triggered them. Otherwise,
	 * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
	 * details.
	 */ 
	curl_easy_setopt(pCURL, CURLOPT_MAIL_FROM, pFrom);

	/* Add two recipients, in this particular case they correspond to the
	 * To: and Cc: addressees in the header, but they could be any kind of
	 * recipient. */ 
	recipients = curl_slist_append(recipients, pTo);
	curl_easy_setopt(pCURL, CURLOPT_MAIL_RCPT, recipients);

	/* We're using a callback function to specify the payload (the headers and
	 * body of the message). You could just use the CURLOPT_READDATA option to
	 * specify a FILE pointer to read from. */ 
	curl_easy_setopt(pCURL, CURLOPT_READFUNCTION, FTM_SMTP_payload);
	curl_easy_setopt(pCURL, CURLOPT_READDATA, pParams);
	curl_easy_setopt(pCURL, CURLOPT_UPLOAD, 1L);

	/* Send the message */ 
	res = curl_easy_perform(pCURL);

	/* Check for errors */ 
	if(res != CURLE_OK)
	{
		xRet = FTM_RET_SEND_MAIL_ERROR;
		ERROR(xRet, "Send mail error[%s]", curl_easy_strerror(res));
		TRACE("%s", pBuff);
		TRACE("%s", pParams->pPayload);
	}

	/* Free the list of recipients */ 
	curl_slist_free_all(recipients);

	/* curl won't send the QUIT command until you call cleanup, so you should
	 * be able to re-use this connection for additional messages (setting
	 * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
	 * curl_easy_perform() again. It may not be a good idea to keep the
	 * connection open for a very long time though (more than a few minutes
	 * may result in the server timing out the connection), and you do want to
	 * clean up in the end.
	 */ 
	curl_easy_cleanup(pCURL);

finished:
	if (pParams != NULL)
	{
		FTM_MEM_free(pParams);	
	}
	
	return	xRet;
}
