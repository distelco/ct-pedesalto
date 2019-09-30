//
// Copyright (c) Distelxo S.r.l..  All rights reserved.
//
/*
Module Name:
    ax5032adrv.c
    Written By: Franco Novi
    Distelco s.r.l.
    Release $Id: ax5032drv.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#include <stdio.h>
#include <common.h>
#include <sys/io.h>
#include "ax5032.h"
#include "errno.h"
#include "timer.h"


#define	dim(x)	(sizeof(x) / sizeof(x[0]))

static WORD IO_BASE = 0x300 ;		//Indirizzo di base della SCHEDA

static int ioaccess;

//Array per l'antirimbalzo sugli ingressi digitali
static BYTE d_antirimbalzo[16][3];
static int d_i=0;

#define DOUTAddress0 	(IO_BASE)		//indirizzo delle uscite digitali
#define DOUTAddress1	(IO_BASE+2)

#define DINAddress0	(IO_BASE+1)		//indirizzo degli ingressi digitali
#define DINAddress1	(IO_BASE+3)
//scrive le uscite digitali
static inline
void WriteDOut( const BYTE IO_value )
{
		outb(  (IO_value & 0xff), (DOUTAddress0));
//		outb(  ((IO_value>>8)&0xff), (DOUTAddress1));
		return;
}
/*
	legge gli ingressi digitali
*/ 
static inline 
BYTE ReadDIn(void)
{
	return inb((DINAddress0));
}

//
// pack from byte array to bit array
// size is size of the byte array
//
static void IOPack( BYTE* bitArr, BYTE* byteArr, int size )
{
	int i, byte_i, bit_i;
	BYTE c=0 ;

	bit_i=0;
	byte_i=0;
	while( byte_i < size )
	{
		c=0;
		for (i=0; i<8; i++)
		{
			c >>= 1 ;
			if (byteArr[byte_i])
				c |= 0x80 ;

			byte_i++ ;
		}
		bitArr[bit_i++] = c ;
	}
	return;
}


static inline BYTE d_ripple( int id)
{
	return( (d_antirimbalzo[id][0] == d_antirimbalzo[id][1]) ? d_antirimbalzo[id][0] : ((d_antirimbalzo[id][0] == d_antirimbalzo[id][2]) ? d_antirimbalzo[id][0] : d_antirimbalzo[id][2]) );
}


//
// unpack from bit array to byte array
// size is size of byte array
//
static void IOUnpack( BYTE* byteArr, BYTE* bitArr, int size)
{
	int i, byte_i, bit_i;
	BYTE c=0 ;

	bit_i=0;
	byte_i=0;
	while( byte_i < size )
	{
		c = bitArr[bit_i++] ;
		for (i=0; i<8; i++)
		{
			if ( c & 1 )
			{
				d_antirimbalzo[byte_i][d_i] = 1;
			}	
			else {
				d_antirimbalzo[byte_i][d_i] = 0;
			}
			byteArr[byte_i] = d_ripple(byte_i);
			byte_i++ ;
			c >>= 1 ;
		}
	}
	//incremento l'indice dell'array di antirimbalzo
	d_i = (d_i+1) % 3;
	
	return ;
}

#define min(A,B) ((A)<(B) ? (A) : (B))
/*
	Lettura ingressi digitali ax10412
*/
static inline int read_di( char *buf, int buflen )
{
	int num = min(buflen, DIG_INPUT_NUM);
	BYTE	values = 0;

	if(buf == NULL)
	{
		ERR_LOG("Bad buffer pointer");
		return -1;
	}
	if(buflen <= 0)
	{
		ERR_LOG("Bad buflen");
		return -1;
	}
	
	if(ioaccess == 0)
	{
		memset(buf, 0, buflen);
	
		values = ReadDIn();
		IOUnpack( buf, &values, num);
	}
	return 0;
}



/*
	Scrittura uscite digitali ax10412
*/
static inline int write_do( char *buf, int buflen )
{
	int num = min(buflen, DIG_OUTPUT_NUM);
	BYTE	port = 0;
	
	if(buf == NULL)
	{
		ERR_LOG("Bad data pointer");
		return -1;
	}
	if(buflen <= 0)
	{
		ERR_LOG("Bad buflen");
		return -1;
	}
	if( ioaccess == 0)
	{
		IOPack(buf, &port, num);
		WriteDOut(port)	;
	}
	return 0;
}

/*
    funzioni esportate
*/

int ax5032_read_di( char *buf )
{
	return read_di(buf, DIG_INPUT_NUM);
}

int ax5032_write_do( char *buf )
{
	return write_do(buf, DIG_OUTPUT_NUM);
}
/*
	Inizializza il driver, imposta i permessi per l'area di memoria occupata
*/
int ax5032_init_devicestate( struct S_ax5032_data *d )
{
	int i,j;
	IO_BASE = d->address;
	if ( (ioaccess = ioperm((unsigned long)IO_BASE, (unsigned long) 4, 1)) < 0 )
	{
		ERR_LOG("ax5032_init_devicestate:Cannot set ioperm");
		return -1;
	}
	for(i=0;i<8;i++)
	{
		for( j=0; j<3;j++)
		{
			d_antirimbalzo[i][j]=0;
		}
	}
	
	return 0;
}
