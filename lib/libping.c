
#include <sys/time.h>
#include <unistd.h>

char copyright[] =
  "@(#) Copyright (c) 1989 The Regents of the University of California.\n"
  "All rights reserved.\n";
char rcsid[] = "$Id: ping.c,v 1.22 1997/06/08 19:39:47 dholland Exp $";
char pkg[] = "netkit-base-0.10";



#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <common.h>
#include "ftm_trace.h"

#ifdef __linux__
#define SAFE_TO_DROP_ROOT
#endif

#if defined(__GLIBC__) && (__GLIBC__ >= 2)
#define icmphdr			icmp
//#define ICMP_DEST_UNREACH	ICMP_UNREACH
//#define ICMP_NET_UNREACH	ICMP_UNREACH_NET
//#define ICMP_HOST_UNREACH	ICMP_UNREACH_HOST
//#define ICMP_PORT_UNREACH	ICMP_UNREACH_PORT
//#define ICMP_PROT_UNREACH	ICMP_UNREACH_PROTOCOL
//#define ICMP_FRAG_NEEDED	ICMP_UNREACH_NEEDFRAG
//#define ICMP_SR_FAILED		ICMP_UNREACH_SRCFAIL
//#define ICMP_NET_UNKNOWN	ICMP_UNREACH_NET_UNKNOWN
//#define ICMP_HOST_UNKNOWN	ICMP_UNREACH_HOST_UNKNOWN
//#define ICMP_HOST_ISOLATED	ICMP_UNREACH_ISOLATED
//#define ICMP_NET_UNR_TOS	ICMP_UNREACH_TOSNET
//#define ICMP_HOST_UNR_TOS	ICMP_UNREACH_TOSHOST
//#define ICMP_SOURCE_QUENCH	ICMP_SOURCEQUENCH
//#define ICMP_REDIR_NET		ICMP_REDIRECT_NET
//#define ICMP_REDIR_HOST		ICMP_REDIRECT_HOST
//#define ICMP_REDIR_NETTOS	ICMP_REDIRECT_TOSNET
//#define ICMP_REDIR_HOSTTOS	ICMP_REDIRECT_TOSHOST
//#define ICMP_TIME_EXCEEDED	ICMP_TIMXCEED
//#define ICMP_EXC_TTL		ICMP_TIMXCEED_INTRANS
//#define ICMP_EXC_FRAGTIME	ICMP_TIMXCEED_REASS
//#define	ICMP_PARAMETERPROB	ICMP_PARAMPROB
//#define ICMP_TIMESTAMP		ICMP_TSTAMP
//#define ICMP_TIMESTAMPREPLY	ICMP_TSTAMPREPLY
//#define ICMP_INFO_REQUEST	ICMP_IREQ
//#define ICMP_INFO_REPLY		ICMP_IREQREPLY
#else
#define ICMP_MINLEN	28
#define inet_ntoa(x) inet_ntoa(*((struct in_addr *)&(x)))
#endif


#define	DEFDATALEN	(64 - 8)	/* default data length */
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	MAXPACKET	(65536 - 60 - 8)/* max packet size */
#define	MAXWAIT		10		/* max seconds to wait for response */
#define	NROUTES		9		/* number of record route slots */

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))

/* various options */
int options;
#define	F_FLOOD		0x001
#define	F_INTERVAL	0x002
#define	F_NUMERIC	0x004
#define	F_PINGFILLED	0x008
#define	F_QUIET		0x010
#define	F_RROUTE	0x020
#define	F_SO_DEBUG	0x040
#define	F_SO_DONTROUTE	0x080
#define	F_VERBOSE	0x100

/* multicast options */
int moptions;
#define MULTICAST_NOLOOP	0x001
#define MULTICAST_TTL		0x002
#define MULTICAST_IF		0x004




#define MAX_DUP_CHK (8 * 128)
int mx_dup_ck = MAX_DUP_CHK;
char rcvd_tbl[MAX_DUP_CHK / 8];

struct sockaddr whereto;    /* who to ping */
int datalen = DEFDATALEN;
int s;              /* socket file descriptor */
u_char outpack[MAXPACKET];
char BSPACE = '\b';     /* characters written for flood */
char DOT = '.';
static char *pHostName = NULL;
static int ident;       /* process id to identify our packets */

/* counters */
static long npackets;       /* max packets to transmit */
static long nreceived;      /* # of packets we got back */
static long nrepeats;       /* number of duplicates */
static long ntransmitted;   /* sequence # for outbound packets = #sent */
static int interval = 1;    /* interval between packets */

/* timing */
static int timing;      /* flag to do timing */
static long tmin = LONG_MAX;    /* minimum round trip time */
static long tmax = 0;       /* maximum round trip time */
static u_long tsum;     /* sum of all times, for doing average */

/* protos */
static int in_cksum(u_short *addr, int len);
static FTM_VOID	FTM_PING_catcher(FTM_INT nIgnore);
static FTM_VOID	FTM_PING_finish(FTM_INT	nIgnore);
static FTM_VOID	FTM_PING_pinger(FTM_VOID);

