#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ftm_mem.h"
#include "ftm_db.h"
#include "ftm_trace.h"

#undef	__MODULE__
#define	__MODULE__	"database"

typedef	FTM_INT	(*FTM_DB_GET_ELEMENT_LIST_CALLBACK)(FTM_VOID_PTR pData, FTM_INT nArgc, FTM_CHAR_PTR _PTR_ ppArgv, FTM_CHAR_PTR _PTR_ ppColumnName);

FTM_RET	FTM_DB_getElementCount
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pTableName,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_getElementList
(
	FTM_DB_PTR 		pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_DB_getElementListOrderByTime
(
	FTM_DB_PTR 		pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_DB_getElementListOfTimePeriod
(
	FTM_DB_PTR 		pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulCount,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_DB_CONFIG_setDefault
(
	FTM_DB_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	strncpy(pConfig->pFileName, FTM_CATCHB_DB_DEFAULT_FILE_NAME, sizeof(pConfig->pFileName) - 1);

	return	FTM_RET_OK;
}

FTM_RET	FTM_DB_CONFIG_load
(
	FTM_DB_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "file");
	if (pItem != NULL)
	{
		if (pItem->type == cJSON_String)
		{
			strncpy(pConfig->pFileName, pItem->valuestring, sizeof(pConfig->pFileName) - 1);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_DB_CONFIG_save
(
	FTM_DB_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON_AddStringToObject(pRoot, "file", pConfig->pFileName);

	return	FTM_RET_OK;
}

FTM_RET	FTM_DB_CONFIG_show
(
	FTM_DB_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL		xLevel
)
{
	ASSERT(pConfig != NULL);

	OUTPUT(xLevel, "[ Database Configuration ]");
	OUTPUT(xLevel, "%16s : %s", "File Name", pConfig->pFileName);

	return	FTM_RET_OK;
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_create
(
	FTM_DB_PTR _PTR_ ppDB
)
{
	ASSERT(ppDB != NULL);

	FTM_RET		xRet=FTM_RET_OK;
	FTM_DB_PTR	pDB;

	pDB = (FTM_DB_PTR)FTM_MEM_calloc(sizeof(FTM_DB),1);

	if (pDB == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create DB!");
	}
	else
	{
		FTM_DB_CONFIG_setDefault(&pDB->xConfig);
		strcpy(pDB->pCCTVTableName, "tb_cctv");
		strcpy(pDB->pAlarmTableName, "tb_alarm");
		strcpy(pDB->pLogTableName, "tb_log");
		strcpy(pDB->pDenyTableName, "tb_deny");
		strcpy(pDB->pSwitchTableName, "tb_switch");
		strcpy(pDB->pStatisticsTableName, "tb_statistics");

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
	FTM_DB_PTR 	pDB
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
		if (sqlite3_open(pDB->xConfig.pFileName, &pDB->pSQLite3) != 0)
		{
			xRet = FTM_RET_DB_OPEN_FAILED;	
			ERROR(xRet, "DB open failed!");
		}
		else
		{
			INFO("The database[%s] opened successfully.", pDB->xConfig.pFileName);
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

FTM_RET	FTM_DB_setConfig
(
	FTM_DB_PTR	pDB,
	FTM_DB_CONFIG_PTR	pConfig
)
{
	ASSERT(pDB != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pDB->xConfig, pConfig, sizeof(FTM_DB_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET	FTM_DB_getConfig
(
	FTM_DB_PTR	pDB,
	FTM_DB_CONFIG_PTR	pConfig
)
{
	ASSERT(pDB != NULL);
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &pDB->xConfig, sizeof(FTM_DB_CONFIG));

	return	FTM_RET_OK;
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
		INFO("Query : %s", pQuery);
		if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
		{
			xRet = FTM_RET_DB_EXEC_ERROR;
			ERROR(xRet, "Failed to execute query!");
			sqlite3_free(pErrorMsg);
		}
		INFO("Query success!");
	}
		
	INFO("The tables[%s] created.", pTableName);
	return	xRet;
}

FTM_RET	FTM_DB_destroyTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pTableName
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_BOOL	bExist = FTM_FALSE;

	FTM_DB_isExistTable(pDB, pTableName, &bExist);
	if (!bExist)
	{
		xRet = FTM_RET_DB_TABLE_NOT_EXIST;	
		ERROR(xRet, "The table[%s] is not exist!", pTableName);
	}
	else
	{
		FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
		FTM_CHAR_PTR	pErrorMsg;

		memset(pQuery, 0, sizeof(pQuery));

		snprintf(pQuery, sizeof(pQuery) - 1, "DROP TABLE %s", pTableName);
		INFO("Query : %s", pQuery);
		if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
		{
			xRet = FTM_RET_DB_EXEC_ERROR;
			ERROR(xRet, "Failed to execute query!");
			sqlite3_free(pErrorMsg);
		}
		INFO("Query success!");
	}
		
	return	xRet;
}

typedef struct FTM_DB_IS_EXIST_TABLE_PARAMS_STRUCT
{
	FTM_CHAR_PTR	pName;
	FTM_BOOL        bExist;
	
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

	FTM_DB_IS_EXIST_TABLE_PARAMS xParams;
	
	xParams.bExist = FTM_FALSE;
	xParams.pName 	= pTableName;

	FTM_CHAR_PTR    pQuery = "select name from sqlite_master where type='table' order by name";
	FTM_CHAR_PTR    pErrMsg = NULL;

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_isExistTableCB, &xParams, &pErrMsg) != 0)
	{    
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute DB query !");
		sqlite3_free(pErrMsg);
	}    
	else
	{
		*pExist = xParams.bExist;
	}
	INFO("Query success!");

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

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getElementCountCB, &xParams, &pErrorMsg) != 0)
	{
		sqlite3_free(pErrorMsg);
		*pCount = 0;
	}
	else
	{
		*pCount = xParams.ulCount;
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_getElementList
(
	FTM_DB_PTR pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pDB != NULL);
	ASSERT(pTableName != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s", pTableName);
	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " LIMIT %u OFFSET %u", ulCount, ulIndex);

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, fCallback, pData, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_getElementListOrderByTime
(
	FTM_DB_PTR pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pDB != NULL);
	ASSERT(pTableName != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s", pTableName);
	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC LIMIT %u OFFSET %u", ulCount, ulIndex);

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, fCallback, pData, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_getElementListOfTimePeriod
(
	FTM_DB_PTR pDB, 
	FTM_CHAR_PTR	pTableName,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulCount,
	FTM_DB_GET_ELEMENT_LIST_CALLBACK	fCallback,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pDB != NULL);
	ASSERT(pTableName != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s", pTableName);
	if ((ulStartTime != 0) || (ulEndTime != 0))
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
	}

	if (ulStartTime != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (%u <= _TIME)", ulStartTime);	
	}

	if ((ulStartTime != 0) && (ulEndTime != 0))
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
	}

	if (ulEndTime != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_TIME <= %u)", ulEndTime);	
	}

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC LIMIT %u", ulCount);

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, fCallback, pData, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////

FTM_RET	FTM_DB_createCCTVTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pCCTVTableName, "_ID TEXT PRIMARY KEY, _IP TEXT, _SWITCH_ID TEXT, _COMMENT TEXT, _TIME INT, HASH TEXT, STAT INT");
}

FTM_RET	FTM_DB_isCCTVTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pCCTVTableName, pExist);
}

FTM_RET	FTM_DB_getCCTVCount
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
)
{
	return	FTM_DB_getElementCount(pDB, pDB->pCCTVTableName, pCount);
}

FTM_RET	FTM_DB_addCCTV
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pComment,
	FTM_UINT32		ulTime
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s(_ID, _IP, _SWITCH_ID, _COMMENT, _TIME, HASH, STAT) VALUES('%s', '%s', '%s', '%s', %d, '', 0);", 
		pDB->pCCTVTableName, pID, pIP, pSwitchID, (pComment != NULL)?pComment:"", ulTime);
	
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to insert item to DB!");
		sqlite3_free(pErrorMsg);
	}
	else
	{
		INFO("DB[%s] inserted(%s, %s, %s %s, %d, '')", pDB->pCCTVTableName, pID, pIP, pSwitchID, (pComment != NULL)?pComment:"", ulTime);
	}

	return	xRet;
}

