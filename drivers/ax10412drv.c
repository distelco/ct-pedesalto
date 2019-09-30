//
// Copyright (c) Distelxo S.r.l..  All rights reserved.
//
/*
Module Name:
    ax10412drv.c
    Written By: Franco Novi
    Distelco s.r.l.
    Release $Id: ax10412drv.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#include <stdio.h>
#include <common.h>
#include <sys/io.h>
#include "ax10412.h"
#include "errno.h"
#include "timer.h"


#define	dim(x)	(sizeof(x) / sizeof(x[0]))

static	WORD AIO_BASE = 0x300 ;		//Indirizzo di base della SCHEDA

static int ioaccess;

//Array per l'antirimbalzo sugli ingressi digitali
static BYTE d_antirimbalzo[8][3];
//Array per l'antirimbalzo sugli ingressi analogiche
static WORD a_antirimbalzo[16][5];

static int d_i=0;
static int a_i=0;

#define DOUTAddress 	(AIO_BASE+3)		//indirizzo delle uscite digitali
#define DINAddress	DOUTAddress		//indirizzo degli ingressi digitali

#define DAC_RESOLUTION	12	//Utilizziamo DAC a 12 bit di risoluzione

static struct S_timer timeout;


/*
	
*/
static inline BYTE SetSignal( const BYTE data, const BYTE signal )
{
	return  (data | signal);
}
/*

*/
static inline BYTE ResetSignal( const BYTE data, const BYTE signal )
{
	return  (data & (~signal));
}

//scrive le uscite digitali
static inline
void WriteDOut( const BYTE IO_value )
{
		outb(  IO_value, (DOUTAddress));
		return;
}
/*
	legge gli ingressi digitali
*/ 
static inline	
BYTE ReadDIn(void)
{
	return inb((DINAddress));
}
/*
	restituisce lo stato di un segnale digitale
*/
static inline
int GetSignal(BYTE signal)
{
	return ( (( ReadDIn() & signal) ? 1 : 0 ));
}

/*
	Funzioni per la lettura e configurazione della parte analogica nella scheda ax10412
*/
#define EOC	0x01
#define CJC	0x10

static inline BYTE SetCJC( BYTE b)
{
	return (b | CJC );
}

static inline BYTE ResetCJC( BYTE b)
{
	return ( b & (~CJC) );
}

//ch deve essere compreso tra 0 e 15
// SetChannel setta il canale desiderato e cjc 
static inline void SetChannelCjc( const int ch, const int cjc )
{
	BYTE c = (BYTE) ch;
	
	c = (cjc > 0 ? SetCJC(c) : ResetCJC(c));

	outb( c,(AIO_BASE+1));
	return;
}
/*
	Imposta il canale analogico ch
*/
static inline void SetChannel(const int ch )
{
	SetChannelCjc( ch, 0 );
}
/*
	Lancia la conversione A/D
*/
static inline void StartConversion(void)
{
	outb(0xff,AIO_BASE );
	return;
}
/*
	Segnala la fine della conversione A/D
*/
static inline int WaitEndOfConversion(void)
{
	return ( inb((AIO_BASE+2)) & EOC );
}
/*
	Legge L'ingresso analogico del canale attuale a 12 bit
*/
static inline WORD ReadAIn(void)
{
	WORD	Dvalue = 0x0000;
	BYTE	Status ;

	Status = ((BYTE)(inb((AIO_BASE+2)) ));

	Dvalue = ((WORD)(inb((AIO_BASE+1)) ));
	Dvalue <<= 8;
	Dvalue |= (WORD) inb(AIO_BASE);
	if (Dvalue & 0x1000)
	{
		//Overflow!
		Dvalue = 0x07ff;
	} else {
		Dvalue -= 0x0800;
	}

	return	(Dvalue & 0x0fff);
}

/*
 	questa funzione restituisce una word contenente il valore digitale della porta analogica id
 	id compreso da 0 a A_INPUT - 1
*/

