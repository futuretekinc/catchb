#include <string.h>
#include <stdlib.h>
#include "ftm_trace.h"
#include "ftm_list.h"

FTM_RET FTM_LIST_create(FTM_LIST_PTR _PTR_ ppList)
{
	FTM_LIST_PTR	pList;

	pList = (FTM_LIST_PTR)malloc(sizeof(FTM_LIST));
	if (pList == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTM_LIST_init(pList);

	*ppList = pList;

	return	FTM_RET_OK;
}

FTM_RET FTM_LIST_destroy(FTM_LIST_PTR _PTR_ ppList)
{
	ASSERT(ppList != NULL);

	FTM_LIST_final(*ppList);

	free(*ppList);

	*ppList = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTM_LIST_init(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);


	if (sem_init(&pList->xLock, 0, 1) < 0)
	{
		return	FTM_RET_SEMAPHORE_INIT_FAILED;	
	}

	pList->xHead.pNext 	= &pList->xHead;
	pList->xHead.pPrev 	= &pList->xHead;
	pList->xHead.pData 	= NULL;

	pList->ulCount 		= 0;
	pList->bIterator 	= FTM_FALSE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_final(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	FTM_ENTRY_PTR	pEntry;

	pEntry = pList->xHead.pNext;
	while(pEntry != &pList->xHead)
	{
		FTM_ENTRY_PTR	pNext = pEntry->pNext;
		
		free(pEntry);

		pEntry = pNext;
	}

	sem_destroy(&pList->xLock);

	pList->xHead.pNext 	= &pList->xHead;
	pList->xHead.pPrev 	= &pList->xHead;
	pList->xHead.pData 	= NULL;

	pList->ulCount 		= 0;
	pList->bIterator 	= FTM_FALSE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_seek(FTM_LIST_PTR pList, FTM_VOID_PTR pKey)
{
	ASSERT(pList != NULL);
	ASSERT(pKey != NULL);

	sem_wait(&pList->xLock);

	FTM_RET	xRet = FTM_RET_OBJECT_NOT_FOUND;

	if (pList->fSeeker != NULL)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (pList->fSeeker(pEntry->pData, pKey) == FTM_TRUE)
			{
				xRet = FTM_RET_OK;
				break;
			}
			pEntry = pNext;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_append(FTM_LIST_PTR pList, FTM_VOID_PTR pItem)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	sem_wait(&pList->xLock);

	FTM_RET	xRet = FTM_RET_OK;

	FTM_ENTRY_PTR pEntry;
	pEntry = (FTM_ENTRY_PTR)malloc(sizeof(FTM_ENTRY));
	if (pEntry == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory[size = %d]\n", sizeof(FTM_ENTRY));
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	else
	{
		pEntry->pNext = &pList->xHead;
		pEntry->pPrev = pList->xHead.pPrev;
		pEntry->pData = pItem;

		pList->xHead.pPrev->pNext = pEntry;
		pList->xHead.pPrev = pEntry;

		pList->ulCount++;
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_insert(FTM_LIST_PTR pList, FTM_VOID_PTR pItem, FTM_LIST_POS xPos)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	sem_wait(&pList->xLock);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_ENTRY_PTR	pPrev = NULL;
	FTM_ENTRY_PTR	pNext = NULL;

	switch(xPos)
	{
	case	FTM_LIST_POS_FRONT:
		{
			pPrev = &pList->xHead;
		}
		break;

	case	FTM_LIST_POS_REAR:
		{
			pPrev = pList->xHead.pPrev;
		}
		break;

	case	FTM_LIST_POS_ASSENDING:
		{
			if (pList->fComparator == NULL)
			{
				return	FTM_RET_LIST_NOT_INSERTABLE;
			}

			pNext = pList->xHead.pNext;
			while(pNext != &pList->xHead)
			{
				FTM_INT	nRes = pList->fComparator(pNext->pData, pItem);
				if (nRes > 0)
				{
					break;
				}

				pNext = pNext->pNext;
			}
			pPrev = pNext->pPrev;
		}
		break;

	case	FTM_LIST_POS_DESENDING:
		{
			if (pList->fComparator == NULL)
			{
				return	FTM_RET_LIST_NOT_INSERTABLE;
			}

			pNext = pList->xHead.pNext;
			while(pNext != &pList->xHead)
			{
				FTM_INT	nRes = pList->fComparator(pNext->pData, pItem);
				if (nRes < 0)
				{
					break;
				}

				pNext = pNext->pNext;
			}
			pPrev = pNext->pPrev;
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	FTM_ENTRY_PTR pEntry;
	pEntry = (FTM_ENTRY_PTR)malloc(sizeof(FTM_ENTRY));
	if (pEntry == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Not enough memory[size = %d]\n", sizeof(FTM_ENTRY));
	}
	else
	{
		pEntry->pNext = pPrev->pNext;
		pEntry->pPrev = pPrev;
		pEntry->pData = pItem;

		pPrev->pNext->pPrev = pEntry;
		pPrev->pNext = pEntry;

		pList->ulCount++;
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_remove(FTM_LIST_PTR pList, FTM_VOID_PTR pItem)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	sem_wait(&pList->xLock);

	FTM_RET	xRet = FTM_RET_OBJECT_NOT_FOUND;
	FTM_ENTRY_PTR	pEntry;

	pEntry = pList->xHead.pNext;
	while(pEntry != &pList->xHead)
	{
		FTM_ENTRY_PTR	pNext = pEntry->pNext;
	
		if (pEntry->pData == pItem)
		{
			pEntry->pPrev->pNext = pNext;
			pNext->pPrev = pEntry->pPrev;

			if (pList->pIter == pEntry)
			{
				pList->pIter = pEntry->pPrev;	
			}
			free(pEntry);

			pList->ulCount--;

			xRet = FTM_RET_OK;
			break;
		}

		pEntry = pNext;
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_removeAt(FTM_LIST_PTR pList, FTM_UINT32 ulPosition)
{
	ASSERT(pList != NULL);

	sem_wait(&pList->xLock);

	FTM_RET	xRet = FTM_RET_OBJECT_NOT_FOUND;

	if (ulPosition < pList->ulCount)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (ulPosition == 0)
			{
				pEntry->pPrev->pNext = pNext;
				pNext->pPrev = pEntry->pPrev;
				free(pEntry);

				pList->ulCount--;

				xRet = FTM_RET_OK;
				break;
			}

			pEntry = pNext;
			ulPosition--;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_get(FTM_LIST_PTR pList, FTM_VOID_PTR pKey, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(pKey != NULL);
	ASSERT(ppElement != NULL);

	sem_wait(&pList->xLock);

	FTM_RET	xRet = FTM_RET_OBJECT_NOT_FOUND;

	if (pList->fSeeker != NULL)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (pList->fSeeker(pEntry->pData, pKey) == FTM_TRUE)
			{
				*ppElement = pEntry->pData;

				xRet = FTM_RET_OK;
				break;
			}
			pEntry = pNext;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_getFirst(FTM_LIST_PTR pList, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	if (pList->ulCount == 0)
	{
		return	FTM_RET_LIST_EMPTY;	
	}

	*ppElement = pList->xHead.pNext->pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_getLast(FTM_LIST_PTR pList, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	if (pList->ulCount == 0)
	{
		return	FTM_RET_LIST_EMPTY;	
	}

	*ppElement = pList->xHead.pPrev->pData;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_getAt(FTM_LIST_PTR pList, FTM_UINT32 ulPosition, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	sem_wait(&pList->xLock);

	FTM_RET	xRet = FTM_RET_OBJECT_NOT_FOUND;

	if (ulPosition < pList->ulCount)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (ulPosition == 0)
			{
				*ppElement = pEntry->pData;

				xRet = FTM_RET_OK;
				break;
			}

			pEntry = pNext;
			ulPosition--;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_iteratorStart(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	sem_wait(&pList->xLock);

	pList->bIterator = FTM_TRUE;
	pList->pIter = &pList->xHead;

	sem_post(&pList->xLock);
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_iteratorNext(FTM_LIST_PTR pList, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	sem_wait(&pList->xLock);

	FTM_RET	xRet = FTM_RET_OK;

	if (pList->bIterator != FTM_TRUE)
	{
		xRet = FTM_RET_NOT_INITIALIZED;	
	}
	else
	{
		pList->pIter = pList->pIter->pNext;
		if (pList->pIter == &pList->xHead)
		{
			xRet = FTM_RET_OBJECT_NOT_FOUND;
		}
		else
		{
			*ppElement = pList->pIter->pData;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

FTM_RET	FTM_LIST_count(FTM_LIST_PTR pList, FTM_UINT32_PTR pulCount)
{
	ASSERT(pList != NULL);
	ASSERT(pulCount != NULL);
	
	*pulCount = pList->ulCount;

	return	FTM_RET_OK;
}

FTM_RET FTM_LIST_setSeeker(FTM_LIST_PTR pList, FTM_LIST_ELEM_SEEKER fSeeker)
{
	ASSERT(pList != NULL);
	ASSERT(fSeeker != NULL);

	pList->fSeeker = fSeeker;

	return	FTM_RET_OK;
}


FTM_RET FTM_LIST_setComparator(FTM_LIST_PTR pList, FTM_LIST_ELEM_COMPARATOR fComparator)
{
	ASSERT(pList != NULL);
	ASSERT(fComparator != NULL);

	pList->fComparator = fComparator;

	return	FTM_RET_OK;
}


