/*
	comunication.c
	Contiene le funzioni per la comunicazione tra cl ed il mondo esterno
	Written by Franco Novi
	Distelco s.r.l.
	$Id: comunication.c,v 1.3 2009-01-29 10:32:55 matteoc Exp $
*/

#undef DEBUG

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"
#include "comunication.h"

static int client_connect (char *arg, int port)
{
	int ret;
	struct sockaddr_in serv_addr;
	struct hostent *he;
	struct protoent *pe;
	int sockfd;
	int flags ;

	//Esegue la connessione del socket
	//inizializza il socket di connessione
	//setup dest address
	he = gethostbyname (arg);
	if (he == NULL)
	{
		ERR_LOG ("client_connect:gethostbyname:%s\n", strerror (errno));
		return -1;
	}

	pe = getprotobyname ("tcp");
	if (pe == NULL)
	{
		ERR_LOG ("client_connect:getprotobyname:%s\n", strerror (errno));
		return -1;
	}

	//memcpy (&(serv_addr.sin_addr.s_addr), he->h_addr, sizeof (long));
	memcpy (&(serv_addr.sin_addr), he->h_addr, sizeof (struct in_addr));
	serv_addr.sin_port = htons (port);
	serv_addr.sin_family = AF_INET;
	bzero (&(serv_addr.sin_zero), 8);

	//setup server socket
	//sockfd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		ERR_LOG ("client_connect:ip %s : %s",arg, strerror (errno));
		return -1;
	}
	
	flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1)
	{
		ERR_LOG ("client connect:ip %s : %s",arg, strerror (errno));
		close (sockfd);
		return -1;
	}
	if ( fcntl( sockfd, F_SETFL, flags | O_NONBLOCK ) < 0 )
	{
		ERR_LOG ("client_connect:ip %s : %s",arg, strerror (errno));
		close (sockfd);
		return -1;	
	}

	flags = 1 ;
	setsockopt (sockfd, SOL_TCP, TCP_NODELAY, &flags, sizeof (int));

	ret = connect (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr));
	if (ret < 0)
	{
		if ( errno != EINPROGRESS ) 
		{
			ERR_LOG ("client connect:ip: %s :%s",arg, strerror (errno));
			close (sockfd);
			return -1;
		}
	}

	return sockfd;
}

void tcp_disconnect(struct S_tcp *p )
{
	DEBUG2_LOG("tcp_disconnect: closing : socket %d : ip %s",p->sock, p->ip ) ;
	close(p->sock);
        p->state = E_tcp_disconnected ;
	p->sock = -1;
	return;
}


int tcp_connected( struct S_tcp *p )
{
	struct pollfd ufds;
	int ret ;
	int optval ;
	socklen_t optlen ;

	if(p->sock < 0)
	{
		DEBUG2_LOG( "tcp_connected:Connect start" );
		p->sock = client_connect(p->ip, p->port) ;
	}
	if(p->sock < 0)
	{
		DEBUG2_LOG( "tcp_connected:Connect failed" );
                p->state = E_tcp_disconnected ;
		return -1 ;
	}
	//DEBUG_LOG( "tcp_connected:Try connect socket %d : ip %s",p->sock, p->ip );
	ufds.fd = p->sock;
	ufds.events = POLLIN | POLLOUT;
	if ((ret = poll (&ufds, 1, 0)) < 0)
	{
		DEBUG2_LOG( "tcp_connected:Poll error : socket %d : ip %s",p->sock, p->ip );
                tcp_disconnect( p ) ;
		return -1 ;
	}
	if ( (ufds.revents & POLLOUT) || (ufds.revents & POLLIN))
	{
		optval = 1 ;
		optlen = sizeof(int) ;
		ret = getsockopt( p->sock, SOL_SOCKET, SO_ERROR, &optval, &optlen ) ;
		if ( ret < 0 )
		{
			DEBUG2_LOG("tcp_connected:getsockopt error : socket %d : ip %s",p->sock, p->ip ) ;
                        tcp_disconnect( p ) ;
			return -1 ;
		}
		if ( optval != 0 ) 
		{
			DEBUG2_LOG("tcp_connected:Socket not connected : socket %d : ip %s",p->sock, p->ip ) ;
                        tcp_disconnect( p ) ;
			return -1 ;
		}
		DEBUG2_LOG("tcp_connected:Socket connected : socket %d : ip %s",p->sock, p->ip ) ;
                p->state = E_tcp_connected ;
		return p->sock ;
	} else {
		DEBUG2_LOG("tcp_connected:Socket connecting : socket %d : ip %s",p->sock, p->ip ) ;
                p->state = E_tcp_connecting ;
		return -1 ;
	}
}

/*
	Apre il socket se non e' aperto, ritorna il socket descriptor
*/
int tcp_connect( struct S_tcp *p)
{
	return tcp_connected( p ) ;
}