#define READAIO_TIMEOUT	0x10000000L

static inline long ReadAIO( const int id )
{	
	SetChannel(id);
	usleep(10);
	
	StartConversion();
	usleep(150);
	
	timer_init(&timeout,NULL,300);
	
	while( WaitEndOfConversion() )
	{
		usleep(100);
		if( timer_state(&timeout) > 0 )
		{
			return READAIO_TIMEOUT;
		}
	}
	return (long)ReadAIn();
}
/*
	Setta Mode e Gain del canale corrente
*/
static inline void SetModeAndGain(const BYTE mode, const BYTE gain)
{
	BYTE out =	0x00 | (gain << 1) | mode ;
	outb( out, (AIO_BASE + 2));
	return;
}
/*
	setta mode e gain di tutti gli ingressi analogici
*/
static inline 
void WriteConfig(const BYTE mode, const BYTE gain)
{
	int i;
	for(i = 0; i < A_INPUT ; i++)
	{
		SetChannel(i);
		SetModeAndGain( mode, gain);
	}
	return;
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
				//byteArr[byte_i] = 1;
				d_antirimbalzo[byte_i][d_i] = 1;
			}	
			else {
				//byteArr[byte_i] = 0;
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

static inline long a_ripple(int id)
{
	int i;
	long min = 65536, max = -65536;
	long sum = 0;
	for(i=0;i<5;i++)
	{
		min = (min > a_antirimbalzo[id][i] ? a_antirimbalzo[id][i] : min );
		max = (max < a_antirimbalzo[id][i] ? a_antirimbalzo[id][i] : max );
		sum += a_antirimbalzo[id][i];
	}
	return( ((sum - min - max)/3) );
}


/*
	Lettura ingressi analogici ax10412
*/
static inline int read_ai( long *a_buf, char *d_buf, int buflen )
{
	int ioport;
	int num = min(buflen, A_INPUT);

	if(d_buf == NULL)
	{
		ERR_LOG("Bad data pointer");
		return -1;
	}
	if(a_buf == NULL)
	{
		ERR_LOG("Bad data pointer");
		return -1;
	}
	if(buflen <= 0)
	{
		ERR_LOG("Bad buflen");
		return -1;
	}
	if(ioaccess == 0)
	{	
		// memset( buf, 0, buflen );
		// read all inputs from devices
		for( ioport = 0 ; ioport < (int) num; ioport++)
		{
			long tmp;
			long data ;
			if( ( data = ReadAIO( ioport )) == READAIO_TIMEOUT )
			{
				d_buf[ioport] = 0;
			} else {
				tmp = (data << 4) ; // from 12 to 16 bit
				d_buf[ioport] = 1;
				a_antirimbalzo[ioport][a_i] = tmp;
			}		
			a_buf[ioport] = a_ripple(ioport);
		}
	}
	a_i = (a_i+1)%5;
	return 0;
}

int ax10412_read_ai( long *a_buf, char *d_buf )
{
	return 	read_ai(a_buf, d_buf, A_INPUT);
}

int ax10412_read_di( char*buf )
{
	return read_di(buf, DIG_INPUT_NUM);
}

int ax10412_write_do( char *buf )
{
	return write_do(buf, DIG_OUTPUT_NUM);
}
/*
	Inizializza il driver, imposta i permessi per l'area di memoria occupata
*/
int ax10412_init_devicestate( struct S_ax_data *d )
{
	int i,j;
	AIO_BASE = d->address;
	if ( (ioaccess = ioperm((unsigned long)AIO_BASE, (unsigned long) 4, 1)) < 0 )
	{
		ERR_LOG("ax10412_init_devicestate:Cannot set ioperm");
		return -1;
	}
	//Setta CJC, Gain e Mode per tutti gli ingressi analogici
	WriteConfig(d->mode, d->gain);
	
	for(i=0;i<A_INPUT;i++)
	{
		for( j=0; j<5;j++)
		{
			a_antirimbalzo[i][j]=0;
		}
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

