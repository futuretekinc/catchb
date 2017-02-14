#ifndef __CCTV_MAIND_H__
#define __CCTV_MAIND_H__


#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <common.h>
#include <libdb.h>
#include <syslog.h>
#include <errno.h>


#define CHECK_SECOND       10
#define CHECK_MINUTE       60

#define APP_PATH            "/root/cctv_check/bin"

int check_process();
void get_timef(time_t, char *);
int db_cctv_log_check_delete();


#endif