FTM_RET	FTM_DB_updateCCTV
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
		if (!bNew)
		{
			ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " _COMMENT = '%s'", pComment);
	}

	if (pHash != NULL)
	{
		if (!bNew)
		{
			ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " HASH = '%s'", pHash);
	}

	
	ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE _ID = '%s';", pID);
	
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_deleteCCTV
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
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _ID = '%s'", pDB->pCCTVTableName, pID);
	
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

typedef struct
{
	FTM_UINT32			ulMaxCount;
	FTM_UINT32			ulCount;
	FTM_CCTV_CONFIG_PTR	pElements;
}   FTM_GET_CCTV_CONFIG_LIST_PARAMS, _PTR_ FTM_GET_CCTV_CONFIG_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_getCCTVListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT		nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_GET_CCTV_CONFIG_LIST_PARAMS_PTR	pParams = (FTM_GET_CCTV_CONFIG_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "_ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_SWITCH_ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pSwitchID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_COMMENT") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pComment, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_TIME") == 0)
			{    
				pParams->pElements[pParams->ulCount].ulTime = strtoul(ppArgv[i], 0, 10);
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

FTM_RET	FTM_DB_getCCTVUsingIP
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
    FTM_GET_CCTV_CONFIG_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= 1;
	xParams.ulCount 	= 0;
	xParams.pElements	= pCCTV;


	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s WHERE _IP = %s", pDB->pCCTVTableName, pIP);

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getCCTVListCB, (FTM_VOID_PTR)&xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	else if (xParams.ulCount == 0)
	{
		xRet = FTM_RET_OBJECT_NOT_FOUND;
		ERROR(xRet, "Object not found!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_getCCTVList
(
	FTM_DB_PTR			pDB,
	FTM_UINT32			ulMaxCount,
	FTM_CCTV_CONFIG_PTR	pElements,
	FTM_UINT32_PTR		pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_GET_CCTV_CONFIG_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount 	= 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pCCTVTableName, 0, ulMaxCount, FTM_DB_getCCTVListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

FTM_RET	FTM_DB_updateCCTVHash
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
	snprintf(pQuery, sizeof(pQuery) - 1, "UPDATE %s SET HASH='%s' WHERE _ID = '%s'", pDB->pCCTVTableName, pHash, pID);

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_setCCTVStat
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
	snprintf(pQuery, sizeof(pQuery) - 1, "UPDATE %s SET STAT=%d WHERE _ID = '%s'", pDB->pCCTVTableName, xStat, pID);

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_setCCTVProperties
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32	ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;
	FTM_BOOL	bFirst = FTM_TRUE;

	memset(pQuery, 0, sizeof(pQuery));
	ulQueryLen = snprintf(pQuery, sizeof(pQuery) - 1, "UPDATE %s SET ", pDB->pCCTVTableName);

	if (ulFieldFlags & FTM_CCTV_FIELD_IP)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, "_IP = %s", pConfig->pIP);
		bFirst = FTM_FALSE;
	}

	if (ulFieldFlags & FTM_CCTV_FIELD_SWITCH_ID)
	{
		if (!bFirst)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, ", ");
			bFirst = FTM_FALSE;
		}
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, "_SWITCH_ID = %s", pConfig->pSwitchID);
	}

	if (ulFieldFlags & FTM_CCTV_FIELD_COMMENT)
	{
		if (!bFirst)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, ", ");
			bFirst = FTM_FALSE;
		}
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, "_COMMENT = \"%s\"", pConfig->pComment);
	}

	if (ulFieldFlags & FTM_CCTV_FIELD_HASH)
	{
		if (!bFirst)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, ", ");
			bFirst = FTM_FALSE;
		}
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, "_HASH = %s", pConfig->pHash);
	}

	if (ulFieldFlags & FTM_CCTV_FIELD_STAT)
	{
		if (!bFirst)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, ", ");
			bFirst = FTM_FALSE;
		}
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, "_STAT = %d", pConfig->xStat);
	}

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen - 1, " WHERE _ID = '%s'", pID);

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_createAlarmTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pAlarmTableName, "_NAME TEXT PRIMARY KEY, _EMAIL TEXT, _MESSAGE TEXT");
}

