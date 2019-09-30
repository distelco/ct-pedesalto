/*
	commands.c
	contiene parser, comandi etc. per l'interfaccia comandi
	Written by Franco Novi
	Distelco s.r.l.
	$Id: commands.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/
//#undef DEBUG
#include <stdarg.h>
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "mymemory.h"
#include "commands.h"
#include "extern.h"

#undef CMDAPI
#if 0
#define CMDAPI( name, code )							\
	extern int name##_f ( char *args, char *resp, int len, char *cod );	\
	/* function pointer */							\
	typedef int (* name##_t) ( char *args, char *resp, int len, char *cod );\
	extern name##_t name;
	
#define CMDAPI_IMPL( name, code )						\
	extern int name##_f ( char *args, char *resp, int len, char *cod );	\
	name##_t name = name##_f ;						\
	extern int name##_f ( char *args, char *resp, int len, char *cod )
#endif
	
#define CMDAPI_IMPL( name, code )	int name( char *args, char *resp, int len, char *cod )
#define CMDAPI( name, code )		{ (func_addr)name, #name, code },

typedef int (*func_addr) (const char *args, char *resp, int len, char *c);
struct S_funct
{
	func_addr  func;
	const char *name;
	const char *code;
	
};
extern struct S_funct commands[] ;

#define MKOKRESP(fmt, varg...) snprintf(resp, len, "%s " fmt " ", cod, ##varg) ;
#define MKOKRESPNOCH(fmt, varg...) snprintf(resp, len, "%s " fmt " ", cod, ##varg) ;
#define MKERRRESP(fmt, varg...) snprintf(resp, len, "%s -1 " fmt " ", cod, ##varg) ;

#ifndef TRUE
	#define FALSE	0
	#define TRUE	1
#endif


/*
 function used to process commands
*/

static int
parse_command (char *line, char *resp, int len)
{
	int res, i;		// < 0 error, = 0 Ok, > 0 OK no msg
	const char *p;
	char arg[20];
	char cmd[10];
	int cmd_not_found;

	if( resp == NULL )
		return 0;
	strcpy (resp, "0");
	
	p = line;
	while (isspace (*p))
		p++;
	if (*p == '\0' || *p == '#')
		return 0;

	get_arg (arg, sizeof (arg), &p);
/*
	if (!strcasecmp (arg, "quit"))
	{
		return -2;		
	}
	else
*/
	{
		// look for the command number
		//cmd = atoi (arg);
		strcpy(cmd, arg);
		i = 0;
		res = -1;
		cmd_not_found = 1;
		while (commands[i].code)
		{
			//if (commands[i].code == cmd && commands[i].func != NULL)
			if ( (!strcmp( commands[i].code, cmd )) && commands[i].func != NULL)
			{
				cmd_not_found = 0;
				res = (commands[i].func)(p, resp, len, cmd);
				break;
			}
			i++;
		}
		if (cmd_not_found)
		{
			ERR_LOG ("Command not found: %s", cmd);
			/* manda il codice del comando e l'errore */
			snprintf (resp, len, "%s -2 ", cmd);
		}
	}
	return 0;
}

#define MAX_RESP 1024
#define MAX_CMD	 1024

/**************************************************************************
 Questo thread si occupa di ascoltare i comandi, permette multiconnessioni 
***************************************************************************/
#define CMDPORT		15444
static int sock = -1;	// listen socket
static int sockfd = -1;	// connect socket
static FILE * sock_f;


static void
listen_multinet_command_cancel( void *p)
{

	DEBUG_LOG( "listen_multinet_command_cancel: executing" );	
	if ( sockfd != -1 )
	{
		shutdown( sockfd, SHUT_RDWR );
		close( sockfd ) ;
	}
	if ( sock != -1 )
	{
		shutdown( sock, SHUT_RDWR );
		close( sock ) ;
	}
	return ;
}

void *
listen_multinet_command (void *arg)
{
	char *resp;
	char *mesg;
	struct pollfd ufds;
	struct sockaddr_in sock_from;
	int i;
	
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL ) ;
	pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL ) ;
			
	pthread_cleanup_push( listen_multinet_command_cancel, NULL ) ;

	// Incremento il contatore dei thread attivi    
	th_count++;
	
	sockfd = -1;
	sock = -1;
	// create socket to host
	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		ERR_LOG ("listen_multinet_command:socket creation error");
		pthread_exit (0);
	}

	i = 1 ;
	if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i) ) != 0 )
	{
		ERR_LOG("listen_multinet_command:cannot set socket options");
	}

	/* setup source */
	sock_from.sin_family = AF_INET;
	sock_from.sin_port = htons (CMDPORT);
	sock_from.sin_addr.s_addr = INADDR_ANY;
	bzero (&(sock_from.sin_zero), 8);

	if (bind (sock, (struct sockaddr *) &sock_from, sizeof (struct sockaddr)) == -1)
	{
		close (sock);
		sock = -1;
		ERR_LOG ("listen_multinet_command:port already in use");
		end_flag = 1;
	}
	while(!end_flag)
	{
		if (listen (sock, 1) != 0)
		{
			close (sock);
			sockfd = -1;
			sock_f = NULL ;
			ERR_LOG ("listen_multinet_command:Commands Listen error");
		}
		DEBUG_LOG ("listen_multinet_command:Commands thread Listen");

		// Alloco lo spazio del buffer di risposta e del buffer del comando
		resp = (char *) malloc( MAX_RESP );
		mesg = (char *) malloc( MAX_CMD );
	
		if( resp == NULL )
		{
			close (sock);
			sockfd = -1;
			sock_f = NULL ;
			ERR_LOG("listen_multinet_command:Allocation of resp space failed");
			pthread_exit (0);
		}
		if( mesg == NULL )
		{
			close (sock);
			sockfd = -1;
			sock_f = NULL ;
			ERR_LOG("listen_multinet_command:Allocation of mesg space failed");
			pthread_exit (0);
		}
		
		if ((sockfd = accept (sock, NULL, 0)) == -1)
		{
			close(sockfd);
			sockfd=-1;
			sock_f = NULL ;
			ERR_LOG ("listen_multinet_command:Cannot accept connection");
			continue;
		}
		
		//INFO_LOG ("listen_multinet_command:Commands Accept");
		if( (sock_f = fdopen(sockfd,"r+")) == NULL )
		{
			close(sockfd);
			sockfd=-1;
			sock_f = NULL ;
			ERR_LOG("listen_multinet_command:fdopen fail");
			continue;
		}
		fputs ("Interprete comandi:", sock_f);
		
		while (1)
		{
			if( fputs ("\n\r cmd> ", sock_f) == EOF )
			{
				fclose(sock_f);
				close(sockfd);
				sock_f = NULL ;
				sockfd = -1;
				break;			
			}
			
			fflush (sock_f);
			ufds.fd = sockfd ;
			ufds.events = POLLIN | POLLPRI;
			i = poll (&ufds, 1, (600*1000) );
			if (i == 0)
			{
				fputs( "timeout close", sock_f);
				fflush (sock_f);
				fclose(sock_f);
				close(sockfd);
				sock_f = NULL ;
				sockfd = -1;
				DEBUG_LOG ("listen_multinet_command:timeout close");
				break;				
			}
			if ( fgets (mesg, MAX_CMD, sock_f) == NULL)
			{
				fclose(sock_f);
				close(sockfd);
				sock_f = NULL ;
				sockfd = -1;
				break;
			}
			if( parse_command (mesg, resp, MAX_RESP ) == -2 )
			{
				end_flag = 1;
				shutdown(sockfd, SHUT_RDWR );
				close(sockfd);
				sock_f = NULL ;
				sockfd = -1;
				break;
			}
			if( fputs (resp, sock_f) == EOF )
			{
				close(sockfd);
				sock_f = NULL ;
				sockfd = -1;
				break;			
			}
			memset (mesg, 0, MAX_CMD);
		}
		
		free( resp ); 
		free( mesg ); 
		
	}
	close(sockfd);
	sock_f = NULL ;
	sockfd = -1;
	DEBUG_LOG ("listen_multinet_command:exit");
	
	shutdown(sock, SHUT_RDWR );
	close(sock);
	pthread_cleanup_pop( 0 ) ;
	pthread_exit (0);
}