FTM_INT FTM_PING_check
(
	FTM_CHAR_PTR	pTargetIP, 
	FTM_UINT32_PTR	pReplyCount
)
{
    struct hostent *hp;
    struct sockaddr_in *to;
    struct protoent *proto;
    struct in_addr ifaddr;
    int i;
    int fdmask, hold, packlen;
    u_char *datap, *packet;
    char hnamebuf[MAXHOSTNAMELEN];
    u_char ttl, loop;
#ifdef IP_OPTIONS
    char rspace[3 + 4 * NROUTES + 1];   /* record route space */
#endif

    int count = 2;
    static char *null = NULL;
    __environ = &null;

    /*
     * Pull this stuff up front so we can drop root if desired.
     */
    if (!(proto = getprotobyname("icmp"))) {
        (void)fprintf(stderr, "ping: unknown protocol icmp.\n");
        return 0;
    }
    if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
        if (errno==EPERM) {
            fprintf(stderr, "ping: ping must run as root\n");
        }
        else perror("ping: socket");
        return 0;
    }

#ifdef SAFE_TO_DROP_ROOT
    setuid(getuid());
#endif


    npackets = count;

    datap = &outpack[8 + sizeof(struct timeval)];



    TRACE("PING : %s\n", pTargetIP);
    memset(&whereto, 0, sizeof(struct sockaddr));

////////////////////////
    to = (struct sockaddr_in *)&whereto;
    to->sin_family = AF_INET;
    if (inet_aton(pTargetIP, &to->sin_addr)) 
	{
        pHostName = pTargetIP;
    }
    else 
	{
        hp = gethostbyname(pTargetIP);
        if (!hp) 
		{
            (void)fprintf(stderr, "ping: unknown host %s\n", pTargetIP);
            return 0;
        }

        to->sin_family = hp->h_addrtype;

        if (hp->h_length > (int)sizeof(to->sin_addr)) 
		{
            hp->h_length = sizeof(to->sin_addr);
        }

        memcpy(&to->sin_addr, hp->h_addr, hp->h_length);
        (void)strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
        pHostName = hnamebuf;
    }

    if (options & F_FLOOD && options & F_INTERVAL) 
	{
        (void)fprintf(stderr,
                "ping: -f and -i incompatible options.\n");
        return 0;;
    }

    if (datalen >= (int)sizeof(struct timeval)) /* can we time transfer */
	{
        timing = 1;
	}

    packlen = datalen + MAXIPLEN + MAXICMPLEN;
    packet = malloc((u_int)packlen);

    if (!packet) 
	{
        (void)fprintf(stderr, "ping: out of memory.\n");
        return 0;
    }
    if (!(options & F_PINGFILLED))
	{
        for (i = 8; i < datalen; ++i)
		{
            *datap++ = i;
		}
	}

    ident = getpid() & 0xFFFF;
    hold = 1;

    if (options & F_SO_DEBUG)
	{
        (void)setsockopt(s, SOL_SOCKET, SO_DEBUG, (char *)&hold, sizeof(hold));
	}

    if (options & F_SO_DONTROUTE)
	{
        (void)setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (char *)&hold, sizeof(hold));
	}

    /* this is necessary for broadcast pings to work */
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&hold, sizeof(hold));

    /* record route option */
    if (options & F_RROUTE) 
	{
#ifdef IP_OPTIONS
        memset(rspace, 0, sizeof(rspace));
        rspace[IPOPT_OPTVAL] = IPOPT_RR;
        rspace[IPOPT_OLEN] = sizeof(rspace)-1;
        rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
        if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, rspace,
                    sizeof(rspace)) < 0) {
            perror("ping: record route");
            return 0;
        }
#else
        (void)fprintf(stderr,
                "ping: record route not available in this implementation.\n");
        return 0;