FTM_RET	FTM_DB_isAlarmTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pAlarmTableName, pExist);
}

FTM_RET	FTM_DB_getAlarmCount
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

FTM_RET	FTM_DB_addAlarm
(
	FTM_DB_PTR		pDB,
	FTM_ALARM_PTR	pAlarm
)
{
	ASSERT(pDB != NULL);
	ASSERT(pAlarm != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s(_NAME, _EMAIL, _MESSAGE) VALUES('%s', '%s', '%s');", pDB->pAlarmTableName, pAlarm->pName, pAlarm->pEmail, pAlarm->pMessage);
	
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_deleteAlarm
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pName
)
{
	ASSERT(pDB != NULL);
	ASSERT(pName != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _NAME = '%s'", pDB->pAlarmTableName, pName);
	
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

typedef struct
{
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount;
	FTM_ALARM_PTR	pElements;
}   FTM_GET_ALARM_LIST_PARAMS, _PTR_ FTM_GET_ALARM_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_getAlarmListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT		nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_GET_ALARM_LIST_PARAMS_PTR	pParams = (FTM_GET_ALARM_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "_NAME") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pName, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_MESSAGE") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pMessage, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_EMAIL") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pEmail, ppArgv[i]);
			}    
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}


FTM_RET	FTM_DB_getAlarmList
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
    FTM_GET_ALARM_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount =	 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pAlarmTableName, 0, ulMaxCount, FTM_DB_getAlarmListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}


////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_createLogTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pLogTableName, "_TYPE INT, _TIME TEXT, _ID TEXT, _IP TEXT, _STAT INT, _HASH TEXT");
}

FTM_RET	FTM_DB_isLogTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pLogTableName, pExist);
}

FTM_RET	FTM_DB_getLogCount
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

FTM_RET	FTM_DB_addLog
(
	FTM_DB_PTR		pDB,
	FTM_LOG_TYPE	xType,
	FTM_UINT32		ulTime,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_CHAR_PTR	pHash
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s (_TYPE, _TIME, _ID, _IP, _STAT, _HASH) VALUES(%d, %d, '%s', '%s', %d, '%s');", 
		pDB->pLogTableName, xType, ulTime, pID, pIP, xStat, pHash);
	
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_deleteLogOfID
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
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _ID = '%s'", pDB->pLogTableName, pID);
	
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

