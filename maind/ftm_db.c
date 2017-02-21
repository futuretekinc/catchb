#include <stdlib.h>
#include <string.h>
#include "ftm_mem.h"
#include "ftm_db.h"
#include "ftm_trace.h"

typedef	FTM_INT	(*FTM_DB_GET_ELEMENT_LIST_CALLBACK)(FTM_VOID_PTR pData, FTM_INT nArgc, FTM_CHAR_PTR _PTR_ ppArgv, FTM_CHAR_PTR _PTR_ ppColumnName);

FTM_RET	FTM_DB_getElementCount
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR		pTableName,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_getElementList
(
	FTM_DB_PTR pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_DB_create
(
	FTM_DB_PTR _PTR_ ppDB
)
{
	ASSERT(ppDB != NULL);

	FTM_RET		xRet=FTM_RET_OK;
	FTM_DB_PTR	pDB;

	pDB = (FTM_DB_PTR)FTM_MEM_malloc(sizeof(FTM_DB));
	if (pDB == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create DB!");
	}
	else
	{
		strcpy(pDB->pCCTVTableName, "tb_cctv");
		strcpy(pDB->pAlarmTableName, "tb_alarm");
		strcpy(pDB->pLogTableName, "tb_log");
		strcpy(pDB->pDenyTableName, "tb_deny");
		strcpy(pDB->pSwitchTableName, "tb_switch");
		*ppDB = pDB;	
	}

	return	xRet;
}

FTM_RET	FTM_DB_destroy
(
	FTM_DB_PTR _PTR_ ppDB
)
{
	ASSERT(ppDB != NULL);
	ASSERT(*ppDB != NULL);

	FTM_DB_close(*ppDB);

	FTM_MEM_free(*ppDB);
	*ppDB = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_DB_open
(
	FTM_DB_PTR 	pDB, 
	FTM_CHAR_PTR pFileName
)
{
	ASSERT(pDB != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pDB->pSQLite3 != NULL)
	{
		xRet = FTM_RET_DB_ALREADY_OPENED;	
		ERROR(xRet, "DB already opened!");	
	}
	else
	{
		if (sqlite3_open(pFileName, &pDB->pSQLite3) != 0)
		{
			xRet = FTM_RET_DB_OPEN_FAILED;	
			ERROR(xRet, "DB open failed!");
		}
		else
		{
			TRACE("The database[%s] opened successfully.", pFileName);
		}
	}

	return	xRet;
}

FTM_RET	FTM_DB_close
(
	FTM_DB_PTR 	pDB
)
{
	ASSERT(pDB != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pDB->pSQLite3 != NULL)
	{
		sqlite3_close(pDB->pSQLite3);	
		pDB->pSQLite3 = NULL;	
	}

	return	xRet;
}

FTM_RET	FTM_DB_createTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pTableName,
	FTM_CHAR_PTR	pFields
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_BOOL	bExist = FTM_FALSE;

	FTM_DB_isExistTable(pDB, pTableName, &bExist);
	if (bExist)
	{
		xRet = FTM_RET_DB_ALREADY_EXIST;	
		ERROR(xRet, "The table[%s] exists!", pTableName);
	}
	else
	{
		FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
		FTM_CHAR_PTR	pErrorMsg;

		memset(pQuery, 0, sizeof(pQuery));

		snprintf(pQuery, sizeof(pQuery) - 1, "CREATE TABLE %s (%s)", pTableName, pFields);
		if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
		{
			xRet = FTM_RET_DB_EXEC_ERROR;
			ERROR(xRet, "Failed to execute query!");
			sqlite3_free(pErrorMsg);
		}
	}
		
	return	xRet;
}

typedef struct FTM_DB_IS_EXIST_TABLE_PARAMS_STRUCT
{
	FTM_BOOL        bExist;
	FTM_CHAR_PTR    pName;
} 	FTM_DB_IS_EXIST_TABLE_PARAMS, _PTR_ FTM_DB_IS_EXIST_TABLE_PARAMS_PTR;

FTM_INT FTM_DB_isExistTableCB
(
	FTM_VOID_PTR    pData, 
 	FTM_INT         nArgc, 
 	FTM_CHAR_PTR _PTR_  pArgv, 
 	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTM_INT	i;
	FTM_DB_IS_EXIST_TABLE_PARAMS_PTR pParams = (FTM_DB_IS_EXIST_TABLE_PARAMS_PTR)pData;

	if (nArgc != 0)
	{    
		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(pParams->pName, pArgv[i]) == 0)
			{    
				pParams->bExist = FTM_TRUE;
				break;
			}    
		}    
	}    

	return  0;
}

