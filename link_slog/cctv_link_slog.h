#ifndef __CCTV_LINK_SLOG_H__
#define __CCTV_LINK_SLOG_H__

#define BACKLOG 5

int syslog_init();
int db_cctv_link_server_select();
int systemf(char *fmt, ...);
#endif /* __LOGSER_H__ */

