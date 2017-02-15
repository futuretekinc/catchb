#include <stdlib.h>
#include <string.h>
#include "catchb_db.h"
#include "catchb_trace.h"

CATCHB_RET	CATCHB_DB_getElemetCount
(
	CATCHB_DB_PTR		pDB,
	CATCHB_CHAR_PTR		pTableName,
	CATCHB_UINT32_PTR	pCount
);

CATCHB_RET	CATCHB_DB_create
(
	CATCHB_DB_PTR _PTR_ ppDB
)
{
	ASSERT(ppDB != NULL);

	CATCHB_RET		xRet=CATCHB_RET_OK;
	CATCHB_DB_PTR	pDB;

	pDB = (CATCHB_DB_PTR)malloc(sizeof(CATCHB_DB));
	if (pDB == NULL)
	{
		xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create DB!\n");
	}
	else
	{
		*ppDB = pDB;	
	}

	return	xRet;
}

CATCHB_RET	CATCHB_DB_destroy
(
	CATCHB_DB_PTR _PTR_ ppDB
)
{
	ASSERT(ppDB != NULL);
	ASSERT(*ppDB != NULL);

	CATCHB_DB_close(*ppDB);

	free(*ppDB);
	*ppDB = NULL;

	return	CATCHB_RET_OK;
}

CATCHB_RET	CATCHB_DB_open
(
	CATCHB_DB_PTR 	pDB, 
	CATCHB_CHAR_PTR pFileName
)
{
	ASSERT(pDB != NULL);
	CATCHB_RET	xRet = CATCHB_RET_OK;

	if (pDB->pSQLite3 != NULL)
	{
		xRet = CATCHB_RET_DB_ALREADY_OPENED;	
		ERROR(xRet, "DB already opened!\n");	
	}
	else
	{
		if (sqlite3_open(pFileName, &pDB->pSQLite3) != 0)
		{
			xRet = CATCHB_RET_DB_OPEN_FAILED;	
			ERROR(xRet, "DB open failed!\n");
		}
	}

	return	xRet;
}

CATCHB_RET	CATCHB_DB_close
(
	CATCHB_DB_PTR 	pDB
)
{
	ASSERT(pDB != NULL);
	CATCHB_RET	xRet = CATCHB_RET_OK;

	if (pDB->pSQLite3 != NULL)
	{
		sqlite3_close(pDB->pSQLite3);	
		pDB->pSQLite3 = NULL;	
	}

	return	xRet;
}

CATCHB_RET	CATCHB_DB_getAlarmMailInfoCount
(
	CATCHB_DB_PTR pDB, 
	CATCHB_UINT32_PTR pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);

	CATCHB_RET	xRet = CATCHB_RET_OK;
	
	xRet =CATCHB_DB_getElemetCount(pDB, "tb_alarm_mail", pCount);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tb_alarm_mail!\n");
	}

	return	xRet;
}

CATCHB_RET	CATCHB_DB_getAlarmMailInfoList
(
	CATCHB_DB_PTR pDB, 
	CATCHB_ALARM_MAIL_INFO_PTR pAlarmMailList, 
	CATCHB_UINT32 ulMaxCount, 
	CATCHB_UINT32_PTR pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);

	CATCHB_RET	xRet = CATCHB_RET_OK;

	*pCount = 0;

	return	xRet;
}

typedef	struct	CATCHB_DB_COUNT_PARAMS_STRUCT
{
	CATCHB_UINT32	ulCount;
}	CATCHB_DB_COUNT_PARAMS, _PTR_ CATCHB_DB_COUNT_PARAMS_PTR;

CATCHB_INT	CATCHB_DB_getElementCountCB
(
	CATCHB_VOID_PTR    pData, 
	CATCHB_INT         nArgc, 
	CATCHB_CHAR_PTR _PTR_ pArgv, 
	CATCHB_CHAR_PTR _PTR_ pColName
)
{
	CATCHB_DB_COUNT_PARAMS_PTR pParams = (CATCHB_DB_COUNT_PARAMS_PTR)pData;

	if (nArgc != 0)
	{    
		pParams->ulCount = atoi(pArgv[0]);
	}    

	return  0;
}

CATCHB_RET	CATCHB_DB_getElemetCount
(
	CATCHB_DB_PTR		pDB,
	CATCHB_CHAR_PTR		pTableName,
	CATCHB_UINT32_PTR	pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pTableName != NULL);
	ASSERT(pCount != NULL);

	CATCHB_RET	xRet = CATCHB_RET_OK;
	CATCHB_CHAR	pQuery[CATCHB_DB_QUERY_LEN+1];
	CATCHB_DB_COUNT_PARAMS	xParams;
	CATCHB_CHAR_PTR	pErrorMsg = NULL;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "SELECT COUNT(*) FROM %s", pTableName);

	if (sqlite3_exec(pDB->pSQLite3, pQuery, CATCHB_DB_getElementCountCB, &xParams, &pErrorMsg) != 0)
	{
		sqlite3_free(pErrorMsg);
		*pCount = 0;
	}
	else
	{
		*pCount = xParams.ulCount;
	}


	return	xRet;
}

