
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
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_timer.h"

#undef	__MODULE__
#define	__MODULE__	"ping"

#if defined(__GLIBC__) && (__GLIBC__ >= 2)
#define icmphdr			icmp
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
int options = 0;
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
int sfd;              /* socket file descriptor */
u_char outpack[MAXPACKET];
char BSPACE = '\b';     /* characters written for flood */
char DOT = '.';
static char *hostname;
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
static char *pr_addr(u_long);
static FTM_UINT16	FTM_PING_checksum(u_short *addr, int len);
static FTM_VOID		FTM_PING_catcher(FTM_INT);
static FTM_VOID		FTM_PING_finish(FTM_INT);
static FTM_VOID		FTM_PING_sender(FTM_VOID);
static FTM_RET		FTM_PING_printPacket(FTM_UINT8_PTR	pBuff, int cc, struct sockaddr_in *from, int *num);
static FTM_VOID		FTM_PING_timeValueSub(struct timeval *out, struct timeval *in);
static FTM_VOID		FTM_PING_ICMP_printHeader(struct icmphdr *icp);
static void pr_retip(struct iphdr *ip);

FTM_RET	FTM_PING_check
(
	FTM_CHAR_PTR	pTarget, 
	int *result_num
)
{
	FTM_RET	xRet = FTM_RET_OK;

    struct timeval timeout;
    struct hostent *hp;
    struct sockaddr_in *to;
    struct protoent *proto;
    struct in_addr ifaddr;
    int i;
    int fdmask, hold, packlen, preload;
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
    if (!(proto = getprotobyname("icmp"))) 
	{
        xRet = FTM_RET_PING_UNKNOWN_PROTOCOL_ICMP;
        ERROR(xRet, "Unknown protocol ICMP.");
        return xRet;
    }

    sfd = socket(AF_INET, SOCK_RAW, proto->p_proto);
    if (sfd < 0) 
	{
        if (errno==EPERM) 
		{
			xRet = FTM_RET_PING_MUST_RUN_AS_ROOT;
			ERROR(xRet, "Ping must run as root!");	
        }
        else 
		{
			xRet = FTM_RET_PING_SOCKET_ERROR;
			ERROR(xRet, "Ping socket error!");
		}

		sfd = 0;

		goto finished;
    }

    npackets = count;

    preload = 0;
    datap = &outpack[8 + sizeof(struct timeval)];



    memset(&whereto, 0, sizeof(struct sockaddr));
////////////////////////
    to = (struct sockaddr_in *)&whereto;
    to->sin_family = AF_INET;
    if (inet_aton(pTarget, &to->sin_addr)) 
	{
        hostname = pTarget;
    }
    else 
	{
        hp = gethostbyname(pTarget);
        if (!hp) 
		{
			xRet = FTM_RET_PING_UNKNOWN_HOST;
            ERROR(xRet, "Ping Unknown host[%s].", pTarget);
			goto finished;
        }

        to->sin_family = hp->h_addrtype;
        if (hp->h_length > (int)sizeof(to->sin_addr)) 
		{
            hp->h_length = sizeof(to->sin_addr);
        }

        memcpy(&to->sin_addr, hp->h_addr, hp->h_length);
        (void)strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
        hostname = hnamebuf;
    }

    if (options & F_FLOOD && options & F_INTERVAL) 
	{
		xRet = FTM_RET_INCOMPATIBLE_OPTIONS;
        ERROR(xRet, "Food and Interval incompatible options.");
		goto finished;
    }

    if (datalen >= (int)sizeof(struct timeval)) /* can we time transfer */
	{
        timing = 1;
	}

    packlen = datalen + MAXIPLEN + MAXICMPLEN;
    packet = malloc((u_int)packlen);
    if (!packet) 
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
        ERROR(xRet, "Failed to alloc packet buffer!");
		goto finished;
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
        (void)setsockopt(sfd, SOL_SOCKET, SO_DEBUG, (char *)&hold, sizeof(hold));
	}

    if (options & F_SO_DONTROUTE)
	{
        (void)setsockopt(sfd, SOL_SOCKET, SO_DONTROUTE, (char *)&hold, sizeof(hold));
	}

    /* this is necessary for broadcast pings to work */
    setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, (char *)&hold, sizeof(hold));

    /* record route option */
    if (options & F_RROUTE) 
	{
#ifdef IP_OPTIONS
        memset(rspace, 0, sizeof(rspace));
        rspace[IPOPT_OPTVAL] = IPOPT_RR;
        rspace[IPOPT_OLEN] = sizeof(rspace)-1;
        rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
        if (setsockopt(sfd, IPPROTO_IP, IP_OPTIONS, rspace, sizeof(rspace)) < 0) 
		{
			xRet = FTM_RET_INCOMPATIBLE_OPTIONS;
            ERROR(xRet, "Record route incompatible option.");
			goto finished;
        }
#else
		xRet = FTM_RET_PING_RECORD_ROUTE_NOT_AVAILABLE;
		ERROR(xRet,"Poing is record route not available in this implementation."); 
		goto finished;
#endif /* IP_OPTIONS */
    }

    
    hold = 48 * 1024;
    (void)setsockopt(sfd, SOL_SOCKET, SO_RCVBUF, (char *)&hold, sizeof(hold));


     struct timeval tv_timeo = { 1, 500000 };  /* 3.5 second */
     (void) setsockopt( sfd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeo, sizeof( tv_timeo));

    /*#if 0*/
    if (moptions & MULTICAST_NOLOOP) 
	{
        if (setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, 1) == -1) 
		{
			xRet = FTM_RET_PING_CANT_DISABLE_MULTICAST_LOOPBACK;
            ERROR(xRet, "Can't disable multicast loopback");
			goto finished;
        }
    }

    if (moptions & MULTICAST_TTL) 
	{
        if (setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, 1) == -1) 
		{
			xRet = FTM_RET_PING_CANT_SET_MULTICAST_TIME_TO_LIVE;
            ERROR(xRet, "Ping can't set multicast time-to-live");
			goto finished;
        }
    }

    if (moptions & MULTICAST_IF) 
	{
        if (setsockopt(sfd, IPPROTO_IP, IP_MULTICAST_IF, &ifaddr, sizeof(ifaddr)) == -1) 
		{
			xRet = FTM_RET_PING_CANT_SET_MULTICAST_SOURCE_INTERFACE;
            ERROR(xRet, "Ping can't set multicast source interface");
			goto finished;
        }
    }
