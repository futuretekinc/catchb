
#ifndef __LIBSIGNAL_H__
#define __LIBSIGNAL_H__

typedef struct _signal_information {
    int ck_detection_flag;
    int ck_event_division;
    char ck_cctv_id[24];
    char ck_ip[24];
    char ck_log[64];
    char ck_signature[128];

} CK_SIGNAL_INFO;



int ck_signal(char *path, CK_SIGNAL_INFO *info);

#endif /* _CONNECTION_H */