FTM_RET FTM_DB_isExistTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR    pTableName, 
	FTM_BOOL_PTR    pExist
)
{
	FTM_RET	xRet = FTM_RET_OK;

	FTM_DB_IS_EXIST_TABLE_PARAMS xParams = { .bExist = FTM_FALSE, .pName = pTableName};
	FTM_CHAR_PTR    pQuery = "select name from sqlite_master where type='table' order by name";
	FTM_CHAR_PTR    pErrMsg = NULL;

	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_isExistTableCB, &xParams, &pErrMsg) != 0)
	{    
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrMsg);
	}    
	else
	{
		*pExist = xParams.bExist;
	}

	return	xRet;
}

typedef	struct	FTM_DB_COUNT_PARAMS_STRUCT
{
	FTM_UINT32	ulCount;
}	FTM_DB_COUNT_PARAMS, _PTR_ FTM_DB_COUNT_PARAMS_PTR;

FTM_INT	FTM_DB_getElementCountCB
(
	FTM_VOID_PTR    pData, 
	FTM_INT         nArgc, 
	FTM_CHAR_PTR _PTR_ pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTM_DB_COUNT_PARAMS_PTR pParams = (FTM_DB_COUNT_PARAMS_PTR)pData;

	if (nArgc != 0)
	{    
		pParams->ulCount = atoi(pArgv[0]);
	}    

	return  0;
}

FTM_RET	FTM_DB_getElementCount
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR		pTableName,
	FTM_UINT32_PTR	pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pTableName != NULL);
	ASSERT(pCount != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_DB_COUNT_PARAMS	xParams;
	FTM_CHAR_PTR	pErrorMsg = NULL;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "SELECT COUNT(*) FROM %s", pTableName);

	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getElementCountCB, &xParams, &pErrorMsg) != 0)
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

FTM_RET	FTM_DB_getElementList
(
	FTM_DB_PTR pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pDB != NULL);
	ASSERT(pTableName != NULL);
	ASSERT(pData != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));

	snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s", pTableName);

	if (sqlite3_exec(pDB->pSQLite3, pQuery, fCallback, pData, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////

FTM_RET	FTM_DB_CCTV_createTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pCCTVTableName, "ID TEXT PRIMARY KEY, IP TEXT, COMMENT TEXT, TIME TEXT, HASH TEXT, STAT INT");
}

FTM_RET	FTM_DB_CCTV_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pCCTVTableName, pExist);
}

FTM_RET	FTM_DB_CCTV_count
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
)
{
	return	FTM_DB_getElementCount(pDB, pDB->pCCTVTableName, pCount);
}

FTM_RET	FTM_DB_CCTV_insert
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pComment,
	FTM_CHAR_PTR	pTime
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s(ID, IP, COMMENT, TIME, HASH, STAT) VALUES('%s', '%s', '%s', '%s', '', 0);", 
		pDB->pCCTVTableName, pID, pIP, (pComment != NULL)?pComment:"", pTime);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to insert item to DB!");
		sqlite3_free(pErrorMsg);
	}
	else
	{
		TRACE("DB[%s] inserted(%s, %s, %s, %s, '')", pDB->pCCTVTableName, pID, pIP, (pComment != NULL)?pComment:"", pTime);
	}

	return	xRet;
}

FTM_RET	FTM_DB_CCTV_update
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pComment,
	FTM_CHAR_PTR	pHash
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32	ulQueryLen = 0;
	FTM_BOOL	bNew = FTM_TRUE;
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	ulQueryLen = snprintf(pQuery, sizeof(pQuery) - ulQueryLen, "UPDATE %s SET", pDB->pCCTVTableName);

	if (pComment != NULL)
	{
		if (bNew)
		{
			ulQueryLen +=snprintf(pQuery, sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(pQuery, sizeof(pQuery) - ulQueryLen, " COMMENT = '%s'", pComment);
	}

	if (pHash != NULL)
	{
		if (bNew)
		{
			ulQueryLen +=snprintf(pQuery, sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(pQuery, sizeof(pQuery) - ulQueryLen, " HASH = '%s'", pHash);
	}

	
	ulQueryLen +=snprintf(pQuery, sizeof(pQuery) - ulQueryLen, " WHERE ID = '%s';", pID);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

FTM_RET	FTM_DB_CCTV_delete
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE ID = '%s'", pDB->pCCTVTableName, pID);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

typedef struct
{
	FTM_UINT32			ulMaxCount;
	FTM_UINT32			ulCount;
	FTM_CCTV_CONFIG_PTR	pElements;
}   FTM_CCTV_CONFIG_GET_LIST_PARAMS, _PTR_ FTM_CCTV_CONFIG_GET_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_CCTV_getListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT		nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_CCTV_CONFIG_GET_LIST_PARAMS_PTR	pParams = (FTM_CCTV_CONFIG_GET_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "COMMENT") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pComment, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "TIME") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pTime, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "HASH") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pHash, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "STAT") == 0)
			{
				pParams->pElements[pParams->ulCount].xStat = atoi(ppArgv[i]);
			}
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}

