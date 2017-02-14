#include <string.h>
#include <stdlib.h>
#include "catchb_trace.h"
#include "catchb_list.h"

CATCHB_RET CATCHB_LIST_create(CATCHB_LIST_PTR _PTR_ ppList)
{
	CATCHB_LIST_PTR	pList;

	pList = (CATCHB_LIST_PTR)malloc(sizeof(CATCHB_LIST));
	if (pList == NULL)
	{
		return	CATCHB_RET_NOT_ENOUGH_MEMORY;	
	}

	CATCHB_LIST_init(pList);

	*ppList = pList;

	return	CATCHB_RET_OK;
}

CATCHB_RET CATCHB_LIST_destroy(CATCHB_LIST_PTR _PTR_ ppList)
{
	ASSERT(ppList != NULL);

	CATCHB_LIST_final(*ppList);

	free(*ppList);

	*ppList = NULL;

	return	CATCHB_RET_OK;
}

CATCHB_RET CATCHB_LIST_init(CATCHB_LIST_PTR pList)
{
	ASSERT(pList != NULL);


	if (sem_init(&pList->xLock, 0, 1) < 0)
	{
		return	CATCHB_RET_SEMAPHORE_INIT_FAILED;	
	}

	pList->xHead.pNext 	= &pList->xHead;
	pList->xHead.pPrev 	= &pList->xHead;
	pList->xHead.pData 	= NULL;

	pList->ulCount 		= 0;
	pList->bIterator 	= CATCHB_FALSE;

	return	CATCHB_RET_OK;
}

CATCHB_RET	CATCHB_LIST_final(CATCHB_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	CATCHB_ENTRY_PTR	pEntry;

	pEntry = pList->xHead.pNext;
	while(pEntry != &pList->xHead)
	{
		CATCHB_ENTRY_PTR	pNext = pEntry->pNext;
		
		free(pEntry);

		pEntry = pNext;
	}

	sem_destroy(&pList->xLock);

	pList->xHead.pNext 	= &pList->xHead;
	pList->xHead.pPrev 	= &pList->xHead;
	pList->xHead.pData 	= NULL;

	pList->ulCount 		= 0;
	pList->bIterator 	= CATCHB_FALSE;

	return	CATCHB_RET_OK;
}

