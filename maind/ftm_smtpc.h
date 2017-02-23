/*
 * libsmtpc.h - libsmtpc header file.
 *
 * (C) 2006 Carlos Eduardo Medaglia Dyonisio <cadu(a)nerdfeliz.com>
 *          http://nerdfeliz.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version
 * 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef FTM_SMTPC_H_
#define FTM_SMTPC_H_

#include "ftm_types.h"

typedef	struct	FTM_SMTP_CLIENT_STRUCT
{
	FTM_INT			xSock;
	FTM_CHAR		pHostName[FTM_HOST_NAME_LEN+1];
	FTM_UINT16		usPort;
	FTM_INT			nCode;
} FTM_SMTPC, _PTR_ FTM_SMTPC_PTR;

FTM_RET	FTM_SMTPC_create
(
	FTM_CHAR_PTR	pHostName,
	FTM_UINT16		usPort,
	FTM_SMTPC_PTR _PTR_ ppSMTPC
);

FTM_RET	FTM_SMTPC_destroy
(
	FTM_SMTPC_PTR _PTR_ ppSMTPC
);

FTM_RET	FTM_SMTPC_connect
(
	FTM_SMTPC_PTR	pSMTPC
);

FTM_RET	FTM_SMTPC_disconnect
(
	FTM_SMTPC_PTR	pSMTPC
);

FTM_RET	FTM_SMTPC_sendAndReceive
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pData

);	

FTM_RET	FTM_SMTPC_sendGreeting
(
	FTM_SMTPC_PTR	pSMTPC
);

FTM_RET	FTM_SMTPC_cmdAndArg
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pCmd,
	FTM_CHAR_PTR	pArg
);

FTM_RET	FTM_SMTPC_sendTo
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pTo
);

FTM_RET	FTM_SMTPC_sendFrom
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pFrom
);

FTM_RET	FTM_SMTPC_sendAuth
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pUserID,
	FTM_CHAR_PTR	pPasswd
);

FTM_RET	FTM_SMTPC_startBody
(
	FTM_SMTPC_PTR	pSMTPC
);

FTM_RET	FTM_SMTPC_sendRawBody
(
	FTM_SMTPC_PTR	pSMTPC,
	FTM_CHAR_PTR	pData
);

FTM_RET	FTM_SMTPC_endBody
(
	FTM_SMTPC_PTR	pSMTPC
);

#endif /* _LIBSMTPC_H */