FTM_RET	FTM_DB_CCTV_getUsingIP
(
	FTM_DB_PTR			pDB,
	FTM_CHAR_PTR		pIP,
	FTM_CCTV_CONFIG_PTR	pCCTV
)
{
	ASSERT(pDB != NULL);
	ASSERT(pIP != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;
    FTM_CCTV_CONFIG_GET_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= 1;
	xParams.ulCount 	= 0;
	xParams.pElements	= pCCTV;


	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s WHERE IP = %s", pDB->pCCTVTableName, pIP);

	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_CCTV_getListCB, (FTM_VOID_PTR)&xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	else if (xParams.ulCount == 0)
	{
		xRet = FTM_RET_OBJECT_NOT_FOUND;
		ERROR(xRet, "Object not found!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_CCTV_getList
(
	FTM_DB_PTR			pDB,
	FTM_CCTV_CONFIG_PTR	pElements,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CCTV_CONFIG_GET_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount 	= 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pCCTVTableName, FTM_DB_CCTV_getListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

FTM_RET	FTM_DB_CCTV_hashUpdated
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	ASSERT(pHash != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "UPDATE %s SET HASH='%s' WHERE ID = '%s'", pDB->pCCTVTableName, pHash, pID);

	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_CCTV_setStat
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_STAT	xStat
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "UPDATE %s SET STAT=%d WHERE ID = '%s'", pDB->pCCTVTableName, xStat, pID);

	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_ALARM_createTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pAlarmTableName, "ID TEXT PRIMARY KEY, MESSAGE TEXT, EMAIL TEXT");
}

FTM_RET	FTM_DB_ALARM_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pAlarmTableName, pExist);
}

FTM_RET	FTM_DB_ALARM_count
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	
	xRet =FTM_DB_getElementCount(pDB, pDB->pAlarmTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tv_alarm_info!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_ALARM_insert
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pMail
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s(ID, MESSAGE, EMAIL) VALUES('%s', '%s', '%s');", pDB->pAlarmTableName, pID, "", pMail);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

FTM_RET	FTM_DB_ALARM_delete
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE ID = '%s'", pDB->pAlarmTableName, pID);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

typedef struct
{
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount;
	FTM_ALARM_PTR	pElements;
}   FTM_ALARM_GET_LIST_PARAMS, _PTR_ FTM_ALARM_GET_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_ALARM_getListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT		nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_ALARM_GET_LIST_PARAMS_PTR	pParams = (FTM_ALARM_GET_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "MESSAGE") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pMessage, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "EMAIL") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pEmail, ppArgv[i]);
			}    
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}


FTM_RET	FTM_DB_ALARM_getList
(
	FTM_DB_PTR		pDB,
	FTM_ALARM_PTR	pElements,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_ALARM_GET_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount =	 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pAlarmTableName, FTM_DB_ALARM_getListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}


////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_LOG_createTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pLogTableName, "TIME TEXT, ID TEXT, IP TEXT, HASH TEXT, STATUS TEXT, LOG TEXT");
}

FTM_RET	FTM_DB_LOG_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pLogTableName, pExist);
}

FTM_RET	FTM_DB_LOG_count
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	
	xRet =FTM_DB_getElementCount(pDB, pDB->pLogTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tv_log_info!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_LOG_insert
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pHash,
	FTM_CHAR_PTR	pLog,
	FTM_CHAR_PTR	pTime,
	FTM_INT			nStatus
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s (TIME, ID, IP, HASH, STATUS, LOG) VALUES('%s', '%s', '%s', '%s', '%s', '%s');", 
		pDB->pLogTableName, pTime, pID, pIP, pHash, (nStatus == 0)?"UNUSED":((nStatus == 1)?"NORMAL":"ABORMAL"), pLog);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

FTM_RET	FTM_DB_LOG_delete
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE ID = '%s'", pDB->pLogTableName, pID);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

typedef struct
{
	FTM_UINT32	ulMaxCount;
	FTM_UINT32	ulCount;
	FTM_LOG_PTR	pElements;
}   FTM_LOG_GET_LIST_PARAMS, _PTR_ FTM_LOG_GET_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_LOG_getListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT		nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_LOG_GET_LIST_PARAMS_PTR	pParams = (FTM_LOG_GET_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "LOG") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pLog, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "TIME") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pTime, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "STATUS") == 0)
			{
				pParams->pElements[pParams->ulCount].nStatus = atoi(ppArgv[i]);
			}
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}


