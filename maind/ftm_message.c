#include "ftm_message.h"

FTM_CHAR_PTR	pMessageString[] =
{
	MESSAGE_TO_STRING(FTM_MSG_TYPE_UNKNOWN),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_SEND_ALARM),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_IP_DETECTED),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_ADD_CCTV),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_DELETE_CCTV),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_CHECK_NEW_CCTV),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_FOUND_NEW_CCTV_IN_DB),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_REMOVED_CCTV_IN_DB),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_CCTV_HASH_UPDATED),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_INITIALIZED),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_CCTV_SET_STAT),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_CCTV_REGISTER),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_SWITCH_CONTROL),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_CHECK_NEW_SWITCH),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_FOUND_NEW_SWITCH_IN_DB),
	MESSAGE_TO_STRING(FTM_MSG_TYPE_REMOVED_SWITCH_IN_DB)
};

const
FTM_CHAR_PTR	FTM_MESSAGE_getString
(
	FTM_MSG_TYPE	xType
)
{
	if (xType < sizeof(pMessageString) / sizeof(FTM_CHAR_PTR))
	{
		return	pMessageString[xType];	
	}

	return	pMessageString[0];
}
