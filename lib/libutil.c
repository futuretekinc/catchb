#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <libutil.h>

static int get_pidfile(const char *pidfile);
static int write_pidfile(const char *pidfile, pid_t pid);

int get_pid_by_process_name(const char *process_name)
{
    int pid = -1; 
    char cmd_string[512];
    FILE *fp;



    sprintf(cmd_string,"ps ax | grep %s | grep -v ' grep ' | awk '{print $1}'",process_name);
    fp = popen(cmd_string, "r");
    fseek(fp, 0, SEEK_SET);
    fscanf(fp, "%d", &pid);


    fclose(fp);
    if (pid < 1)
        pid = -1;

    return pid;
}


int check_pid(const char *pname)
{
    int fpid, mypid;
    char pidfile[PID_PATH_MAX];

    if (!pname || *pname == 0x00)
    {
        //printf("process name is empty (pname=%s)", pname?pname:"NIL");
        return -1;
    }

    mypid = get_pid_by_process_name(pname);
    if(mypid < 0)
        return 0;

    /* read pid from file */
    snprintf(pidfile, sizeof(pidfile), "%s/%s.%s", PID_DIR, pname, PID_EXT);
    fpid = get_pidfile(pidfile);

    
    
    if (fpid > 0)   
    {
        if (fpid != mypid)  
        {
            //if (check_dupproc(pname, fpid))
            //{
            //    printf("[%s][%d] fpid:%d\n",__func__,__LINE__,fpid);
                
            //    errno = EALREADY;
            //    return -1;
            //}
        }
        else
        {
            return -1;
        }
    }
   
    return 0;
}

int write_pid(const char *pname)
{
    int ret;
    int mypid;
    char pidfile[PID_PATH_MAX];

    if (!pname || *pname == 0x00)
    {
        //printf("process name is empty (pname=%s)", pname?pname:"NIL");
        return -1;
    }

    mypid = get_pid_by_process_name(pname);

    snprintf(pidfile, sizeof(pidfile), "%s/%s.%s", PID_DIR, pname, PID_EXT);

    /* save pid to file */
    ret = write_pidfile(pidfile, mypid);
    if (ret > 0)
    {
        //printf("pid(%d) of %s is successfully saved.", mypid, pname);
        return 0;
    }
    else
    {
        //printf("pid(%d) of %s is writing Failed.", mypid, pname);
    }

    /* if failed to write, remove previous file and return fail */
    unlink(pidfile);

    return -1;
}

static int write_pidfile(const char *pidfile, pid_t pid)
{
    FILE *fp = NULL;
    int nwritten = 0;

    /* write pid to file */
    fp = fopen(pidfile, "w");
    if (fp == NULL)
    {
        //printf("fail to open file %s for writing (errno=%d /%s)", pidfile, errno, strerror(errno));
        return -1;
    }
    nwritten = fprintf(fp, "%d", pid);
    fclose(fp);

    /* set default file permission */
    chmod(pidfile, PID_PERM);

    return nwritten;
}

static int get_pidfile(const char *pidfile)
{
    char buf[BUFSIZ];
    FILE *fp = NULL;

    /* read pid from file */
    fp = fopen(pidfile, "r");
    if (fp == NULL)
    {
        //printf("fail to open file %s for reading (errno=%d /%s)", pidfile, errno, strerror(errno));
        return -1;
    }
    fgets(buf, sizeof(buf), fp);
    fclose(fp);

    return atoi(buf);
}


