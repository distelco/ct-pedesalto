/**
	\file common.h
	Written by Franco Novi
	email:matteoc@devel

	Revision : $Id: common.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $	

*/

#if !defined( __COMMON_H__)
#define __COMMON_H__

#include "../config.h"
#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>



#define USE_SYSLOG 1
#define USE_PRINTF 2
#define USE_SOCKET 4

/* define per fare i delay in ms */
#define delay(x)	usleep((x)*1000)


void _info_log( char *fmt, ...);
void _warn_log( char *fmt, ...);
void _err_log( char *fmt, ...);
void _debug_log( int lvl, char *fmt, ...);

#define OPENLOG()			openlog(argv[0], 0, LOG_USER)
#define CLOSELOG()			closelog()
#define INFO_LOG(fmt,arg...)		_info_log(fmt "\n", ##arg)
#define WARN_LOG(fmt,arg...)		_warn_log(fmt "\n", ##arg)
#define ERR_LOG(fmt,arg...)		_err_log(fmt "\n", ##arg)
#define DEBUG_LOG(fmt,arg...)		_debug_log(0, fmt "\n", ##arg)
#define DEBUG0_LOG(fmt,arg...)		_debug_log(0, fmt "\n", ##arg)
#if defined(DEBUG)
#define DEBUG1_LOG(fmt,arg...)		_debug_log(1, fmt "\n", ##arg)
#define DEBUG2_LOG(fmt,arg...)		_debug_log(2, fmt "\n", ##arg)
#else
#define DEBUG1_LOG(fmt,arg...)
#define DEBUG2_LOG(fmt,arg...)
#endif

						
void save_runfile(char *) ;
void save_threadnum(char * , int ) ;
void strcat2( char * dest, char* str1, char *str2);

void write_pidfilename( char *fpath, char *fname, int pid );
void remove_pidfilename( char*fpath, char *fname );

void get_arg(char *buf, int buf_size, const char **pp);

#define SET_STRING( A ){get_arg((A), sizeof((A)), &p);}
#define SET_NUM( A ){get_arg(cmd, sizeof(cmd), &p);(A) = atoi(cmd);}

#endif
