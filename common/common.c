/*
	Common function of all PLC programs
	Written By: Franco Novi
	Distelco s.r.l.
	Revision : $Id: common.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $	

*/


#include <errno.h>
#include <ctype.h>
#include <stdarg.h>

#include "common.h"


void save_runfile(char *fn)
{
	int fd ;
	char s[10];
	char buf[256] ;
	
	fd =  creat( fn, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) ;
	if ( fd<0 ) {
		strerror_r( errno, buf, 256 ) ;
		ERR_LOG("Cannot create pid file: %s", buf ) ;
		return ;
	}
	sprintf( s, "%d\n", getpid() ) ;
	write( fd, s, strlen(s) ) ;
	close(fd) ;
	return ;
}

void save_threadnum( char *fn, int num )
{
	int fd ;
	char s[10];
	char buf[256] ;
	
	fd =  creat( fn, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) ;
	if ( fd<0 ) {
		strerror_r( errno, buf, 256 ) ;
		ERR_LOG("Cannot create thread num file: %s", buf ) ;
		return ;
	}
	sprintf( s, "%d\n", num ) ;
	write( fd, s, strlen(s) ) ;
	close(fd) ;
	return ;
}

/* A function to make the server a daemon. */
/* Makes the server run as a daemon */
int make_daemon (void)
{
	pid_t p;

	p = fork();
	if (p < 0) {
		printf(": Fork returned %d\n", (int)p) ;
		return -1;
	} else if (p > 0) {
		sleep (1);
		exit (0);
	}

	if (setpgrp() < 0) {
		perror(":setpgrp") ;
		return -1;
	}
	setsid() ;

	close(0) ;
	close(1) ;
	close(2) ;

	chdir ("/");
	umask (022);
	return 0;
}


/* resolve host with also IP address parsing */
int resolve_host(struct in_addr *sin_addr, const char *hostname)
{
    struct hostent *hp;

    if ((inet_aton(hostname, sin_addr)) == 0) {
        hp = gethostbyname(hostname);
        if (!hp)
            return -1;
        memcpy (sin_addr, hp->h_addr, sizeof(struct in_addr));
    }
    return 0;
}

void strcat2( char * dest, char* str1, char *str2)
{
	strcat(dest, str1);
	strcat(dest, str2);
	return;
}


void remove_pidfilename( char*fpath, char *fname )
{
	char f[256];
	
	strcat2( f, fpath, fname ) ;
	unlink( f);
	return;
}

void write_pidfilename( char *fpath, char *fname, int pid )
{
	int fd;
	char s[10];
	char f[256];
	
	strcat2( f, fpath, fname ) ;
	
	if ( ( fd = open( f , O_RDWR | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH )) < 0 )
	{
		ERR_LOG("Impossibile aprire o creare il file %s", f );
	}
	else
	{
		sprintf( s, "%d\n", pid ) ;
		write( fd, s, strlen(s) ) ;		
		
		if ( close( fd ) < 0 )
		{
			ERR_LOG("Impossibile chiudere il file %s", f );
			return ;
		}
	}
	return;
}

void get_arg(char *buf, int buf_size, const char **pp)
{
	const char *p;
	char *q;
	int quote;

	p = *pp;
	while (isspace(*p))
		p++;
	q = buf;
	quote = 0;
	if (*p == '\"' || *p == '\'')
		quote = *p++;
	for(;;) {
		if (quote) {
			if (*p == quote)
				break;
		} else {
			if (isspace(*p))
				break;
		}
		if (*p == '\0')
			break;
		if ((q - buf) < buf_size - 1)
			*q++ = *p;
		p++;
	}
	*q = '\0';
	if (quote && *p == quote)
		p++;
	*pp = p;
}

//**************************************************************************************
// Debug and logging function
//**************************************************************************************
//#define FMT_TIME	    time_t t; char bb[30] ; time( &t ) ; ctime_r(&t, bb); *(strchr(bb,'\n'))=0
#ifdef DEBUG
static int log_mode = USE_PRINTF | USE_SYSLOG ;
static int debug_level = 3 ;
#else
static int log_mode = USE_SYSLOG;
static int debug_level = -1 ;
#endif

void _debug_level( int lvl )
{
	debug_level = lvl ;
	return ;
}

void _log_mode ( int mode )
{
	log_mode = mode ;
}

void output_command_message( char *fmt, va_list ap );


void _info_log( char *fmt, ...)
{
	va_list ap ;
	
	if ( log_mode & USE_PRINTF )
	{
		va_start( ap, fmt) ;
		vprintf( fmt, ap ) ;
		va_end(ap) ;
	}
	if ( log_mode & USE_SYSLOG )
	{
		va_start( ap, fmt) ;
		vsyslog(LOG_USER | LOG_INFO, fmt, ap) ;
		va_end(ap) ;
	}
	if ( log_mode & USE_SOCKET )
	{
		va_start( ap, fmt) ;
		output_command_message( fmt, ap );
		va_end(ap) ;
	}
	return ;
}

void _warn_log( char *fmt, ...)
{
	va_list ap ;
	
	if ( log_mode & USE_PRINTF )
	{
		va_start( ap, fmt) ;
		vprintf( fmt, ap ) ;
		va_end(ap) ;
	}
	if ( log_mode & USE_SYSLOG )
	{
		va_start( ap, fmt) ;
		vsyslog(LOG_USER | LOG_WARNING, fmt, ap) ;
		va_end(ap) ;
	}
	if ( log_mode & USE_SOCKET )
	{
		va_start( ap, fmt) ;
		output_command_message( fmt, ap );
		va_end(ap) ;
	}
	return ;
}

void _err_log( char *fmt, ...)
{
	va_list ap ;
	
	if ( log_mode & USE_PRINTF )
	{
		va_start( ap, fmt) ;
		vprintf( fmt, ap ) ;
		va_end(ap) ;
	}	
	if ( log_mode & USE_SYSLOG )
	{
		va_start( ap, fmt) ;
		vsyslog(LOG_USER | LOG_ERR, fmt, ap) ;
		va_end(ap) ;
	}
	if ( log_mode & USE_SOCKET )
	{
		va_start( ap, fmt) ;
		output_command_message( fmt, ap );
		va_end(ap) ;
	}
	return ;
}

void _debug_log(int lvl, char *fmt, ...)
{
	va_list ap ;
	if ( lvl <= debug_level )
	{
		if ( log_mode & USE_PRINTF )
		{
			va_start( ap, fmt) ;
			vprintf( fmt, ap ) ;
			va_end(ap) ;
		}	
		if ( log_mode & USE_SYSLOG )
		{
			va_start( ap, fmt) ;
			vsyslog(LOG_USER | LOG_INFO, fmt, ap) ;
			va_end(ap) ;
		}
		if ( log_mode & USE_SOCKET )
		{
			va_start( ap, fmt) ;
			output_command_message( fmt, ap );
			va_end(ap) ;
		}
	}
	return ;
}

