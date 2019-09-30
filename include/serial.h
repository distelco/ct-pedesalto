/*
	serial.h
	Contiene le funzioni per gestire le seriali
	Written By: Franco Novi
	Distelco s.r.l.
	Release $Id: serial.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#if !defined(__SERIAL_H__)
#define __SERIAL_H__

struct S_serial
{
	int fd ;
	char comport[128] ;
} ;

int  com_open(char *comport) ;
void com_close(int fd) ;
int  com_puts( int fd, unsigned char * s, int len ) ;
int  com_gets( int fd, unsigned char * s, int s_len ) ;


#endif
