#include <common.h>
#include <libdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <mysql/mysql.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <libsignal.h>
#include "trace.h"

int ck_signal(char *path, CK_SIGNAL_INFO *info)
{

    int client_len;
    int sockfd;


    int newSockStat;
    int orgSockStat;
    int res, n;
    fd_set  rset, wset;
    struct timeval tval;


    int error = 0;
    int esize;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        return 0;
    }

    if ( (newSockStat = fcntl(sockfd, F_GETFL, NULL)) < 0 ) 
    {
        perror("F_GETFL error");
        return -1;
    }

    orgSockStat = newSockStat;
    newSockStat |= O_NONBLOCK;


    if(fcntl(sockfd, F_SETFL, newSockStat) < 0)
    {
        perror("F_SETLF error");
        return -1;
    }


    struct sockaddr_un clientaddr;

    bzero(&clientaddr, sizeof(clientaddr));

    clientaddr.sun_family = AF_UNIX;

    strcpy(clientaddr.sun_path, path);

    client_len = sizeof(clientaddr);

    if ((res = connect(sockfd, (struct sockaddr *)&clientaddr, client_len)) < 0)
    {

        close(sockfd);
        return 0;
    }

    if(res == 0)
    {
        write(sockfd, info, sizeof(CK_SIGNAL_INFO));
        close(sockfd);
        fcntl(sockfd, F_SETFL, orgSockStat);
        return 0;
    }


    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;

    tval.tv_sec     = 4;    
    tval.tv_usec    = 0;

    if ( (n = select(sockfd+1, &rset, &wset, NULL, &tval)) == 0)
    {
        // timeout
        errno = ETIMEDOUT;    
        return -1;
    }



    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset) )
    {
        esize = sizeof(int);
        write(sockfd, info, sizeof(CK_SIGNAL_INFO));
        close(sockfd);
        if ((n = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&esize)) < 0)
            return -1;
    }
    else
    {
        perror("Socket Not Set");
        return -1;
    }

	return	0;
}