/*
    if (to->sin_family == AF_INET)
        (void)printf("PING %s (%s): %d data bytes\n", hostname,
                inet_ntoa(*(struct in_addr *)&to->sin_addr.s_addr),
                datalen);
    else
        (void)printf("PING %s: %d data bytes\n", hostname, datalen);
*/
    //(void)signal(SIGINT, FTM_PING_finish);
    (void)signal(SIGALRM, FTM_PING_catcher);

    while (preload--)       /* fire off them quickies */
	{
        FTM_PING_sender();
	}

    if ((options & F_FLOOD) == 0)
	{
        FTM_PING_catcher(0);     /* start things going */
	}


    int fail_count = 0;


	FTM_TIMER	xTimeout;

	FTM_TIMER_initS(&xTimeout, 2);
    for (;;) 
	{
        struct sockaddr_in from;
        register int cc;
        socklen_t	fromlen;

        if(fail_count == 2)
		{
            break;
		}

        if (options & F_FLOOD) 
		{
            FTM_PING_sender();
            timeout.tv_sec = 0;
            timeout.tv_usec = 10000;
            fdmask = 1 << sfd;
            if (select(sfd + 1, (fd_set *)&fdmask, (fd_set *)NULL, (fd_set *)NULL, &timeout) < 1)
			{
				INFO("Select error!");
                continue;
			}
        }

        fromlen = sizeof(from);
        if ((cc = recvfrom(sfd, (char *)packet, packlen, 0, (struct sockaddr *)&from, &fromlen)) < 0) 
		{
            if (errno == EINTR)
			{
				INFO("Receive error!");
                continue;
			}

            fail_count++;

            continue;
        }

        FTM_PING_printPacket((FTM_UINT8_PTR)packet, cc, &from, result_num);
#if 0
        if (npackets && nreceived >= npackets)
		{
            break;
		}
#endif
		if (npackets || FTM_TIMER_isExpired(&xTimeout))
		{
			break;
		
		}
    }