typedef struct
{
	FTM_UINT32	ulMaxCount;
	FTM_UINT32	ulCount;
	FTM_LOG_PTR	pElements;
}   FTM_GET_LOG_LIST_PARAMS, _PTR_ FTM_GET_LOG_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_getLogListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT			nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_GET_LOG_LIST_PARAMS_PTR	pParams = (FTM_GET_LOG_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "_ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_TYPE") == 0)
			{    
				pParams->pElements[pParams->ulCount].xType = atoi(ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_TIME") == 0)
			{    
				pParams->pElements[pParams->ulCount].ulTime = strtoul(ppArgv[i], 0, 10);
			}    
			else if (strcmp(ppColName[i], "_STAT") == 0)
			{    
				pParams->pElements[pParams->ulCount].xStat = strtoul(ppArgv[i], 0, 10);
			}    
			else if (strcmp(ppColName[i], "_HASH") == 0)
			{    
				strncpy(pParams->pElements[pParams->ulCount].pHash, ppArgv[i], FTM_HASH_LEN);
			}    
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}

FTM_RET	FTM_DB_getLogList
(
	FTM_DB_PTR		pDB,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pLogs,
	FTM_UINT32_PTR	pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pLogs != NULL);
	ASSERT(pCount != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_LOG_LIST_PARAMS	xParams;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;
	FTM_BOOL		bFirstCondition = FTM_TRUE;

	xParams.ulMaxCount = ulMaxCount;
	xParams.ulCount = 0;
	xParams.pElements 	= pLogs;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s", pDB->pLogTableName);
	if (xType != FTM_LOG_TYPE_UNKNOWN)
	{
		if (bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_TYPE = %d)", xType);	
	}

	if (pID != NULL)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_ID = \'%s\')", pID);	
	}

	if (pIP != NULL)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_IP = \'%s\')", pIP);	
	}
	
	if (xStat != FTM_CCTV_STAT_UNREGISTERED)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_STAT = %d)", xStat);	
	}

	if (ulStartTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (%u <= _TIME)", ulStartTime);	
	}

	if (ulEndTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_TIME <= %u)", ulEndTime);	
	}

//	if ((ulStartTime != 0) || (ulEndTime != 0))
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC");
	}
	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " LIMIT %u", ulMaxCount);

	if (ulIndex != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " OFFSET %u", ulIndex);
	}

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getLogListCB, &xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	else
	{
		*pCount = xParams.ulCount;	
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_getLogInfo
(
	FTM_DB_PTR		pDB,
	FTM_UINT32_PTR	pCount,
	FTM_UINT32_PTR	pulStartTime,
	FTM_UINT32_PTR	pulEndTime
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_LOG_LIST_PARAMS	xParams;
	FTM_LOG			xLog;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;

	xRet =FTM_DB_getElementCount(pDB, pDB->pLogTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tv_log_info!");
		return	xRet;
	}

	xParams.ulMaxCount = 1;
	xParams.ulCount = 0;
	xParams.pElements 	= &xLog;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s ORDER BY _TIME ASC LIMIT 1", pDB->pLogTableName);
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getLogListCB, &xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
		INFO("Query success!");
		goto finished;
	}
	else
	{
		if (xParams.ulCount)
		{
			*pulStartTime = xParams.pElements[0].ulTime;	
		}
	}
	INFO("Query success!");

	xParams.ulMaxCount = 1;
	xParams.ulCount = 0;
	xParams.pElements 	= &xLog;

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s ORDER BY _TIME DESC LIMIT 1", pDB->pLogTableName);
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getLogListCB, &xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
		INFO("Query success!");
		goto finished;
	}
	else
	{
		if (xParams.ulCount)
		{
			*pulEndTime = xParams.pElements[0].ulTime;	
		}
	}
	INFO("Query success!");

finished:
	return	xRet;
}

FTM_RET	FTM_DB_getLogListFrom
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pElements,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements!= NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_LOG_LIST_PARAMS	xParams;

	xParams.ulMaxCount = ulMaxCount;
	xParams.ulCount = 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementListOrderByTime(pDB, pDB->pLogTableName, ulIndex, ulMaxCount, FTM_DB_getLogListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

FTM_RET	FTM_DB_getLogListOfTimePeriod
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pElements,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements!= NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_LOG_LIST_PARAMS	xParams;

	xParams.ulMaxCount = ulMaxCount;
	xParams.ulCount = 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementListOfTimePeriod(pDB, pDB->pLogTableName, ulStartTime, ulEndTime, ulMaxCount, FTM_DB_getLogListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}


FTM_RET	FTM_DB_deleteLog
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulCount
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;
	FTM_BOOL		bFirstCondition = FTM_TRUE;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s", pDB->pLogTableName);

	if (pID != NULL)
	{
		if (bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_ID = \'%s\')", pID);	
	}

	if (pIP != NULL)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_IP = \'%s\')", pIP);	
	}
	
	if (xStat != FTM_CCTV_STAT_UNREGISTERED)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_STAT = %d)", xStat);	
	}

	if (ulStartTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (%u <= _TIME)", ulStartTime);	
	}

	if (ulEndTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_TIME <= %u)", ulEndTime);	
	}

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC");

	if (ulCount != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " LIMIT %u", ulCount);
	}

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, NULL, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_deleteLogFrom
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;
	FTM_BOOL		bFirstCondition = FTM_TRUE;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _TIME IN ( SELECT _TIME FROM %s ", pDB->pLogTableName, pDB->pLogTableName);

	if (pID != NULL)
	{
		if (bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_ID = \'%s\')", pID);	
	}


	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC");

	if (ulCount != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " LIMIT %u", ulCount);
	}

	if (ulIndex != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " OFFSET %u", ulIndex);
	}

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ");");

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, NULL, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_removeExpiredLog
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulRetentionPeriod
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	time_t		xCurrentTime;
	time_t		xCurrentDate;
	FTM_UINT32	ulRetentionTime;
	struct tm 	xTM;

	INFO("Remove expired log : %s", FTM_TIME_printfCurrent(NULL));

	ulRetentionTime = ulRetentionPeriod*24*3600;

	xCurrentTime = time(NULL);
	localtime_r(&xCurrentTime, &xTM);

	xTM.tm_sec = 0;
	xTM.tm_min = 0;
	xTM.tm_hour= 0;

	xCurrentDate = mktime(&xTM);

	if(xCurrentDate > ulRetentionTime)
	{
		FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
		FTM_CHAR_PTR	pErrorMsg;
		FTM_UINT32		ulExpirationDate;

		ulExpirationDate = xCurrentDate - ulRetentionTime;

		memset(pQuery, 0, sizeof(pQuery));
		snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE (%u > _TIME)", pDB->pLogTableName, ulExpirationDate);

		INFO("Query : %s", pQuery);
		if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
		{
			xRet = FTM_RET_ERROR;
			sqlite3_free(pErrorMsg);
		}
		INFO("Query success!");
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_createSwitchTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pSwitchTableName, "_ID TEXT PRIMARY KEY, _MODEL INT, _IP TEXT, _USERID TEXT, _PASSWD TEXT, _SECURE INT, _COMMENT TEXT");
}

