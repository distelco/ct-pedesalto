/*
	serial.c
	Contiene le funzioni per gestire le seriali
	Written By: Franco Novi
	Distelco s.r.l.
	Release $Id: serial.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

//----------------------------------------------------------------

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include "common.h"
#include "serial.h"


/*
	Ritorna il file descriptor della comport aperta
*/
int com_open(char *comport)
{
	int ser_fd = -1 ;
	struct termios term ;
	unsigned long i ;

	DEBUG_LOG("Opening %s", comport) ;	
	if ((ser_fd = open(comport, O_RDWR | O_NONBLOCK)) == -1) {
		ERR_LOG("com_open: cannot open port");
		return -1;
	}

	/* set baud rate */
	tcdrain(ser_fd);
	cfmakeraw(&term);
	cfsetospeed(&term, B19200);
	cfsetispeed(&term, B19200);
	tcflush(ser_fd, TCIOFLUSH);
	term.c_cflag |= CLOCAL;
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 20;
	tcsetattr(ser_fd, TCSANOW, &term);
	tcflush(ser_fd, TCIOFLUSH);
	close(ser_fd);
	/* reopen nonblocking */
	if ((ser_fd = open(comport, O_RDWR)) == -1) {
		ERR_LOG("com_open:cannot open port");
		return -1;
	}
	term.c_cflag |= CLOCAL;
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 20;
	tcsetattr(ser_fd, TCSANOW, &term);
	tcflush(ser_fd, TCIOFLUSH);

	i = TIOCM_DTR ;
	if ( ioctl( ser_fd, TIOCMSET, &i ) < 0 )
	{
		ERR_LOG("com_open:cannot set parameters");
	}

	return ser_fd;
}


//-----------------------------------------------------------

void com_close(int fd)
{
	tcflush(fd, TCIOFLUSH);
	close(fd);
	return;
}

//-----------------------------------------------------------

int com_puts( int fd, unsigned char * s, int len )
{
#ifdef COM_DEBUG
	int i ;
	for ( i=0; i<len;i++)
	    DEBUG_LOG( "com_puts[%d]=%u",i,(unsigned int)(s[i]) ) ;

#endif
	write(fd, s , len);
	return 0 ;
}

static struct pollfd ufds; 

/*
	Legge un carattere dal file descriptor fd
	fd e' il file descriptor della seriale aperto
*/
int com_getc( int fd, unsigned char *s)
{
	int ret;
	ufds.fd = fd;
	ufds.events = POLLIN ;
	
	/* poll the sockets for activity */
	ret = poll( &ufds, 1, 500);
	if (ret > 0)
	{
		if (ufds.revents & POLLIN)
		{
			read(ufds.fd, s, 1);
#ifdef COM_DEBUG
    			DEBUG_LOG( "com_getc=%u",(unsigned int)(*s) ) ;
#endif
		}
	} else {
		//TIMEOUT scaduto
		return -1;
	}
	return 0;
}
/*
	funzione che  legge dal file descriptor fd s_len caratteri e li mette in s
	Ritorna il numero di caratteri letti, -1 errore
*/
int com_gets(int fd, unsigned char *s, int s_len)
{
	int i;
	for(i=0; i<s_len; i++)
	{
		if(com_getc(fd,&s[i])<0)
		{
			return i;
		}
	}
	return s_len;
}