#endif /* IP_OPTIONS */
    }

    
    hold = 48 * 1024;
    (void)setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&hold, sizeof(hold));


     struct timeval tv_timeo = { 1, 500000 };  /* 3.5 second */
     (void) setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, &tv_timeo, sizeof( tv_timeo));

    /*#if 0*/
    if (moptions & MULTICAST_NOLOOP) 
	{
        if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, 1) == -1) 
		{
            perror ("can't disable multicast loopback");
            return 0;
        }
    }

    if (moptions & MULTICAST_TTL) 
	{
        if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, 1) == -1) 
		{
            perror ("can't set multicast time-to-live");
            return 0;
        }
    }

    if (moptions & MULTICAST_IF) 
	{
        if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &ifaddr, sizeof(ifaddr)) == -1) 
		{
            perror ("can't set multicast source interface");
            return 0;
        }
    }
    /*#endif*/

    (void)signal(SIGALRM, FTM_PING_catcher);

	FTM_PING_pinger();

    if ((options & F_FLOOD) == 0)
	{
        FTM_PING_catcher(0);     /* start things going */
	}


    int fail_count = 0;


    for (;;) 
	{
        struct sockaddr_in xFrom;
        register int cc;
        socklen_t nFromLen;

        if(fail_count == 2)
            break;

        if (options & F_FLOOD) 
		{
    		struct timeval xTimeout;

            FTM_PING_pinger();
            xTimeout.tv_sec = 0;
            xTimeout.tv_usec = 10000;
            fdmask = 1 << s;

            if (select(s + 1, (fd_set *)&fdmask, (fd_set *)NULL, (fd_set *)NULL, &xTimeout) < 1)
			{
                continue;
			}
        }

        nFromLen = sizeof(xFrom);
        if ((cc = recvfrom(s, (char *)packet, packlen, 0, (struct sockaddr *)&xFrom, &nFromLen)) < 0) 
		{
            if (errno == EINTR)
			{
                continue;
			}

            perror("ping: recvfrom");
            fail_count++;
            continue;
        }

        if (npackets && nreceived >= npackets)
		{
            break;
		}
    }
    /* NOTREACHED */
    return 0;
}

///////////////////////
   


FTM_VOID	FTM_PING_catcher
(
	FTM_INT	nIgnore
)
{
    int waittime;

    (void)nIgnore;

    FTM_PING_pinger();
    (void)signal(SIGALRM, FTM_PING_catcher);
    if (!npackets || ntransmitted < npackets){
        alarm((u_int)interval);
    }
    else {
        if (nreceived) {
            waittime = 2 * tmax / 1000;
            if (!waittime)
                waittime = 1;
            if (waittime > MAXWAIT)
                waittime = MAXWAIT;
        } else{
            waittime = MAXWAIT;
        }
        (void)signal(SIGALRM, FTM_PING_finish);
        (void)alarm((u_int)waittime);
    }
}

#if !defined(__GLIBC__) || (__GLIBC__ < 2)
#define icmp_type type
#define icmp_code code
#define icmp_cksum checksum
#define icmp_id un.echo.id
#define icmp_seq un.echo.sequence
#define icmp_gwaddr un.gateway
#endif /* __GLIBC__ */

#define ip_hl ihl
#define ip_v version
#define ip_tos tos
#define ip_len tot_len
#define ip_id id
#define ip_off frag_off
#define ip_ttl ttl
#define ip_p protocol
#define ip_sum check
#define ip_src saddr
#define ip_dst daddr


FTM_VOID FTM_PING_pinger(FTM_VOID)
{
    register struct icmphdr *icp;
    register int cc;
    int i;

    icp = (struct icmphdr *)outpack;
    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq = ntransmitted++;
    icp->icmp_id = ident;           /* ID */

    CLR(icp->icmp_seq % mx_dup_ck);

    if (timing)
        (void)gettimeofday((struct timeval *)&outpack[8],
                (struct timezone *)NULL);

    cc = datalen + 8;           /* skips ICMP portion */

    /* compute ICMP checksum here */
    icp->icmp_cksum = in_cksum((u_short *)icp, cc);

    i = sendto(s, (char *)outpack, cc, 0, &whereto,
            sizeof(struct sockaddr));

    if (i < 0 || i != cc)  
	{
        if (i < 0)
		{
            perror("ping: sendto");
		}
		TRACE("PING : wrote %s %d chars, ret=%d\n", pHostName, cc, i);
    }

    if (!(options & F_QUIET) && options & F_FLOOD)
	{
        (void)write(STDOUT_FILENO, &DOT, 1);
	}
}


int
in_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register int sum = 0;
    u_short answer = 0;

        while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(u_char *)(&answer) = *(u_char *)w ;
        sum += answer;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return(answer);
}

FTM_VOID FTM_PING_finish
(
	FTM_INT	nIgnore
)
{
    (void)nIgnore;

    //(void)signal(SIGINT, SIG_IGN);
    (void)putchar('\n');
    (void)fflush(stdout);
    (void)printf("--- %s ping statistics ---\n", pHostName);
    (void)printf("%ld packets transmitted, ", ntransmitted);
    (void)printf("%ld packets received, ", nreceived);
    if (nrepeats)
	{
        (void)printf("+%ld duplicates, ", nrepeats);
	}

    if (ntransmitted)
	{
        if (nreceived > ntransmitted)
            (void)printf("-- somebody's printing up packets!");
        else
            (void)printf("%d%% packet loss",
                    (int) (((ntransmitted - nreceived) * 100) /
                        ntransmitted));
	}
    (void)putchar('\n');

    if (nreceived && timing)
	{
        (void)printf("round-trip min/avg/max = %ld.%ld/%lu.%ld/%ld.%ld ms\n",
                tmin/10, tmin%10,
                (tsum / (nreceived + nrepeats))/10,
                (tsum / (nreceived + nrepeats))%10,
                tmax/10, tmax%10);
	}

    if (nreceived==0) return;
    return;
}