FTM_RET	FTM_DB_isSwitchTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pSwitchTableName, pExist);
}

FTM_RET	FTM_DB_getSwitchCount
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

FTM_RET	FTM_DB_addSwitch
(
	FTM_DB_PTR			pDB,
	FTM_CHAR_PTR		pID,
	FTM_SWITCH_MODEL	xModel,
	FTM_CHAR_PTR		pUser,
	FTM_CHAR_PTR		pPasswd,
	FTM_CHAR_PTR		pIP,
	FTM_BOOL			bSecure,
	FTM_CHAR_PTR		pComment	
)
{
	ASSERT(pDB != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s (_ID, _MODEL, _IP, _USERID, _PASSWD, _SECURE, _COMMENT) VALUES('%s', %d, '%s', '%s', '%s', %d, '%s');", 
		pDB->pSwitchTableName, pID, xModel, (pIP)?pIP:"", (pUser)?pUser:"", (pPasswd)?pPasswd:"", bSecure, (pComment)?pComment:"");

	INFO("QUERY : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_deleteSwitch
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
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _ID = '%s'", pDB->pSwitchTableName, pID);
	
		INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
		INFO("Query success!");

	return	xRet;
}

typedef struct
{
	FTM_UINT32		ulMaxCount;
	FTM_UINT32		ulCount;
	FTM_SWITCH_CONFIG_PTR	pElements;
}   FTM_DB_GET_SWITCH_LIST_PARAMS, _PTR_ FTM_DB_GET_SWITCH_LIST_PARAMS_PTR;

FTM_RET	FTM_DB_setSwitchProperties
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
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
	ulQueryLen = snprintf(pQuery, sizeof(pQuery) - ulQueryLen, "UPDATE %s SET", pDB->pSwitchTableName);

	if (ulFieldFlags & FTM_SWITCH_FIELD_IP)
	{
		if (!bNew)
		{
			ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " _IP = '%s'", pConfig->pIP);
	}

	if (ulFieldFlags & FTM_SWITCH_FIELD_USER_ID)
	{
		if (!bNew)
		{
			ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " _USERID = '%s'", pConfig->pUserID);
	}

	if (ulFieldFlags & FTM_SWITCH_FIELD_PASSWD)
	{
		if (!bNew)
		{
			ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " _PASSWD = '%s'", pConfig->pPasswd);
	}

	if (ulFieldFlags & FTM_SWITCH_FIELD_SECURE)
	{
		if (!bNew)
		{
			ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " _SECURE = '%d'", pConfig->bSecure);
	}

	if (ulFieldFlags & FTM_SWITCH_FIELD_COMMENT)
	{
		if (!bNew)
		{
			ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ",");
			bNew = FTM_FALSE;
		}

		ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " _COMMENT = '%s'", pConfig->pComment);
	}

	
	ulQueryLen +=snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE _ID = '%s';", pID);

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}


static 
FTM_INT	FTM_DB_getSwitchListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT			nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_DB_GET_SWITCH_LIST_PARAMS_PTR	pParams = (FTM_DB_GET_SWITCH_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "_ID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_MODEL") == 0)
			{
				pParams->pElements[pParams->ulCount].xModel = atoi(ppArgv[i]);
			}
			else if (strcmp(ppColName[i], "_IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_USERID") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pUserID, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_PASSWD") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pPasswd, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_SECURE") == 0)
			{    
				pParams->pElements[pParams->ulCount].bSecure = strtoul(ppArgv[i],0, 10);
			}    
			else if (strcmp(ppColName[i], "_COMMENT") == 0)
			{
				strcpy(pParams->pElements[pParams->ulCount].pComment, ppArgv[i]);
			}
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}


