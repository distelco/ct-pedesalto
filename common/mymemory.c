/*
	memory.c
	File contenente le funzioni per accedere alla memoria
	Written by Franco Novi
	$Id: mymemory.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "mymemory.h"
#include "string.h"
#include "extern.h"
//struct S_digital_memory memory_area;

struct S_digital_memory digital_input_area;
struct S_digital_memory digital_prev_input_area;
struct S_digital_memory digital_output_area;
struct S_digital_memory digital_changed_output_area;
struct S_analog_memory  analog_input_area;
struct S_analog_memory  analog_prev_input_area;
struct S_analog_memory  analog_output_area;
struct S_analog_memory  analog_changed_output_area;

static void create_dig_mem( struct S_digital_memory *p, const int num_elem )
{
	p->head = (char *) malloc( num_elem*sizeof(char));
	p->number = num_elem;
	p->cur = p->head;
	memset(p->head,0,((p->number)*sizeof(char)));
	return;
}

static void destroy_dig_mem( struct S_digital_memory *p)
{
	free(p->head);
	p->number = 0;
	p->cur = p->head = NULL;
	return;
}



static void create_an_mem( struct S_analog_memory *p, const int num_elem )
{
	p->head = (long *) malloc( num_elem*sizeof(long));
	p->number = num_elem;
	p->cur = p->head;
	memset(p->head,0,((p->number)*sizeof(long)));
	return;
}

static void destroy_an_mem( struct S_analog_memory *p)
{
	free(p->head);
	p->number = 0;
	p->cur = p->head = NULL;
	return;
}

static void create_digital_area()
{
	create_dig_mem( &digital_input_area, DIN_NUM );
	create_dig_mem( &digital_prev_input_area, DIN_NUM );
	create_dig_mem( &digital_output_area, DOUT_NUM );
	create_dig_mem( &digital_changed_output_area, DOUT_NUM );
	return;
}

static void destroy_digital_area()
{
	destroy_dig_mem( &digital_input_area);
	destroy_dig_mem( &digital_prev_input_area);
	destroy_dig_mem( &digital_output_area);
	destroy_dig_mem( &digital_changed_output_area);
	return;
}


static void create_analog_area()
{
	create_an_mem( &analog_input_area, AIN_NUM );
	create_an_mem( &analog_prev_input_area, AIN_NUM );
	create_an_mem( &analog_output_area, AOUT_NUM );
	create_an_mem( &analog_changed_output_area, AOUT_NUM );
	return;
}

static void destroy_analog_area()
{
	destroy_an_mem( &analog_input_area);
	destroy_an_mem( &analog_prev_input_area);
	destroy_an_mem( &analog_output_area);
	destroy_an_mem( &analog_changed_output_area);
	return;
}

/*
static void create_memory_area()
{
	create_dig_mem( &memory_area, MEM_SIZE );
	return;
}

static void destroy_memory_area()
{
	destroy_dig_mem( &memory_area);
	return;
}
*/
void create_memories()
{
	create_digital_area();
	create_analog_area();
//	create_memory_area();
	/* setup mutex*/	
	pthread_mutex_init (&mem_data_sem, NULL);

	return;
}

void destroy_memories()
{
	destroy_digital_area();
	destroy_analog_area();
//	destroy_memory_area();
	/* delete mutex*/	
	pthread_mutex_destroy (&mem_data_sem);

	return;
}

//******************************************************************************************
//******************************************************************************************

long read_dinput( long index )
{
	short	ret;
	
	if( (index < 0) || (index >= digital_input_area.number ))
		return 0;
	LOCK_SEM();
	ret = ((short)(*((digital_input_area.head)+index)));
	UNLOCK_SEM();
	return ret;
}

long write_dinput( long index , long value)
{
	if( (index < 0) || (index >= digital_input_area.number ))
		return -1;
		
	LOCK_SEM();
	*((digital_input_area.head)+index)= (value > 0 ? 1 : 0);
	UNLOCK_SEM();
	
	return 0;
}

long set_dinput(long index)
{	
	return write_dinput(index, 1);
}

long reset_dinput(long index)
{	
	return write_dinput(index, 0);
}

/*
	se l'ingresso e' cambiato ritorna 0,altrimenti 1
*/
long trig_dinput( long index )
{
	int ret;
	
	if( (index < 0) || (index >= digital_input_area.number ))
		return 0;
	LOCK_SEM();	
	ret = (( *((digital_input_area.head)+index)) ^ ( *((digital_prev_input_area.head)+index)));	
	UNLOCK_SEM();
	return ret;
}

long change_dinput( long index )
{
	if( (index < 0) || (index >= digital_input_area.number ))
		return -1;
	LOCK_SEM();
	//segnale digitale 0 o 1	
	*((digital_input_area.head)+index)= !(*((digital_input_area.head)+index));
	
	UNLOCK_SEM();
	return 0;
}