void output_command_message( char *fmt, va_list ap )
{
	if ( sockfd != -1 && sock_f != NULL )
	{
		vfprintf( sock_f, fmt, ap ) ;
	}
	return ;
}


CMDAPI_IMPL ( read_in_cell, "rdi" )
{
	char cmd[20] ;
	const char *p = args ;
	int v ;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	if (v < 0 )
	{
		MKERRRESP("Bad Cell number");
		return -1;
	}

	MKOKRESP("%d %d", v, (int)read_data(IS(v)));
	return 0 ;
}

CMDAPI_IMPL( read_in_reg, "rai" )
{
	char cmd[20] ;
	const char *p = args ;
	int v;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	if (v < 0 )
	{
		MKERRRESP("Bad reg number");
		return -1;
	}

	MKOKRESP("%d %d", v, (int) read_data(IR(v)) ) ;
	return 0 ;
}

CMDAPI_IMPL ( read_out_cell, "rdo" )
{
	char cmd[20] ;
	const char *p = args ;
	int v ;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	if (v < 0 )
	{
		MKERRRESP("Bad Cell number");
		return -1;
	}

	MKOKRESP("%d %d", v, (int)read_data(CS(v)));
	return 0 ;
}

CMDAPI_IMPL( read_out_reg, "rao" )
{
	char cmd[20] ;
	const char *p = args ;
	int v;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	if (v < 0 )
	{
		MKERRRESP("Bad reg number");
		return -1;
	}

	MKOKRESP("%d %d", v, (int)read_data(HR(v)) ) ;
	return 0 ;
}

