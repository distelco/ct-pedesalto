/*
	$Id: mymemory.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/
#if !defined(__MYMEMORY_H__)

#define __MYMEMORY_H__

#include <string.h>
#include <pthread.h>
#include <unistd.h>

struct S_digital_memory {
	int number;
	char *cur;
	char *head;
};

struct S_analog_memory {
	int number;
	long *cur;
	long *head;
};

void create_memories();
void destroy_memories();

long write_data( long index, long value );
long read_data( long index );
long change_data( long index );
long set_data ( long index );
long reset_data ( long index );
long trig_data( long index );


long save_digital_mem(char*filename, long start_index);
long load_digital_mem(char*filename, long start_index);
long save_analog_mem(char*filename, long start_index);
long load_analog_mem(char*filename, long start_index);

long save_all_digital_mem(char*filename);
long load_all_digital_mem(char*filename);
long save_all_analog_mem(char*filename);
long load_all_analog_mem(char*filename);

long scale4_20(long an);

pthread_mutex_t mem_data_sem; // questo semaforo blocca la modifica delle aree di memoria durante l'elaborazione

#define DIN_NUM		4096
#define DOUT_NUM	4096
#define AIN_NUM		4096
#define AOUT_NUM	4096
#define MEM_SIZE	4096

#define CS_BASE		10000	// digital output
#define IS_BASE		20000	// digital input
#define IR_BASE		30000	// analog input
#define HR_BASE		40000	// holding registers

#define CS(x)		(x+CS_BASE)
#define IS(x)		(x+IS_BASE)
#define IR(x)		(x+IR_BASE)
#define HR(x)		(x+HR_BASE)
/*

*/

#define LOCK_SEM( void )			\
	pthread_mutex_lock (&mem_data_sem);	


#define UNLOCK_SEM( void )			\
	pthread_mutex_unlock (&mem_data_sem);	\
	sleep(0);


#endif
