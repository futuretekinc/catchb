/*
 * connection.h - connection stuff for libsmtpc.
 *
 * (C) 2006 Carlos Eduardo Medaglia Dyonisio <cadu(a)nerdfeliz.com>
 *          http://nerdfeliz.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version
 * 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __LIBALARM_H__
#define __LIBALARM_H__

#include <libsignal.h>

#define ALARM_TYPE_NONE         0x00 // 사용 안함
#define ALARM_TYPE_SYSTEM       0x01 // 시스템로그
#define ALARM_TYPE_DETECTION    0x02 // 탐지로그

#define CC_NAME_LEN 64
#define CC_ALARM_ADDRESS_LEN 128 

#define WDB_SUCCESS 0
#define WDB_ERROR   1

int connection_new(const char *host_name, int port);
char *connection_read(const int smtp_sock);
int connection_write(const int smtp_sock, const char *data);

int db_cctv_info_select(CK_SIGNAL_INFO *signal_info);
#endif /* _CONNECTION_H */