finished:
	if (sfd != 0)
	{
		close(sfd);	
	}

    /* NOTREACHED */
    return xRet;
}

///////////////////////
   


void FTM_PING_catcher
(
	int ignore
)
{
    int waittime;

    (void)ignore;

    FTM_PING_sender();
    (void)signal(SIGALRM, FTM_PING_catcher);
    if (!npackets || ntransmitted < npackets)
	{
        alarm((u_int)interval);
    }
    else 
	{
        if (nreceived) 
		{
            waittime = 2 * tmax / 1000;
            if (!waittime)
			{
                waittime = 1;
			}

            if (waittime > MAXWAIT)
			{
                waittime = MAXWAIT;
			}
        } 
		else
		{
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


FTM_VOID	FTM_PING_sender(FTM_VOID)
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

	INFO("send ping...");
    CLR(icp->icmp_seq % mx_dup_ck);

    if (timing)
	{
        gettimeofday((struct timeval *)&outpack[8], (struct timezone *)NULL);
	}

    cc = datalen + 8;           /* skips ICMP portion */

    /* compute ICMP checksum here */
    icp->icmp_cksum = FTM_PING_checksum((u_short *)icp, cc);

    i = sendto(sfd, (char *)outpack, cc, 0, &whereto, sizeof(struct sockaddr));

    if (i < 0 || i != cc)  
	{
        if (i < 0)
		{
			INFO("Failed to send ping[%d] - %s", sfd, strerror(errno));
		}
    }

    if (!(options & F_QUIET) && options & F_FLOOD)
	{
        (void)write(STDOUT_FILENO, &DOT, 1);
	}
}


FTM_RET	FTM_PING_printPacket
(
	FTM_UINT8_PTR	pBuff, 
	int cc, 
	struct sockaddr_in *from, 
	int * num
)
{
	FTM_RET	xRet;
    register struct icmphdr *icp;
    register int i;
    register u_char *cp;
    /*#if 0*/
    register u_long l;
    register int j;
    static int old_rrlen;
    static char old_rr[MAX_IPOPTLEN];
    /*#endif*/
    struct iphdr *ip;
    struct timeval tv, *tp;
    long triptime = 0;
    int hlen;

    (void)gettimeofday(&tv, (struct timezone *)NULL);

    /* Check the IP header */
    ip = (struct iphdr *)pBuff;
    hlen = ip->ip_hl << 2;
    if (cc < datalen + ICMP_MINLEN) 
	{
        if (options & F_VERBOSE)
		{
			xRet = FTM_RET_PACKET_TOO_SHORT;
			ERROR(xRet, "Ping packet too short (%d bytes) from %s\n", cc, inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr));
		}
        return	xRet;
    }

    /* Now the ICMP part */
    cc -= hlen;
    icp = (struct icmphdr *)(pBuff + hlen);

    //log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "icmp type : %d\n",icp->icmp_type);
    if (icp->icmp_type == ICMP_ECHOREPLY) 
	{
        if (icp->icmp_id != ident)
		{
            return	FTM_RET_OK;         /* 'Twas not our ECHO */
		}
        ++nreceived;
        if (timing) 
		{
#ifndef icmp_data
            tp = (struct timeval *)(icp + 1);
#else
            tp = (struct timeval *)icp->icmp_data;
#endif
            FTM_PING_timeValueSub(&tv, tp);
            triptime = tv.tv_sec * 10000 + (tv.tv_usec / 100);
            tsum += triptime;
            if (triptime < tmin)
                tmin = triptime;
            if (triptime > tmax)
                tmax = triptime;
        }

        if (TST(icp->icmp_seq % mx_dup_ck)) 
		{
            ++nrepeats;
            --nreceived;
        } 
		else 
		{
            SET(icp->icmp_seq % mx_dup_ck);
        }

        if (options & F_QUIET)
		{
            return	FTM_RET_OK;
		}

        if (options & F_FLOOD)
		{
            (void)write(STDOUT_FILENO, &BSPACE, 1);
		}
        else 
		{
#ifndef icmp_data
            cp = ((u_char*)(icp + 1) + 8);
#else
            cp = (u_char*)icp->icmp_data + 8;
#endif
#if 0
    register u_char *dp;
            dp = &outpack[8 + sizeof(struct timeval)];
            for (i = 8; i < datalen; ++i, ++cp, ++dp) 
			{
                if (*cp != *dp) 
				{
                    (void)printf("\nwrong data byte #%d should be 0x%x but was 0x%x", i, *dp, *cp);
                    cp = (u_char*)(icp + 1);
                    for (i = 8; i < datalen; ++i, ++cp) 
					{
                        if ((i % 32) == 8)
						{
                            (void)printf("\n\t");
						}
                        (void)printf("%x ", *cp);
                    }

                    break;
                }
            }
#endif
        }
    } 
	else 
	{
        /* We've got something other than an ECHOREPLY */
        if (!(options & F_VERBOSE))
		{
            return FTM_RET_OK;
		}

        (void)printf("%d bytes from %s: ", cc, pr_addr(from->sin_addr.s_addr));
        FTM_PING_ICMP_printHeader(icp);
    }

    /*#if 0*/
    /* Display any IP options */
    cp = (u_char *)pBuff + sizeof(struct iphdr);

    for (; hlen > (int)sizeof(struct iphdr); --hlen, ++cp)
	{
		switch (*cp) 
		{
       	case IPOPT_EOL:
			{
				hlen = 0;
			}
			break;

		case IPOPT_LSRR:
			{
				(void)printf("\nLSRR: ");
				hlen -= 2;
				j = *++cp;
				++cp;
				if (j > IPOPT_MINOFF)
				{
					for (;;) 
					{
						l = *++cp;
						l = (l<<8) + *++cp;
						l = (l<<8) + *++cp;
						l = (l<<8) + *++cp;
						if (l == 0)
						{
							(void)printf("\t0.0.0.0");
						}
						else
						{
							(void)printf("\t%s", pr_addr(ntohl(l)));
						}

						hlen -= 4;
						j -= 4;
						if (j <= IPOPT_MINOFF)
						{
							break;
						}
						(void)putchar('\n');
					}
				}
			}
			break;
		case IPOPT_RR:
			{
				j = *++cp;      /* get length */
				i = *++cp;      /* and pointer */
				hlen -= 2;
				if (i > j)
				{
					i = j;
				}

				i -= IPOPT_MINOFF;
				if (i <= 0)
				{
					continue;
				}

				if (i == old_rrlen && cp == (u_char *)pBuff + sizeof(struct iphdr) + 2 && !memcmp((char *)cp, old_rr, i) && !(options & F_FLOOD)) 
				{
					(void)printf("\t(same route)");
					i = ((i + 3) / 4) * 4;
					hlen -= i;
					cp += i;
					break;
				}

				old_rrlen = i;
				memcpy(old_rr, cp, i);
				(void)printf("\nRR: ");
				for (;;) 
				{
					l = *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					if (l == 0)
					{
						(void)printf("\t0.0.0.0");
					}	
					else
					{
						(void)printf("\t%s", pr_addr(ntohl(l)));
					}
					hlen -= 4;
					i -= 4;

					if (i <= 0)
					{
						break;
					}

					(void)putchar('\n');
				}
			}
			break;
		case IPOPT_NOP:
			{
				(void)printf("\nNOP");
			}
			break;
		default:
			{
				(void)printf("\nunknown option %x", *cp);
			}
			break;
		}
	}

	*num = ip->ip_ttl;
	/*#endif*/

	return	FTM_RET_OK;
}


FTM_UINT16	FTM_PING_checksum
(
	u_short *addr, int len
)
{
	register int nleft = len;
    register u_short *w = addr;
    register int sum = 0;
    u_short answer = 0;

	while (nleft > 1)  
	{
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) 
	{
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */

    return(answer);
}

FTM_VOID	FTM_PING_timeValueSub
(
	struct timeval *out, 
	struct timeval *in
)
{
    if ((out->tv_usec -= in->tv_usec) < 0) 
	{
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}


FTM_VOID	FTM_PING_finish
(
	FTM_INT	ignore
)
{
    (void)ignore;
    return;
}

#ifdef notdef
static char *ttab[] = 
{
    "Echo Reply",       /* ip + seq + udata */
    "Dest Unreachable", /* net, host, proto, port, frag, sr + IP */
    "Source Quench",    /* IP */
    "Redirect",     /* redirect type, gateway, + IP  */
    "Echo",
    "Time Exceeded",    /* transit, frag reassem + IP */
    "Parameter Problem",    /* pointer + IP */
    "Timestamp",        /* id + seq + three timestamps */
    "Timestamp Reply",  /* " */
    "Info Request",     /* id + sq */
    "Info Reply"        /* " */
};
#endif


FTM_VOID	FTM_PING_ICMP_printHeader
(
	struct icmphdr *icp
)
{
    switch(icp->icmp_type) 
	{
    case ICMP_ECHOREPLY:
		(void)printf("Echo Reply\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_DEST_UNREACH:
		switch(icp->icmp_code) 
		{
		case ICMP_NET_UNREACH:
			(void)printf("Destination Net Unreachable\n");
			break;
		case ICMP_HOST_UNREACH:
			(void)printf("Destination Host Unreachable\n");
			break;
		case ICMP_PROT_UNREACH:
			(void)printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_PORT_UNREACH:
			(void)printf("Destination Port Unreachable\n");
			break;
		case ICMP_FRAG_NEEDED:
			(void)printf("frag needed and DF set\n");
			break;
		case ICMP_SR_FAILED:
			(void)printf("Source Route Failed\n");
			break;
		case ICMP_NET_UNKNOWN:
			(void)printf("Network Unknown\n");
			break;
		case ICMP_HOST_UNKNOWN:
			(void)printf("Host Unknown\n");
			break;
		case ICMP_HOST_ISOLATED:
			(void)printf("Host Isolated\n");
			break;
		case ICMP_NET_UNR_TOS:
			printf("Destination Network Unreachable At This TOS\n");
			break;
		case ICMP_HOST_UNR_TOS:
			printf("Destination Host Unreachable At This TOS\n");
			break;
#ifdef ICMP_PKT_FILTERED
		case ICMP_PKT_FILTERED:
			(void)printf("Packet Filtered\n");
			break;
#endif
#ifdef ICMP_PREC_VIOLATION
		case ICMP_PREC_VIOLATION:
			(void)printf("Precedence Violation\n");
			break;
#endif
#ifdef ICMP_PREC_CUTOFF
		case ICMP_PREC_CUTOFF:
			(void)printf("Precedence Cutoff\n");
			break;
#endif
		default:
			(void)printf("Dest Unreachable, Unknown Code: %d\n",
					icp->icmp_code);
			break;
		}
		/* Print returned IP header information */
#ifndef icmp_data
		pr_retip((struct iphdr *)(icp + 1));
#else
		pr_retip((struct iphdr *)icp->icmp_data);
#endif
		break;
	case ICMP_SOURCE_QUENCH:
		(void)printf("Source Quench\n");
#ifndef icmp_data
		pr_retip((struct iphdr *)(icp + 1));
#else
		pr_retip((struct iphdr *)icp->icmp_data);
#endif
		break;
	case ICMP_REDIRECT:
		switch(icp->icmp_code) 
		{
		case ICMP_REDIR_NET:
			(void)printf("Redirect Network");
			break;
		case ICMP_REDIR_HOST:
			(void)printf("Redirect Host");
			break;
		case ICMP_REDIR_NETTOS:
			(void)printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIR_HOSTTOS:
			(void)printf("Redirect Type of Service and Host");
			break;
		default:
			(void)printf("Redirect, Bad Code: %d", icp->icmp_code);
			break;
		}
		(void)printf("(New addr: %s)\n", 
				inet_ntoa(icp->icmp_gwaddr));
#ifndef icmp_data
		pr_retip((struct iphdr *)(icp + 1));
#else
		pr_retip((struct iphdr *)icp->icmp_data);
#endif
		break;

	case ICMP_ECHO:
		(void)printf("Echo Request\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_TIME_EXCEEDED:
		switch(icp->icmp_code) 
		{
		case ICMP_EXC_TTL:
			(void)printf("Time to live exceeded\n");
			break;
		case ICMP_EXC_FRAGTIME:
			(void)printf("Frag reassembly time exceeded\n");
			break;
		default:
			(void)printf("Time exceeded, Bad Code: %d\n",
					icp->icmp_code);
			break;
		}
#ifndef icmp_data
		pr_retip((struct iphdr *)(icp + 1));
#else
		pr_retip((struct iphdr *)icp->icmp_data);
#endif
		break;
	case ICMP_PARAMETERPROB:
		(void)printf("Parameter problem: IP address = %s\n",
				inet_ntoa (icp->icmp_gwaddr));
#ifndef icmp_data
		pr_retip((struct iphdr *)(icp + 1));
#else
		pr_retip((struct iphdr *)icp->icmp_data);
#endif
		break;
	case ICMP_TIMESTAMP:
		(void)printf("Timestamp\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_TIMESTAMPREPLY:
		(void)printf("Timestamp Reply\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_INFO_REQUEST:
		(void)printf("Information Request\n");
		/* XXX ID + Seq */
		break;
	case ICMP_INFO_REPLY:
		(void)printf("Information Reply\n");
		/* XXX ID + Seq */
		break;
#ifdef ICMP_MASKREQ
	case ICMP_MASKREQ:
		(void)printf("Address Mask Request\n");
		break;
#endif
#ifdef ICMP_MASKREPLY
	case ICMP_MASKREPLY:
		(void)printf("Address Mask Reply\n");
		break;
#endif
	default:
		(void)printf("Bad ICMP type: %d\n", icp->icmp_type);
	}
}


void pr_iph
(
	struct iphdr *ip
)
{
	int hlen;
	u_char *cp;

	hlen = ip->ip_hl << 2;
	cp = (u_char *)ip + 20;     /* point to options */

	(void)printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
	(void)printf(" %1x  %1x  %02x %04x %04x",
			ip->ip_v, ip->ip_hl, ip->ip_tos, ip->ip_len, ip->ip_id);
	(void)printf("   %1x %04x", ((ip->ip_off) & 0xe000) >> 13,
			(ip->ip_off) & 0x1fff);
	(void)printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ip->ip_sum);
	(void)printf(" %s ", inet_ntoa(*((struct in_addr *) &ip->ip_src)));
	(void)printf(" %s ", inet_ntoa(*((struct in_addr *) &ip->ip_dst)));
	/* dump and option bytes */
	while (hlen-- > 20) 
	{
		(void)printf("%02x", *cp++);
	}
	(void)putchar('\n');
}


char * pr_addr(u_long l)
{
	struct hostent *hp;
	static char buf[256];

	if ((options & F_NUMERIC) ||
			!(hp = gethostbyaddr((char *)&l, 4, AF_INET)))
		(void)snprintf(buf, sizeof(buf), "%s", 
				inet_ntoa(*(struct in_addr *)&l));
	else
		(void)snprintf(buf, sizeof(buf), "%s (%s)", hp->h_name,
				inet_ntoa(*(struct in_addr *)&l));
	return(buf);
}


void pr_retip(struct iphdr *ip)
{
	int hlen;
	u_char *cp;

	pr_iph(ip);
	hlen = ip->ip_hl << 2;
	cp = (u_char *)ip + hlen;

	if (ip->ip_p == 6)
		(void)printf("TCP: from port %u, to port %u (decimal)\n",
				(*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
	else if (ip->ip_p == 17)
		(void)printf("UDP: from port %u, to port %u (decimal)\n",
				(*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
}