FTM_RET	FTM_DB_getSwitchList
(
	FTM_DB_PTR		pDB,
	FTM_SWITCH_CONFIG_PTR	pElements,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_DB_GET_SWITCH_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount 	= 0;
	xParams.pElements	= pElements;

	xRet = FTM_DB_getElementList(pDB, pDB->pSwitchTableName, 0, ulMaxCount, FTM_DB_getSwitchListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_createACTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID
)
{
	ASSERT(pDB != NULL);
	ASSERT(pSwitchID != NULL);

	FTM_CHAR	pTableName[256];

	sprintf(pTableName, "tb_sw_%s_ac", pSwitchID);

	return FTM_DB_createTable(pDB, pTableName, "_IP TEXT PRIMARY KEY, _INDEX INT, _POLICY INT");
}

FTM_RET	FTM_DB_destroyACTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID
)
{
	ASSERT(pDB != NULL);
	ASSERT(pSwitchID != NULL);

	FTM_CHAR	pTableName[256];

	sprintf(pTableName, "tb_sw_%s_ac", pSwitchID);

	return FTM_DB_destroyTable(pDB, pTableName);
}

FTM_RET	FTM_DB_isACTableExist
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID,
	FTM_BOOL_PTR	pExist
)
{
	ASSERT(pDB != NULL);
	ASSERT(pSwitchID != NULL);
	ASSERT(pExist != NULL);

	FTM_CHAR	pTableName[256];

	sprintf(pTableName, "tb_sw_%s_ac", pSwitchID);

	return	FTM_DB_isExistTable(pDB, pTableName, pExist);
}

