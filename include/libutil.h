#ifndef __LIBUTIL_H__
#define __LIBUILT_H__


/* Process ID */
#define PID_DIR     "/var/run"
#define PID_EXT     "pid"
#define PID_PERM    0644


#define PID_PATH_MAX    32


int check_pid(const char *pname);
int write_pid(const char *pname);

#endif
