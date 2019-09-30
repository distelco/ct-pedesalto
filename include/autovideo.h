/*
    autovideo.h
    file contente le dichiarazioni delle funzioni 
    per interfacciarsi con autovideo
    Written by Franco Novi
    
    $Id: autovideo.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#if !defined(__AUTOVIDEO_H__)

#define __AUTOVIDEO_H__

#include"devices.h"
#include"comunication.h"

struct S_atv_data {
	struct S_tcp * tcp;
	int	channel;
	char	cmd_code[64];
};


int atv_load( struct S_driver_descriptor *, int );
//int atv_load( struct S_driver_descriptor *);


#define drv_atv_did 0x23fe


#endif