FTM_RET	FTM_DB_getACCount
(
	FTM_DB_PTR 		pDB, 
	FTM_CHAR_PTR	pSwitchID,
	FTM_UINT32_PTR 	pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);

	FTM_CHAR	pTableName[256];
	FTM_RET		xRet = FTM_RET_OK;

	sprintf(pTableName, "tb_sw_%s_ac", pSwitchID);
	
	xRet =FTM_DB_getElementCount(pDB, pTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tv_deny_info!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_addAC
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pIP,
	FTM_UINT32		nIndex,
	FTM_SWITCH_AC_POLICY	xPolicy
)
{
	ASSERT(pDB != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pSwitchID != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;
	FTM_CHAR	pTableName[256];

	sprintf(pTableName, "tb_sw_%s_ac", pSwitchID);

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s (_IP, _INDEX, _POLICY) VALUES('%s', %d, %d);", pTableName, pIP, nIndex, xPolicy);
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_deleteAC
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pIP
)
{
	ASSERT(pDB != NULL);
	ASSERT(pIP != NULL);
	FTM_RET			xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;
	FTM_CHAR	pTableName[256];

	sprintf(pTableName, "tb_sw_%s_ac", pSwitchID);

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _IP = '%s'", pTableName, pIP);
	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}


typedef struct
{
	FTM_UINT32			ulMaxCount;
	FTM_UINT32			ulCount;
	FTM_SWITCH_AC_PTR	pElements;
}   FTM_GET_AC_LIST_PARAMS, _PTR_ FTM_GET_AC_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_getACListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT			nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_GET_AC_LIST_PARAMS_PTR	pParams = (FTM_GET_AC_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "_IP") == 0)
			{    
				strcpy(pParams->pElements[pParams->ulCount].pIP, ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_POLICY") == 0)
			{    
				pParams->pElements[pParams->ulCount].xPolicy = atoi(ppArgv[i]);
			}    
			else if (strcmp(ppColName[i], "_INDEX") == 0)
			{    
				pParams->pElements[pParams->ulCount].nIndex = atoi(ppArgv[i]);
			}    
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}


FTM_RET	FTM_DB_getACList
(
	FTM_DB_PTR			pDB,
	FTM_CHAR_PTR		pSwitchID,
	FTM_SWITCH_AC_PTR	pElements,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pSwitchID != NULL);
	ASSERT(pElements != NULL);
	ASSERT(pCount != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_AC_LIST_PARAMS	xParams;
	FTM_CHAR	pTableName[256];

	sprintf(pTableName, "tb_sw_%s_ac", pSwitchID);
	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount 	= 0;
	xParams.pElements	= pElements;

	xRet = FTM_DB_getElementList(pDB, pTableName, 0, ulMaxCount, FTM_DB_getACListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_DB_createStatisticsTable
(
	FTM_DB_PTR	pDB
)
{
	return FTM_DB_createTable(pDB, pDB->pStatisticsTableName, "_TIME INT, _CPU INT, _TOTAL_MEM INT, _FREE_MEM INT, _NET_RX INT, _NET_TX INT");
}

FTM_RET	FTM_DB_isStatisticsTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
)
{
	return	FTM_DB_isExistTable(pDB, pDB->pStatisticsTableName, pExist);
}

FTM_RET	FTM_DB_getStatisticsCount
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	
	xRet =FTM_DB_getElementCount(pDB, pDB->pStatisticsTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table tv_log_info!");
	}

	return	xRet;
}

FTM_RET	FTM_DB_addStatistics
(
	FTM_DB_PTR		pDB,
	FTM_STATISTICS_PTR	pStatistics
)
{
	ASSERT(pDB != NULL);
	ASSERT(pStatistics != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR	pQuery[FTM_DB_QUERY_LEN+1];
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));
	snprintf(pQuery, sizeof(pQuery) - 1, "INSERT INTO %s (_TIME, _CPU, _TOTAL_MEM, _FREE_MEM, _NET_RX, _NET_TX) VALUES(%d, %d, %d, %d , %d, %d);", 
		pDB->pStatisticsTableName, 
		(FTM_UINT32)pStatistics->xTime.xTimeval.tv_sec, 
		(FTM_UINT32)(pStatistics->fCPU * 100), 
		pStatistics->xMemory.ulTotal, 
		pStatistics->xMemory.ulFree,
		pStatistics->xNet.ulRxBytes,
		pStatistics->xNet.ulTxBytes);

	INFO("Query : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
	{
		xRet = FTM_RET_ERROR;
		sqlite3_free(pErrorMsg);
	}
	INFO("Query success!");

	return	xRet;
}

typedef struct
{
	FTM_UINT32	ulMaxCount;
	FTM_UINT32	ulCount;
	FTM_STATISTICS_PTR	pElements;
}   FTM_GET_STATISTICS_LIST_PARAMS, _PTR_ FTM_GET_STATISTICS_LIST_PARAMS_PTR;

static 
FTM_INT	FTM_DB_getStatisticsListCB
(
	FTM_VOID_PTR	pData, 
 	FTM_INT			nArgc, 
 	FTM_CHAR_PTR	ppArgv[],
 	FTM_CHAR_PTR	ppColName[]
)
{
	FTM_GET_STATISTICS_LIST_PARAMS_PTR	pParams = (FTM_GET_STATISTICS_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{    
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++) 
		{    
			if (strcmp(ppColName[i], "_TIME") == 0)
			{    
				pParams->pElements[pParams->ulCount].xTime.xTimeval.tv_sec = (time_t)strtoul(ppArgv[i], 0, 10);
			}    
			else if (strcmp(ppColName[i], "_CPU") == 0)
			{    
				pParams->pElements[pParams->ulCount].fCPU = (float)strtoul(ppArgv[i], 0, 10) / 100;
			}    
			else if (strcmp(ppColName[i], "_TOTAL_MEM") == 0)
			{    
				pParams->pElements[pParams->ulCount].xMemory.ulTotal = strtoul(ppArgv[i], 0, 10);
			}    
			else if (strcmp(ppColName[i], "_FREE_MEM") == 0)
			{    
				pParams->pElements[pParams->ulCount].xMemory.ulFree = strtoul(ppArgv[i], 0, 10);
			}    
			else if (strcmp(ppColName[i], "_NET_RX") == 0)
			{    
				pParams->pElements[pParams->ulCount].xNet.ulRxBytes = strtoul(ppArgv[i], 0, 10);
			}    
			else if (strcmp(ppColName[i], "_NET_TX") == 0)
			{    
				pParams->pElements[pParams->ulCount].xNet.ulTxBytes = strtoul(ppArgv[i], 0, 10);
			}    
		}    

		pParams->ulCount++;
	}    

	return  FTM_RET_OK;
}

FTM_RET	FTM_DB_getStatisticsList
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pStatistics,
	FTM_UINT32_PTR	pCount
)
{
	ASSERT(pDB != NULL);
	ASSERT(pStatistics != NULL);
	ASSERT(pCount != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_STATISTICS_LIST_PARAMS	xParams;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;
	FTM_BOOL		bFirstCondition = FTM_TRUE;

	xParams.ulMaxCount = ulMaxCount;
	xParams.ulCount = 0;
	xParams.pElements 	= pStatistics;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s", pDB->pStatisticsTableName);

	if (ulStartTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (%u <= _TIME)", ulStartTime);	
	}

	if (ulEndTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_TIME <= %u)", ulEndTime);	
	}

	if ((ulStartTime != 0) || (ulEndTime != 0))
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC");
	}
	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " LIMIT %u", ulMaxCount);

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getStatisticsListCB, &xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	else
	{
		*pCount = xParams.ulCount;	
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_getStatisticsInfo
(
	FTM_DB_PTR		pDB,
	FTM_UINT32_PTR	pCount,
	FTM_UINT32_PTR	pulStartTime,
	FTM_UINT32_PTR	pulEndTime
)
{
	ASSERT(pDB != NULL);
	ASSERT(pCount != NULL);

	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_STATISTICS_LIST_PARAMS	xParams;
	FTM_STATISTICS	xStatistics;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;

	xRet =FTM_DB_getElementCount(pDB, pDB->pStatisticsTableName, pCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get element count of table %s!", pDB->pStatisticsTableName);
		return	xRet;
	}

	xParams.ulMaxCount = 1;
	xParams.ulCount = 0;
	xParams.pElements 	= &xStatistics;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s ORDER BY _TIME ASC LIMIT 1;", pDB->pStatisticsTableName);
	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getStatisticsListCB, &xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
		INFO("Query success!");
		goto finished;
	}
	else
	{
		if (xParams.ulCount)
		{
			FTM_TIME_toSecs(&xParams.pElements[0].xTime, pulStartTime);
		}
	}
	INFO("Query success!");

	xParams.ulMaxCount = 1;
	xParams.ulCount = 0;
	xParams.pElements 	= &xStatistics;

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "SELECT * FROM %s ORDER BY _TIME DESC LIMIT 1;", pDB->pStatisticsTableName);
	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, FTM_DB_getStatisticsListCB, &xParams, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
		INFO("Query success!");
		goto finished;
	}
	else
	{
		if (xParams.ulCount)
		{
			FTM_TIME_toSecs(&xParams.pElements[0].xTime, pulEndTime);
		}
	}
	INFO("Query success!");

