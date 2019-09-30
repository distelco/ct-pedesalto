/*
	ax5032.h
	Written By : Franco Novi
	Distelco s.r.l.

	$Id: ax5032.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#if !defined(__AX5032_H__)

#define __AX5032_H__

#include "devices.h"

#define DIG_INPUT_NUM	16	//Numero di input digitali della scheda
#define DIG_OUTPUT_NUM	16	//Numero di output digitali della scheda


#define BYTE	unsigned char
#define WORD	short

// this structure keeps track of each device instance
struct S_ax5032_data {

	long	address;	//Address of board
};

int ax5032_init_devicestate( struct S_ax5032_data *d );
int ax5032_read_di( char *buf );
int ax5032_write_do( char *buf );


#endif
