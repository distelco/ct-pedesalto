/*
	ax10412.h
	Written By : Franco Novi
	Distelco s.r.l.

	$Id: ax10412.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#if !defined(__AX10412_H__)

#define __AX10412_H__

#include "devices.h"

#define DIG_INPUT_NUM	8	//Numero di input digitali della scheda
#define DIG_OUTPUT_NUM	8	//Numero di output digitali della scheda
#define	A_INPUT	8		//Numero di input analogici della scheda


#define BYTE	unsigned char
#define WORD	short

// this structure keeps track of each device instance
struct S_ax_data {

	long	address;	//Address of board
	BYTE	mode;		// define conversion mode:								
					// 0 -> 12 bit resolution
					// 1 ->	16 bit resolution

	BYTE	gain;		//define Gain value:
					// 0x00 -> 0 - 10 V , +-5V
					// 0x01 -> 0 - 1 V
					// 0x10 -> 0 - 100 mV															// 0x11 -> 0 - 20 mV
	BYTE	cjc;		//define CJC en/dis:	
					// 0x00 -> disable
};

int ax10412_init_devicestate( struct S_ax_data *d );
int ax10412_read_ai( long *a_buf, char *d_buf );
int ax10412_read_di( char *buf );
int ax10412_write_do( char *buf );


#endif