finished:
	return	xRet;
}

FTM_RET	FTM_DB_getStatisticsListFrom
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pElements,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements!= NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_STATISTICS_LIST_PARAMS	xParams;

	xParams.ulMaxCount 	= ulMaxCount;
	xParams.ulCount 	= 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementListOrderByTime(pDB, pDB->pStatisticsTableName, ulIndex, ulMaxCount, FTM_DB_getStatisticsListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}

FTM_RET	FTM_DB_getStatisticsListOfTimePeriod
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pElements,
	FTM_UINT32_PTR	pCount

)
{
	ASSERT(pDB != NULL);
	ASSERT(pElements!= NULL);
	ASSERT(pCount != NULL);
	FTM_RET	xRet = FTM_RET_OK;
    FTM_GET_STATISTICS_LIST_PARAMS	xParams;

	xParams.ulMaxCount = ulMaxCount;
	xParams.ulCount = 0;
	xParams.pElements 	= pElements;

	xRet = FTM_DB_getElementListOfTimePeriod(pDB, pDB->pStatisticsTableName, ulStartTime, ulEndTime, ulMaxCount, FTM_DB_getStatisticsListCB, (FTM_VOID_PTR)&xParams);
	if (xRet == FTM_RET_OK)
	{
		*pCount = xParams.ulCount;
	}

	return	xRet;
}


FTM_RET	FTM_DB_deleteStatistics
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulCount
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;
	FTM_BOOL		bFirstCondition = FTM_TRUE;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _TIME IN ( SELECT _TIME FROM %s ", pDB->pStatisticsTableName, pDB->pStatisticsTableName);

	if (ulStartTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (%u <= _TIME)", ulStartTime);	
	}

	if (ulEndTime != 0)
	{
		if (!bFirstCondition)
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " AND");	
		}
		else
		{
			ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " WHERE");	
			bFirstCondition = FTM_FALSE;
		}	

		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " (_TIME <= %u)", ulEndTime);	
	}

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC");

	if (ulCount != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " LIMIT %u", ulCount);
	}

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ");");

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, NULL, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_deleteStatisticsFrom
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
	FTM_UINT32		ulQueryLen = 0;
	FTM_CHAR_PTR	pErrorMsg;

	memset(pQuery, 0, sizeof(pQuery));

	ulQueryLen += snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE _TIME IN ( SELECT _TIME FROM %s ", pDB->pStatisticsTableName, pDB->pStatisticsTableName);

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " ORDER BY _TIME DESC");

	if (ulCount != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " LIMIT %u", ulCount);
	}

	if (ulIndex != 0)
	{
		ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, " OFFSET %u", ulIndex);
	}

	ulQueryLen += snprintf(&pQuery[ulQueryLen], sizeof(pQuery) - ulQueryLen, ");");

	INFO("SQL : %s", pQuery);
	if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, NULL, &pErrorMsg) < 0)
	{
		xRet = FTM_RET_DB_EXEC_ERROR;
		ERROR(xRet, "Failed to execute query!");
	}
	INFO("Query success!");

	return	xRet;
}

FTM_RET	FTM_DB_removeExpiredStatistics
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulRetentionPeriod
)
{
	ASSERT(pDB != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	time_t		xCurrentTime;
	time_t		xCurrentDate;
	FTM_UINT32	ulRetentionTime;
	struct tm 	xTM;

	INFO("Remove expired log : %s", FTM_TIME_printfCurrent(NULL));

	ulRetentionTime = ulRetentionPeriod*24*3600;

	xCurrentTime = time(NULL);
	localtime_r(&xCurrentTime, &xTM);

	xTM.tm_sec = 0;
	xTM.tm_min = 0;
	xTM.tm_hour= 0;

	xCurrentDate = mktime(&xTM);

	if(xCurrentDate > ulRetentionTime)
	{
		FTM_CHAR		pQuery[FTM_DB_QUERY_LEN+1];
		FTM_CHAR_PTR	pErrorMsg;
		FTM_UINT32		ulExpirationDate;

		ulExpirationDate = xCurrentDate - ulRetentionTime;

		memset(pQuery, 0, sizeof(pQuery));
		snprintf(pQuery, sizeof(pQuery) - 1, "DELETE FROM %s WHERE (%u > _TIME)", pDB->pStatisticsTableName, ulExpirationDate);

		INFO("Query : %s", pQuery);
		if (sqlite3_exec(pDB->pSQLite3, pQuery, NULL, 0, &pErrorMsg) != 0)
		{
			xRet = FTM_RET_ERROR;
			sqlite3_free(pErrorMsg);
		}
		INFO("Query success!");
	}

	return	xRet;
}

////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////