//******************************************************************************************
//******************************************************************************************

long read_doutput( long index )
{
	short ret;
	
	if( (index < 0) || (index >= digital_output_area.number ))
	{
		return 0;
	}
		
	LOCK_SEM();
	ret = 	((short)(*((digital_output_area.head)+index)));
	UNLOCK_SEM();
	
	return ret;
}

long write_doutput( long index, long value )
{
	if( (index < 0) || (index >= digital_output_area.number ))
		return -1;
		
	LOCK_SEM();
	//se l'uscita cambia setto il flag di changed
	if (value != (*((digital_output_area.head)+index)))
		*((digital_changed_output_area.head)+index)= 1;
	//scrivo l'uscita
	*((digital_output_area.head)+index)= (value > 0 ? 1 : 0);
	UNLOCK_SEM();
	return (*((digital_output_area.head)+index));
}

static long set_doutput(long index)
{	
	return write_doutput(index, 1);
}

static long reset_doutput(long index)
{	
	return write_doutput(index, 0);
}

/*
	se l'uscita e' cambiata ritorna 1,altrimenti 0
*/
static long trig_doutput( long index )
{	
	if( (index < 0) || (index >= digital_output_area.number ))
		return -1;
	
	return ( *((digital_changed_output_area.head)+index));
}


static long change_doutput( long index )
{
	if( (index < 0) || (index >= digital_output_area.number ))
		return -1;
	LOCK_SEM();
	//segnale digitale 0 o 1	
	*((digital_output_area.head)+index)= !(*((digital_output_area.head)+index));
	//imposto il flag di changed a 1
	(*( (digital_changed_output_area.head)+index)) = 1;
	
	UNLOCK_SEM();
	return ((*((digital_output_area.head)+index))) ;
}


//******************************************************************************************
//******************************************************************************************
static long read_ainput( long index )
{
	long ret;
	
	if( (index < 0) || (index >= analog_input_area.number ))
		return -1;
	ret = ((long)(*((analog_input_area.head)+index)));
	ret &= 0x0ffff ;
	return ret;
}

static long write_ainput( long index, long value )
{
	if( (index < 0) || (index >= analog_input_area.number ))
		return -1;
	LOCK_SEM();
	//cambio il valore della memoria
	*((analog_input_area.head)+index)=(value & 0x0ffff);
	UNLOCK_SEM();
	return 0;
}

//******************************************************************************************
//******************************************************************************************
static long read_aoutput( long index )
{
	long ret;
	
	if( (index < 0) || (index >= analog_output_area.number ))
		return -1;
	ret = ((long)(*((analog_output_area.head)+index)));
	ret &= 0x0ffff ;
	return ret;
}

static long write_aoutput( long index, long value )
{
	if( (index < 0) || (index >= analog_output_area.number ))
		return -1;
	LOCK_SEM();
	//setto il falg di changed se l'uscita cambia
	if( (*((analog_output_area.head)+index))!= value )
		(*((analog_changed_output_area.head)+index)) = 1;
	//cambio il valore della memoria
	*((analog_output_area.head)+index)=(value & 0x0ffff);
	UNLOCK_SEM();
	return (*((analog_output_area.head)+index));
}

//******************************************************************************************
// Routine di accesso unificate 
//******************************************************************************************
long write_data( long index, long value )
{
	if ( (index >= IS_BASE) && (index < (IS_BASE+DIN_NUM) ) )
	{
		return write_dinput( index-IS_BASE, value) ;
	}
	if ( (index >= CS_BASE) && (index < (CS_BASE+DOUT_NUM) ) )
	{
		return write_doutput( index-CS_BASE, value) ;
	}
	if ( (index >= IR_BASE) && (index < (IR_BASE+AIN_NUM) ) )
	{
		return write_ainput( index-IR_BASE, value) ;
	}
	if ( (index >= HR_BASE) && (index < (HR_BASE+AOUT_NUM) ) )
	{
		return write_aoutput( index-HR_BASE, value) ;
	}
	ERR_LOG("Bad index=%d call in write_data", index ) ;
	exit(-1) ;
}

long read_data( long index )
{
	if ( (index >= IS_BASE) && (index < (IS_BASE+DIN_NUM) ) )
	{
		return read_dinput( index-IS_BASE) ;
	}
	if ( (index >= CS_BASE) && (index < (CS_BASE+DOUT_NUM) ) )
	{
		return read_doutput( index-CS_BASE) ;
	}
	if ( (index >= IR_BASE) && (index < (IR_BASE+AIN_NUM) ) )
	{
		return read_ainput( index-IR_BASE) ;
	}
	if ( (index >= HR_BASE) && (index < (HR_BASE+AOUT_NUM) ) )
	{
		return read_aoutput( index-HR_BASE) ;
	}
	ERR_LOG("Bad index=%d call in read_data", index ) ;
	exit(-1) ;
}

