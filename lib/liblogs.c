
#include <stdio.h>
#include <stdarg.h>
#include <liblogs.h>
#include <time.h>
#include <sys/time.h>

//test
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <liblogs.h>
#include <string.h>

#include <common.h>
void cctv_system_error(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    fprintf(stdout, "ERROR : ");
    vfprintf(stdout, msg, args);
    fprintf(stdout, "\n");
    va_end(args);
}

void log_message(char *filename, const char *message, ...)
{
    FILE *logfile;
    char date[24];
    struct timeval mytime;
    struct tm tm_time;
    va_list args;

    va_start(args, message);

    gettimeofday(&mytime, NULL);
    localtime_r(&mytime.tv_sec, &tm_time);

    sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d" ,
            tm_time.tm_year+1900, tm_time.tm_mon+1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);

    logfile = fopen(filename, "a+");
    if(!logfile){
        cctv_system_error("libcctv/log_message() - %s", strerror(errno));
        return;
    }

    fprintf(logfile, "%s ", date);
    vfprintf(logfile, message, args);
    fprintf(logfile, "\n");

    va_end(args);
    fclose(logfile);
}

void cctv_system_debugger(char * path, const char *msg, ...)
{

    FILE *debugfile;
    va_list args;

    va_start(args, msg);

    debugfile = fopen(DEBUG_FILE, "a");

    if(!debugfile){
        cctv_system_error("libcctv/cctv_system_debugger() - %s", strerror(errno));
        return;
    }

    fprintf(debugfile, "%s : ", path);
    vfprintf(debugfile, msg, args);
    fprintf(debugfile, "\n");
    va_end(args);
    fclose(debugfile);
}



