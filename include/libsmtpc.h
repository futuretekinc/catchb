/*
 * smtpc.h - SMTP stuff for libsmtpc.
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

#ifndef __LIBSMTPC_H__
#define __LIBSMTPC_H__

struct smtpclient {
    int sock;
    char *host;
    int port;
};

struct smtpclient *smtpc_new_client(const char *hostname, const int port);
void smtpc_free_client(struct smtpclient *smtpc);
int smtpc_connect(struct smtpclient *smtpc);
int smtpc_send_and_receive(const struct smtpclient *smtpc,
        const char *data);
int smtpc_send_greeting(const struct smtpclient *smtpc,
        const char *hostname);
int smtpc_send_cmd_and_arg(const struct smtpclient *smtpc,
        const char *cmd, const char *arg);
int smtpc_send_to(const struct smtpclient *smtpc, const char *to);
int smtpc_send_from(const struct smtpclient *smtpc, const char *from);
int smtpc_start_body(const struct smtpclient *smtpc);
int smtpc_end_body(const struct smtpclient *smtpc);
int smtpc_send_raw_body(const struct smtpclient *smtpc, const char *data);
int smtpc_quit(const struct smtpclient *smtpc);
int smtpc_close(const struct smtpclient *smtpc);

#endif /* _SMTPC_H */

