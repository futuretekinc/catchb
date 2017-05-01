#include "ftm_log.h"

FTM_CHAR_PTR	FTM_LOG_TYPE_print
(
	FTM_LOG_TYPE	xType
)
{
	switch(xType)
	{
	case	FTM_LOG_TYPE_NORMAL:	return	"Normal";
	case	FTM_LOG_TYPE_ERROR:	return	"Error";
	default:
		return	"Unknown";
	}

}