FTM_RET	FTM_DB_LOG_getList
(
	FTM_DB_PTR		pDB,
	FTM_LOG_PTR		pElements,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements!= NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_LOG_GET_LIST_PARAMS	xParams;

	xParams.ulMaxCount = ulMaxCount;
	xParams.ulCount = 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pLogTableName, FTM_DB_LOG_getListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_SWITCH_createTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pSwitchTableName, "ID TEXT PRIMARY KEY, USER TEXT, PASSWD TEXT, NAME TEXT, IP TEXT, COMMENT TEXT");
}

FTM_RET	FTM_DB_SWITCH_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pSwitchTableName, pExist);
}

FTM_RET	FTM_DB_SWITCH_count
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	
	xRet =FTM_DB_getElementCount(pDB, pDB->pSwitchTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tv_switch_info!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_SWITCH_insert
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pUser,
	FTM_CHAR_PTR	pPasswd,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pComment	
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s (ID, USER, PASSWD, NAME, IP, COMMENT) VALUES('%s', '%s', '%s', '%s', '%s', '%s');", 
		pDB->pSwitchTableName, pID, (pUser)?pUser:"", (pPasswd)?pPasswd:"", (pName)?pName:"", (pIP)?pIP:"", (pComment)?pComment:"");
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

FTM_RET	FTM_DB_SWITCH_delete
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE ID = '%s'", pDB->pSwitchTableName, pID);
	
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

typedef struct
{
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount;
	FTM_SWITCH_PTR	pElements;
}   FTM_SWITCH_GET_LIST_PARAMS, _PTR_ FTM_SWITCH_GET_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_SWITCH_getListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT			nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_SWITCH_GET_LIST_PARAMS_PTR	pParams = (FTM_SWITCH_GET_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "USER") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pUser, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "PASSWD") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pPasswd, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "NAME") == 0)
			{
				strcpy(pParams->pElements[pParams->ulCount].pName, ppArgv[i]);
			}
			else if (strcmp(ppColName[i], "COMMENT") == 0)
			{
				strcpy(pParams->pElements[pParams->ulCount].pComment, ppArgv[i]);
			}
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}


FTM_RET	FTM_DB_SWITCH_getList
(
	FTM_DB_PTR		pDB,
	FTM_SWITCH_PTR	pElements,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_SWITCH_GET_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount 	= 0;
	xParams.pElements	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pSwitchTableName, FTM_DB_SWITCH_getListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_DENY_createTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pDenyTableName, "IP TEXT PRIMARY KEY, INDEX INT, SWITCH TEXT");
}

FTM_RET	FTM_DB_DENY_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pDenyTableName, pExist);
}

FTM_RET	FTM_DB_DENY_count
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	
	xRet =FTM_DB_getElementCount(pDB, pDB->pDenyTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tv_deny_info!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_DENY_insert
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pIP,
	FTM_UINT32		nIndex,
	FTM_CHAR_PTR	pSwitchID
)
{
	ASSERT(pDB != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pSwitchID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s (IP, INDEX, SWITCH) VALUES('%s', %d, '%s');", pDB->pDenyTableName, pIP, nIndex, pSwitchID);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}

FTM_RET	FTM_DB_DENY_delete
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pIP
)
{
	ASSERT(pDB != NULL);
	ASSERT(pIP != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE IP = '%s'", pDB->pDenyTableName, pIP);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}

	return	xRet;
}


typedef struct
{
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount;
	FTM_DENY_PTR	pElements;
}   FTM_DENY_GET_LIST_PARAMS, _PTR_ FTM_DENY_GET_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_DENY_getListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT			nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_DENY_GET_LIST_PARAMS_PTR	pParams = (FTM_DENY_GET_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "SWITCH") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pSwitchID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "INDEX") == 0)
			{    
				pParams->pElements[pParams->ulCount].nIndex = atoi(ppArgv[i]);
			}    
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}


FTM_RET	FTM_DB_DENY_get
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pIP,
	FTM_DENY_PTR	pDeny
)
{
	ASSERT(pDB != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pDeny != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;
    FTM_DENY_GET_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= 1;
	xParams.ulCount 	= 0;
	xParams.pElements	= pDeny;


	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM ft_deny_info WHERE IP = %s", pIP);

	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_DENY_getListCB, (FTM_VOID_PTR)&xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	else if (xParams.ulCount == 0)
	{
		xRet = FTM_RET_OBJECT_NOT_FOUND;
		ERROR(xRet, "Object not found!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_DENY_getList
(
	FTM_DB_PTR		pDB,
	FTM_DENY_PTR	pElements,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_DENY_GET_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount 	= 0;
	xParams.pElements	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pDenyTableName, FTM_DB_DENY_getListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