CATCHB_RET	CATCHB_LIST_seek(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR pKey)
{
	ASSERT(pList != NULL);
	ASSERT(pKey != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET	xRet = CATCHB_RET_OBJECT_NOT_FOUND;

	if (pList->fSeeker != NULL)
	{
		CATCHB_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			CATCHB_ENTRY_PTR	pNext = pEntry->pNext;

			if (pList->fSeeker(pEntry->pData, pKey) == CATCHB_TRUE)
			{
				xRet = CATCHB_RET_OK;
				break;
			}
			pEntry = pNext;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

CATCHB_RET	CATCHB_LIST_append(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR pItem)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET	xRet = CATCHB_RET_OK;

	CATCHB_ENTRY_PTR pEntry;
	pEntry = (CATCHB_ENTRY_PTR)malloc(sizeof(CATCHB_ENTRY));
	if (pEntry == NULL)
	{
		xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory[size = %d]\n", sizeof(CATCHB_ENTRY));
		xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;	
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

CATCHB_RET	CATCHB_LIST_insert(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR pItem, CATCHB_LIST_POS xPos)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET			xRet = CATCHB_RET_OK;
	CATCHB_ENTRY_PTR	pPrev = NULL;
	CATCHB_ENTRY_PTR	pNext = NULL;

	switch(xPos)
	{
	case	CATCHB_LIST_POS_FRONT:
		{
			pPrev = &pList->xHead;
		}
		break;

	case	CATCHB_LIST_POS_REAR:
		{
			pPrev = pList->xHead.pPrev;
		}
		break;

	case	CATCHB_LIST_POS_ASSENDING:
		{
			if (pList->fComparator == NULL)
			{
				return	CATCHB_RET_LIST_NOT_INSERTABLE;
			}

			pNext = pList->xHead.pNext;
			while(pNext != &pList->xHead)
			{
				CATCHB_INT	nRes = pList->fComparator(pNext->pData, pItem);
				if (nRes > 0)
				{
					break;
				}

				pNext = pNext->pNext;
			}
			pPrev = pNext->pPrev;
		}
		break;

	case	CATCHB_LIST_POS_DESENDING:
		{
			if (pList->fComparator == NULL)
			{
				return	CATCHB_RET_LIST_NOT_INSERTABLE;
			}

			pNext = pList->xHead.pNext;
			while(pNext != &pList->xHead)
			{
				CATCHB_INT	nRes = pList->fComparator(pNext->pData, pItem);
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
			return	CATCHB_RET_INVALID_ARGUMENTS;	
		}
	}

	CATCHB_ENTRY_PTR pEntry;
	pEntry = (CATCHB_ENTRY_PTR)malloc(sizeof(CATCHB_ENTRY));
	if (pEntry == NULL)
	{
		xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Not enough memory[size = %d]\n", sizeof(CATCHB_ENTRY));
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

CATCHB_RET	CATCHB_LIST_remove(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR pItem)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET	xRet = CATCHB_RET_OBJECT_NOT_FOUND;
	CATCHB_ENTRY_PTR	pEntry;

	pEntry = pList->xHead.pNext;
	while(pEntry != &pList->xHead)
	{
		CATCHB_ENTRY_PTR	pNext = pEntry->pNext;
	
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

			xRet = CATCHB_RET_OK;
			break;
		}

		pEntry = pNext;
	}

	sem_post(&pList->xLock);

	return	xRet;
}

CATCHB_RET	CATCHB_LIST_removeAt(CATCHB_LIST_PTR pList, CATCHB_UINT32 ulPosition)
{
	ASSERT(pList != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET	xRet = CATCHB_RET_OBJECT_NOT_FOUND;

	if (ulPosition < pList->ulCount)
	{
		CATCHB_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			CATCHB_ENTRY_PTR	pNext = pEntry->pNext;

			if (ulPosition == 0)
			{
				pEntry->pPrev->pNext = pNext;
				pNext->pPrev = pEntry->pPrev;
				free(pEntry);

				pList->ulCount--;

				xRet = CATCHB_RET_OK;
				break;
			}

			pEntry = pNext;
			ulPosition--;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

CATCHB_RET	CATCHB_LIST_get(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR pKey, CATCHB_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(pKey != NULL);
	ASSERT(ppElement != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET	xRet = CATCHB_RET_OBJECT_NOT_FOUND;

	if (pList->fSeeker != NULL)
	{
		CATCHB_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			CATCHB_ENTRY_PTR	pNext = pEntry->pNext;

			if (pList->fSeeker(pEntry->pData, pKey) == CATCHB_TRUE)
			{
				*ppElement = pEntry->pData;

				xRet = CATCHB_RET_OK;
				break;
			}
			pEntry = pNext;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

CATCHB_RET	CATCHB_LIST_getFirst(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	if (pList->ulCount == 0)
	{
		return	CATCHB_RET_LIST_EMPTY;	
	}

	*ppElement = pList->xHead.pNext->pData;

	return	CATCHB_RET_OK;
}

CATCHB_RET	CATCHB_LIST_getLast(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	if (pList->ulCount == 0)
	{
		return	CATCHB_RET_LIST_EMPTY;	
	}

	*ppElement = pList->xHead.pPrev->pData;

	return	CATCHB_RET_OK;
}

CATCHB_RET	CATCHB_LIST_getAt(CATCHB_LIST_PTR pList, CATCHB_UINT32 ulPosition, CATCHB_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET	xRet = CATCHB_RET_OBJECT_NOT_FOUND;

	if (ulPosition < pList->ulCount)
	{
		CATCHB_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			CATCHB_ENTRY_PTR	pNext = pEntry->pNext;

			if (ulPosition == 0)
			{
				*ppElement = pEntry->pData;

				xRet = CATCHB_RET_OK;
				break;
			}

			pEntry = pNext;
			ulPosition--;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

CATCHB_RET	CATCHB_LIST_iteratorStart(CATCHB_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	sem_wait(&pList->xLock);

	pList->bIterator = CATCHB_TRUE;
	pList->pIter = &pList->xHead;

	sem_post(&pList->xLock);
	
	return	CATCHB_RET_OK;
}

CATCHB_RET	CATCHB_LIST_iteratorNext(CATCHB_LIST_PTR pList, CATCHB_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	sem_wait(&pList->xLock);

	CATCHB_RET	xRet = CATCHB_RET_OK;

	if (pList->bIterator != CATCHB_TRUE)
	{
		xRet = CATCHB_RET_NOT_INITIALIZED;	
	}
	else
	{
		pList->pIter = pList->pIter->pNext;
		if (pList->pIter == &pList->xHead)
		{
			xRet = CATCHB_RET_OBJECT_NOT_FOUND;
		}
		else
		{
			*ppElement = pList->pIter->pData;
		}
	}

	sem_post(&pList->xLock);

	return	xRet;
}

CATCHB_RET	CATCHB_LIST_count(CATCHB_LIST_PTR pList, CATCHB_UINT32_PTR pulCount)
{
	ASSERT(pList != NULL);
	ASSERT(pulCount != NULL);
	
	*pulCount = pList->ulCount;

	return	CATCHB_RET_OK;
}

CATCHB_RET CATCHB_LIST_setSeeker(CATCHB_LIST_PTR pList, CATCHB_LIST_ELEM_SEEKER fSeeker)
{
	ASSERT(pList != NULL);
	ASSERT(fSeeker != NULL);

	pList->fSeeker = fSeeker;

	return	CATCHB_RET_OK;
}


CATCHB_RET CATCHB_LIST_setComparator(CATCHB_LIST_PTR pList, CATCHB_LIST_ELEM_COMPARATOR fComparator)
{
	ASSERT(pList != NULL);
	ASSERT(fComparator != NULL);

	pList->fComparator = fComparator;

	return	CATCHB_RET_OK;
}