long change_data( long index )
{
	if ( (index >= IS_BASE) && (index < (IS_BASE+DIN_NUM) ) )
	{
		return change_dinput( index-IS_BASE) ;
	}
	if ( (index >= CS_BASE) && (index < (CS_BASE+DOUT_NUM) ) )
	{
		return change_doutput( index-CS_BASE) ;
	}
	ERR_LOG("Bad index=%d call in read_data", index ) ;
	exit(-1) ;
}

long set_data ( long index )
{
	if ( (index >= IS_BASE) && (index < (IS_BASE+DIN_NUM) ) )
	{
		return set_dinput( index-IS_BASE) ;
	}
	if ( (index >= CS_BASE) && (index < (CS_BASE+DOUT_NUM) ) )
	{
		return set_doutput( index-CS_BASE) ;
	}
	ERR_LOG("Bad index=%d call in read_data", index ) ;
	exit(-1) ;
}
long reset_data ( long index )
{
	if ( (index >= IS_BASE) && (index < (IS_BASE+DIN_NUM) ) )
	{
		return reset_dinput( index-IS_BASE) ;
	}
	if ( (index >= CS_BASE) && (index < (CS_BASE+DOUT_NUM) ) )
	{
		return reset_doutput( index-CS_BASE) ;
	}
	ERR_LOG("Bad index=%d call in read_data", index ) ;
	exit(-1) ;

}
long trig_data( long index )
{
	if ( (index >= IS_BASE) && (index < (IS_BASE+DIN_NUM) ) )
	{
		return trig_dinput( index-IS_BASE) ;
	}
	if ( (index >= CS_BASE) && (index < (CS_BASE+DOUT_NUM) ) )
	{
		return trig_doutput( index-CS_BASE) ;
	}
	ERR_LOG("Bad index=%d call in read_data", index ) ;
	exit(-1) ;

}

//******************************************************************************************
// Routine di salvataggio e caricamento aree di memoria
//******************************************************************************************
long save_digital_mem(char*filename, long start_index)
{
	int fd;
	
	fd= creat( filename, O_RDWR | 0660 );
	if( fd < 0)
	{
		ERR_LOG("File open error: %s", filename);
		return -1;
	}
	
	write( fd, ((digital_output_area.head)+start_index), (digital_output_area.number-start_index) );
	
	close(fd);
	return 0;
}

long load_digital_mem(char*filename, long start_index)
{
	int fd;
	
	fd= open(filename, O_RDWR);
	if( fd < 0)
	{
		ERR_LOG("File open error: %s", filename);
		return -1;
	}
	
	read( fd, ((digital_output_area.head)+start_index), (digital_output_area.number-start_index) );
	
	close(fd);	
	return 0;
}



long save_analog_mem(char*filename, long start_index)
{
	int fd;
	
	fd= creat(filename, O_RDWR | 0660 );
	if( fd < 0)
	{
		ERR_LOG("File open error: %s", filename);
		return -1;
	}
	
	write( fd, ((analog_output_area.head)+start_index), (analog_output_area.number-start_index)*sizeof(long) );
	close(fd);	
	return 0;
}

long load_analog_mem(char*filename, long start_index)
{
	int fd;
	
	fd= open(filename, O_RDWR);
	if( fd < 0)
	{
		ERR_LOG("File open error: %s", filename);
		return -1;
	}
	
	read( fd, ((analog_output_area.head)+start_index), (analog_output_area.number-start_index)*sizeof(long) );
	close(fd);	
	return 0;
}

// Funzioni per salvataggio aree di memoria complete
long save_all_digital_mem(char*filename)
{
	int fd;
	
	fd= creat( filename, O_RDWR | 0660 );
	if( fd < 0)
	{
		ERR_LOG("File open error: %s", filename);
		return -1;
	}
	
	write( fd, digital_output_area.head, digital_output_area.number );
	
	close(fd);
	return 0;
}

long save_all_analog_mem(char*filename)
{
	int fd;
	
	fd= creat(filename, O_RDWR | 0660 );
	if( fd < 0)
	{
		ERR_LOG("File open error: %s", filename);
		return -1;
	}
	
	write( fd, analog_output_area.head, analog_output_area.number*sizeof(long) );
	close(fd);	
	return 0;
}


//*****************************************************************************
// Conversione e scaling misure
//
long scale4_20( long an )
{
	if ( an < 3000 )
		return 0 ;
	if ( an < 6550 )
		return 6550 ;
		
	return an ;
}


