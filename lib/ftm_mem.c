#include <stdlib.h>
#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_list.h"
#include "ftm_mem.h"

#undef	__MODULE__
#define	__MODULE__	"MEM"

typedef struct
{
	FTM_CHAR_PTR	pFile;
	FTM_UINT32		ulLine;
	FTM_UINT32		xSize;
	FTM_UINT8		pMem[];
}	FTM_MEM_BLOCK, _PTR_ FTM_MEM_BLOCK_PTR;

static FTM_BOOL	FTM_MEM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTM_MEM_comparator(const FTM_VOID_PTR pA, const FTM_VOID_PTR pB);

static FTM_BOOL	bInitialized = FTM_FALSE;
static FTM_LIST_PTR	pMemList = NULL;
static FTM_BOOL bTrace = FTM_FALSE;

FTM_RET			FTM_MEM_init(void)
{
	FTM_LIST_create(&pMemList);
	FTM_LIST_setSeeker(pMemList, FTM_MEM_seeker);
	FTM_LIST_setComparator(pMemList, FTM_MEM_comparator);
	bInitialized = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET			FTM_MEM_final(void)
{
	FTM_RET		xRet;
	FTM_UINT32	i, ulLeakedBlockCount;

	xRet = FTM_LIST_count(pMemList, &ulLeakedBlockCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (ulLeakedBlockCount != 0)
	{
		FTM_MEM_BLOCK_PTR	pMB;

		INFO("Memory leak detected : %lu\n", ulLeakedBlockCount);	
		for(i = 0 ; i < ulLeakedBlockCount ; i++)
		{
			FTM_LIST_getAt(pMemList, i, (FTM_VOID_PTR _PTR_)&pMB);
			INFO("%3lu : %s[%3lu] - %08x(%lu)\n", i, pMB->pFile, pMB->ulLine, pMB->pMem, (FTM_UINT32)pMB->xSize);
			if (pMB->pFile != NULL)
			{
				free(pMB->pFile);	
			}
			free(pMB);
		}
	}

	FTM_LIST_final(pMemList);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_MEM_TRACE_malloc(size_t xSize, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	if (bInitialized == FTM_FALSE)
	{
		return	malloc(xSize);	
	}

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}
	memset(pMB, 0,sizeof(FTM_MEM_BLOCK) + xSize);

	if (pFile != NULL)
	{
		pMB->pFile = malloc(strlen(pFile)+1);
		if (pMB->pFile == NULL)
		{
			free(pMB);
			return	NULL;
		}
	
		strcpy(pMB->pFile, pFile);
	}

	pMB->ulLine= ulLine;
	pMB->xSize = xSize;
	FTM_LIST_append(pMemList, pMB);

	if (bTrace)
	{
		INFO("Memory allocated.- %08lx(%3d) \n", pMB->pMem, xSize);
	}
	return	pMB->pMem;
}

FTM_VOID_PTR	FTM_MEM_TRACE_calloc(size_t xNumber, size_t xSize, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	if (bInitialized == FTM_FALSE)
	{
		return	calloc(xNumber, xSize);	
	}

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xNumber * xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}

	memset(pMB, 0, sizeof(FTM_MEM_BLOCK) + xNumber * xSize);
	if (pFile != NULL)
	{
		pMB->pFile = malloc(strlen(pFile) + 1);
		if (pMB->pFile == NULL)
		{
			free(pMB);
			return	NULL;
		}
		strcpy(pMB->pFile, pFile); 
	}
	pMB->ulLine= ulLine;
	pMB->xSize = xNumber * xSize;
	FTM_LIST_append(pMemList, pMB);

	if (bTrace)
	{
		INFO("Memory allocated.- %08lx(%3d) \n", pMB->pMem, xSize);
	}
	return	pMB->pMem;
}

FTM_RET	FTM_MEM_TRACE_free(FTM_VOID_PTR pMem, const char *pFile, unsigned long ulLine)
{
	FTM_RET	xRet = FTM_RET_OK;
	FTM_MEM_BLOCK_PTR	pMB;

	if (bInitialized == FTM_FALSE)
	{
		free(pMem);	
	}
	else
	{
		if (FTM_LIST_get(pMemList, pMem, (FTM_VOID_PTR _PTR_)&pMB) != FTM_RET_OK)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			ERROR(xRet, "The memory block(%08lx) not found. - %s[%3d]\n", pMem, pFile, ulLine);
			return	xRet;
		}
	
		if (bTrace)
		{
			INFO("%s[%3lu] - %08x(%lu)\n", pMB->pFile, pMB->ulLine, pMB->pMem, (FTM_UINT32)pMB->xSize);
		}
		FTM_LIST_remove(pMemList, pMB);
		if (pMB->pFile != NULL)
		{
			free(pMB->pFile);
		}
		free(pMB);
	}
	
	return	xRet;
}

FTM_BOOL	FTM_MEM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTM_MEM_BLOCK_PTR	pMB = (FTM_MEM_BLOCK_PTR)pElement;

	return	(pMB->pMem == pIndicator);
}

FTM_INT	FTM_MEM_comparator(const FTM_VOID_PTR pA, const FTM_VOID_PTR pB)
{
	if (pA == pB)
	{
		return	0;	
	}

	return	1;
}
