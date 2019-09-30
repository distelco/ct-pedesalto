#if !defined(__COMUNICATION_H__)

#define __COMUNICATION_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

enum E_tcp_state
{
        E_tcp_disconnected,
        E_tcp_connecting,
        E_tcp_connected
} ;

struct S_tcp {
	char	ip[64];
	int 	port;
	int 	sock;
        enum E_tcp_state     state;  
	long	timeout;
};

static inline void init_tcp_struct( struct S_tcp *ptcp, char *pip, int p, long t)
{
	if( pip != NULL)
		strcpy(ptcp->ip, pip);
	if(p>0)
		ptcp->port = p;
		
	if(t>0) {
		ptcp->timeout = t;
	} else {
		ptcp->timeout = 100;
	}		
	ptcp->sock = -1;
	return;
}

//int client_connect (char *arg, int port);
int tcp_connect(struct S_tcp *p);
void tcp_disconnect(struct S_tcp *p );

#endif
