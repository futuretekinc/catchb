#ifndef	FTM_ALARM_H_
#define	FTM_ALARM_H_

#include "ftm_types.h"

#define	FTM_ALARM_FIELD_NAME	(1 << 0)
#define	FTM_ALARM_FIELD_EMAIL	(1 << 1)
#define	FTM_ALARM_FIELD_MESSAGE	(1 << 2)
#define	FTM_ALARM_FIELD_ALL		(0xFF)

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
