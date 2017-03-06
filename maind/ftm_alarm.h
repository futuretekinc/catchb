#ifndef	FTM_ALARM_H_
#define	FTM_ALARM_H_

#include "ftm_types.h"


typedef	struct	FTM_ALARM_STRUCT
{
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_CHAR	pEmail[FTM_EMAIL_LEN+1];
	FTM_CHAR	pMessage[FTM_ALARM_MESSAGE_LEN+1];
}	FTM_ALARM, _PTR_ FTM_ALARM_PTR;

FTM_RET	FTM_ALARM_send
(
	FTM_ALARM_PTR	pAlarm
);

#endif
