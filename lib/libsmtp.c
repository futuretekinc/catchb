/*
 * smtpc.c - SMTP stuff for libsmtpc.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

//#include <glib-2.0/glib.h>

#include <libsmtpc.h>
#include <libalarm.h>

#define	g_new0(x,y)	calloc(sizeof(x), (y))
#define	g_free(x)	free((x))
#define	g_strdup(x)	strdup((x))
char*	g_strdup_printf(char *format, ...)
{
	va_list va;
	char	buffer[1024];

	va_start(va, format);
	
	vsnprintf(buffer, sizeof(buffer) - 1, format, va);

	va_end(va);

	return	strdup(buffer);
}

struct smtpclient *smtpc_new_client(const char *hostname, const int port)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    struct smtpclient *smtpc;

    smtpc = g_new0(struct smtpclient, 1);
    smtpc->host = g_strdup(hostname);
    if(port)
        smtpc->port = port;
    else
        smtpc->port = 25;

    printf("[%s][%d]\n",__func__,__LINE__);
    return smtpc;
}


void smtpc_free_client(struct smtpclient *smtpc)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    g_free(smtpc->host);
    g_free(smtpc);
    printf("[%s][%d]\n",__func__,__LINE__);
}

int smtpc_connect(struct smtpclient *smtpc)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    char *msg;
    int ret;


    smtpc->sock = connection_new(smtpc->host, smtpc->port);

    printf("[%s][%d]\n",__func__,__LINE__);
    printf("[%s][%d]\n",__func__,__LINE__);
    msg = connection_read(smtpc->sock);
    printf("[%s][%d]\n",__func__,__LINE__);
    if(!msg){
    printf("[%s][%d]\n",__func__,__LINE__);
        return -1;
    }
    ret = atoi(msg);
    g_free(msg);

    printf("[%s][%d]\n",__func__,__LINE__);
    return ret;
}

int smtpc_send_and_receive(const struct smtpclient *smtpc,
        const char *data)
{
    int ret;
    char *buf;

    printf("[%s][%d]\n",__func__,__LINE__);
    if(connection_write(smtpc->sock, data) < 0)
        return -1;

    printf("[%s][%d]\n",__func__,__LINE__);
    buf = connection_read(smtpc->sock);
    if(!buf)
        return -1;
    ret = atoi(buf);
    g_free(buf);

    printf("[%s][%d]\n",__func__,__LINE__);
    return ret;
}

int smtpc_send_greeting(const struct smtpclient *smtpc,
        const char *hostname)
{
    int ret;
    char *buf;

    printf("[%s][%d]\n",__func__,__LINE__);
    if(!smtpc)
        return -1;
    buf = g_strdup_printf("EHLO %s\r\n", hostname);

    printf("[%s][%d]\n",__func__,__LINE__);
    ret = smtpc_send_and_receive(smtpc, buf);
    g_free(buf);

    printf("[%s][%d]\n",__func__,__LINE__);
    return ret;
}

int smtpc_send_cmd_and_arg(const struct smtpclient *smtpc,
        const char *cmd, const char *arg)
{
    int ret;
    char *buf;

    printf("[%s][%d]\n",__func__,__LINE__);
    if(!smtpc)
        return -1;

    printf("[%s][%d]\n",__func__,__LINE__);
    buf = g_strdup_printf("%s %s\r\n", cmd, arg);

    printf("[%s][%d]\n",__func__,__LINE__);
    ret = smtpc_send_and_receive(smtpc, buf);
    printf("[%s][%d]\n",__func__,__LINE__);
    g_free(buf);

    printf("[%s][%d]\n",__func__,__LINE__);
    return ret;
}

/* TODO: inline */
int smtpc_send_to(const struct smtpclient *smtpc, const char *to)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    return smtpc_send_cmd_and_arg(smtpc, "RCPT TO:", to);
}

int smtpc_send_from(const struct smtpclient *smtpc, const char *from)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    return smtpc_send_cmd_and_arg(smtpc, "MAIL FROM:", from);
}

int smtpc_start_body(const struct smtpclient *smtpc)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    return smtpc_send_and_receive(smtpc, "DATA\r\n");
}

int smtpc_end_body(const struct smtpclient *smtpc)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    return smtpc_send_and_receive(smtpc, "\r\n.\r\n");
}

int smtpc_send_raw_body(const struct smtpclient *smtpc, const char *data)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    return connection_write(smtpc->sock, data);
}

int smtpc_quit(const struct smtpclient *smtpc)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    // TODO: close
    return smtpc_send_and_receive(smtpc, "QUIT\r\n");
}

int smtpc_close(const struct smtpclient *smtpc)
{
    printf("[%s][%d]\n",__func__,__LINE__);
    // TODO: close
    return close(smtpc->sock);
}