CMDAPI_IMPL ( write_out_cell, "wdo" )
{
	char cmd[20] ;
	const char *p = args ;
	int v ;
	long value = 0;

	get_arg(cmd, sizeof(cmd), &p);
	v = atoi(cmd) ;
	
	if (v < 0 )
	{
		MKERRRESP("Bad Cell number");
		return -1;
	}
	
	get_arg(cmd, sizeof(cmd), &p);
	value = (atoi(cmd) > 0 ? 1 : 0);
	
	MKOKRESP("%d %d", v, (int)write_data(CS(v),value));
	return 0 ;
}

CMDAPI_IMPL ( write_in_cell, "wdi" )
{
	char cmd[20] ;
	const char *p = args ;
	int v ;
	long value = 0;

	get_arg(cmd, sizeof(cmd), &p);
	v = atoi(cmd) ;
	
	if (v < 0 )
	{
		MKERRRESP("Bad Cell number");
		return -1;
	}
	
	get_arg(cmd, sizeof(cmd), &p);
	value = (atoi(cmd) > 0 ? 1 : 0);
	
	MKOKRESP("%d %d", v, (int)write_data(IS(v),value));
	return 0 ;
}

CMDAPI_IMPL( write_out_reg, "wao" )
{
	char cmd[20] ;
	const char *p = args ;
	int v, value;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	if (v < 0 )
	{
		MKERRRESP("Bad reg number");
		return -1;
	}
	get_arg(cmd, sizeof(cmd), &p);
	value = atoi(cmd) ;

	if(value < 0)
	{
		MKERRRESP("Bad reg value");
		return -1;
	}
	write_data(HR(v), value );
	
	MKOKRESP("%d %d", v, (int)read_data(HR(v))) ;
	return 0 ;
}

CMDAPI_IMPL( write_in_reg, "wai" )
{
	char cmd[20] ;
	const char *p = args ;
	int v, value;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	if (v < 0 )
	{
		MKERRRESP("Bad reg number");
		return -1;
	}
	get_arg(cmd, sizeof(cmd), &p);
	value = atoi(cmd) ;

	if(value < 0)
	{
		MKERRRESP("Bad reg value");
		return -1;
	}
	write_data(IR(v), value );
	
	MKOKRESP("%d %d", v, (int) read_data(IR(v)) ) ;
	return 0 ;
}


void _debug_level( int lvl );
void _log_mode ( int mode );

CMDAPI_IMPL( set_debug, "debug" )
{
	char cmd[20] ;
	const char *p = args ;
	int v;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	_debug_level(v);
	
	MKOKRESP(" ");
	return 0;
}
CMDAPI_IMPL( set_log, "log" )
{
	char cmd[20] ;
	const char *p = args ;
	int v;

	get_arg(cmd, sizeof(cmd), &p);

	v = atoi(cmd) ;
	_log_mode(v);
	
	MKOKRESP(" ");
	return 0;
}


CMDAPI_IMPL( breakpoint, "brk" )
{
	MKOKRESP(" ");
	return 0;
}






struct S_funct commands[] = {
	#include "commands.gen"
	{NULL, NULL, NULL}
};


