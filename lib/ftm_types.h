#ifndef	FTM_TYPES_H_
#define	FTM_TYPES_H_

#include <stdint.h>
#include <sys/types.h>

#include "ftm_ret.h"
#include "ftm_default.h"

#define	_PTR_	*

#define	FTM_RET			uint32_t

#define	FTM_VOID		void
#define	FTM_VOID_PTR	void *
#define	FTM_FALSE		0
#define	FTM_TRUE		!(FTM_FALSE)

#define	FTM_SOCKET		int
typedef	unsigned char	FTM_BOOL,	_PTR_ FTM_BOOL_PTR;
typedef	char			FTM_CHAR,	_PTR_ FTM_CHAR_PTR;
typedef	unsigned char	FTM_UINT8,	_PTR_ FTM_UINT8_PTR;
typedef	int				FTM_INT,	_PTR_ FTM_INT_PTR;
typedef	uint16_t		FTM_UINT16, _PTR_ FTM_UINT16_PTR;
typedef	int32_t			FTM_INT32, 	_PTR_ FTM_INT32_PTR;
typedef	uint32_t		FTM_UINT32, _PTR_ FTM_UINT32_PTR;
typedef	int64_t			FTM_INT64, 	_PTR_ FTM_INT64_PTR;
typedef	uint64_t		FTM_UINT64, _PTR_ FTM_UINT64_PTR;
typedef	float			FTM_FLOAT,	_PTR_ FTM_FLOAT_PTR;

typedef	FTM_CHAR		FTM_ID[FTM_ID_LEN+1];
typedef	FTM_ID 	_PTR_	FTM_ID_PTR;

typedef	FTM_CHAR		FTM_NAME[FTM_NAME_LEN+1];
typedef	FTM_NAME _PTR_	FTM_NAME_PTR;

#endif
