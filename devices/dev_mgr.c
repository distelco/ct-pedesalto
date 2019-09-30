/*
	dev_mgr.c
	gestisce la lettura e la scrittura su tutti i device e la lettura e la scrittura della memoria del plc
	Written by Franco Novi
	$Id: dev_mgr.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#undef DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "devices.h"
#include "mymemory.h"
#include "cfg.h"
#include "dev_mgr.h"
#include "extern.h"

#define MAX_TYPES	10
#define MAX_IO_BLOCK	100

extern struct S_driver_descriptor	*drv_types[];

extern struct S_driver_io_block		di_table[];
extern struct S_driver_io_block		do_table[];
extern struct S_driver_io_block		ai_table[];
extern struct S_driver_io_block		ao_table[];
extern struct S_driver_io_block		mem_table[];

extern struct S_transform_io_block	di_transform[];
extern struct S_transform_io_block	ai_transform[];

//extern struct S_digital_memory memory_area;
extern struct S_digital_memory digital_input_area;
extern struct S_digital_memory digital_output_area;
extern struct S_digital_memory digital_prev_input_area;
extern struct S_analog_memory  analog_input_area;
extern struct S_analog_memory  analog_prev_input_area;
extern struct S_analog_memory  analog_output_area;

#if defined( DYNAMIC_LOAD_DRIVER )
static inline int seek_free_io_block( struct S_driver_io_block *block )
{
	int i = 0;
	
	if(block == NULL)
		return -1;
		
	while( block[i] && (i < MAX_IO_BLOCK) )
		i++;
	if (i < MAX_IO_BLOCK)
		return i;
		
	return -1;
}

static inline struct S_driver_io_block *create_block()
{
	struct S_driver_io_block *p;
	p =(struct S_driver_io_block *) malloc(sizeof(struct S_driver_io_block));
	memset(p,0,sizeof(struct S_driver_io_block));
	return p;
}

static inline void destroy_block( struct S_driver_io_block * pblock)
{
	free(pblock);
	pblock = NULL;
	return;
}

static inline void set_base_len( struct S_driver_io_block * p, int base, int lenght )
{
	if (p == NULL)
	{
		ERR_LOG("Driver IO block NULL pointer");
		return ;
	}
	p->base = base;
	p->lenght = lenght;
	
	return;
}

inline struct S_driver_descriptor *create_drv_type()
{
	struct S_driver_descriptor * p;
	
	return 0;
	
	
	p = (struct S_driver_descriptor *)malloc(sizeof(struct S_driver_descriptor));
	memset(p,0,sizeof(struct S_driver_descriptor));
	return p;
}

inline void destroy_drv_type( struct S_driver_descriptor * pd )
{
	
	if( pd != NULL )
	{
		free(pd);
		pd = NULL;
	}
	return;
}

//dato un descrittore di driver ricerca il corrispettivo nella lista dei driver
//ritorna l'indice della lista, -1 in caso non trovi nessun indice corrispondente
inline int find_driver_name(struct S_driver_io_block *io_block, struct S_driver_descriptor *drv)
{
	int i = 0;
	
	//per ogni driver scorro nella tabelle finche' non trovo il corrispondente nome
	while( strcmp((io_block+i)->name,drv->name) != 0)
	{
		if( (++i) >= MAX_TYPES)
			return -1;
	}
	return i;	
}

static inline int find_drv_type(char *name, int *pdid )
{
	int i=0;
	
	if(pdid!= NULL)
	{
		while( drv_types[i] && (drv_types[i]->did != (*pdid) ) && (i < MAX_TYPES) )
			i++;
	} 
	else if(name != NULL)
	{
		while( drv_types[i] && strcmp(name,drv_types[i]->name) && (i < MAX_TYPES))
			i++;
	} else {
		return -1;
	}
	
	if( i < MAX_TYPES )
	{
		return i;
	}
	
	return -1;
}

int load_driver_did( char *name, int did )
{
	int i;
	
	if (name == NULL)
	{
		ERR_LOG("Bad driver name");
		return -1;
	}
	if( (i = find_drv_type(name, &did )) >=0 )
	{
		drv_types[i]->did = did;
	} else {
		return -1;
	}
		
	return 0;
}

int load_driver( char*name, int did, char*fname)
{
	int i=0;
	FILE * fd;
	
	if (name == NULL)
	{
		ERR_LOG("Bad driver name");
		return -1;
	}
	if (fname == NULL)
	{
		ERR_LOG("Bad driver configuration file");
		return -1;
	}	
	
	//apro il file di configurazione specifico per quel driver
	fd = fopen( fname, "r" );
	if( fd == NULL)
	{
		ERR_LOG("Impossible aprire il file di configurazione %s", fname);
		return -1;
	}	
	if( (i = find_drv_type(name, &did )) >=0 )
	{
		int j;
			
		if(drv_types[i]->read_digital)
		{
			j=0;
			//cerco il primo puntatore della tabella degli digital input io_block libero
			if( (j = seek_free_io_block(di_table)) >= 0 )
			{
				//alloco un nuovo io_block;
				di_table[j] = create_block();
				//scrivo il nome del driver
				strcpy( di_table[j]->name, name);
				//scrivo il driver_id
				di_table[j]->did = did;
				//copio i dati nell'io_block				
				drv_types[i]->init_block(di_table[j],fd);
			}
		}
		if(drv_types[i]->write_digital)
		{
			j=0;
			//cerco il primo puntatore della tabella degli digital input io_block libero
			if( (j = seek_free_io_block(do_table)) >= 0 )
			{
				//alloco un nuovo io_block;
				do_table[j] = create_block();
				//scrivo il nome del driver
				strcpy( do_table[j]->name, name);
				//scrivo il driver_id
				do_table[j]->did = did;
				//copio i dati nell'io_block				
				drv_types[i]->init_block(do_table[j],fd);
			}
		}
		
		if(drv_types[i]->read_analog)
		{
			j=0;
			//cerco il primo puntatore della tabella degli digital input io_block libero
			if( (j = seek_free_io_block(ai_table)) >= 0 )
			{
				//alloco un nuovo io_block;
				ai_table[j] = create_block();
				//scrivo il nome del driver
				strcpy( ai_table[j]->name, name);
				//scrivo il driver_id
				ai_table[j]->did = did;
				//copio i dati nell'io_block				
				drv_types[i]->init_block(ai_table[j],fd);
			}
		}
		if(drv_types[i]->write_analog)
		{
			j=0;
			//cerco il primo puntatore della tabella degli digital input io_block libero
			if( (j = seek_free_io_block(ao_table)) >= 0 )
			{
				//alloco un nuovo io_block;
				ao_table[j] = create_block();
				//scrivo il nome del driver
				strcpy( ao_table[j]->name, name);
				//scrivo il driver_id
				ao_table[j]->did = did;
				//copio i dati nell'io_block				
				drv_types[i]->init_block(ao_table[j],fd);
			}
		}		
	}
	
	//chiudo il file	
	i = fclose(fd);
	if( i < 0 )
	{
		ERR_LOG("Impossible chiudere il file di configurazione %s", fname);
		return -1;
	}	
	
	return 0;
}

void destroy_tables()
{
	int i=0;
	
	while(di_table[i])
		destroy_block(di_table[i++]);
	i=0;		
	while(do_table[i])
		destroy_block(do_table[i++]);
	i=0;		
	while(ai_table[i])
		destroy_block(ai_table[i++]);
	i=0;		
	while(ao_table[i])
		destroy_block(ao_table[i++]);
	i=0;		
	while(mem_table[i])
		destroy_block(mem_table[i++]);
		
	//dealloco l'array di descrittori dei driver
	i=0;		
	while(do_table[i])
		destroy_drv_type(drv_types[i++]);
}
#endif	//#if defined( DYNAMIC_LOAD_DRIVER )

#define BUF_SIZE	1024

int io_manager(int ro)
{
	int i, j=0;
	char buf[BUF_SIZE];
	
	//azzero buf
	memset(buf,0,BUF_SIZE);	

	//salvo i dati precedenti
	memcpy( digital_prev_input_area.head ,digital_input_area.head, DIN_NUM*sizeof(char));
	memcpy( analog_prev_input_area.head ,analog_input_area.head, AIN_NUM*sizeof(long));
	
	for ( i=0; drv_types[i] != NULL; i++ )
	{
		//per ogni driver trovato scorro nella tabelle finche' non trovo tutte le sue occorrenze
		if ( drv_types[i]->read_digital != NULL )
		{
			for(j=0; di_table[j].did != 0 ; j++ )
			{
				if ( di_table[j].did == drv_types[i]->did )
				{
					//leggo ingressi digitali
						
					di_table[j].state = drv_types[i]->read_digital((di_table[j]).drv_info, buf, di_table[j].lenght);
					if ( di_table[j].state == 0)
					{
						int v;
						LOCK_SEM();
						//controlla conversioni
						for ( v=0; di_transform[v].type != NULL; v++ )
						{
							if ( (di_transform[v].addr >= di_table[j].base) && (di_transform[v].addr < (di_table[j].base+di_table[j].lenght)) )
							{
								buf[(di_transform[v].addr-di_table[j].base)] = !buf[(di_transform[v].addr-di_table[j].base)];
							}
						}
						//trasferisco i dati in memoria
						memcpy( ((digital_input_area.head) + (di_table[j].base)) ,buf,di_table[j].lenght);
						UNLOCK_SEM();
					}
				}
			}
			if ( drv_types[i]->flush_io ) 
				drv_types[i]->flush_io();
		}
		
		if ( (ro == 0) && (drv_types[i]->write_digital != NULL) )
		{
			for(j=0; do_table[j].did != 0L ; j++ )
			{
				if ( do_table[j].did == drv_types[i]->did)
				{
					//scrivo le uscite digitali
					drv_types[i]->write_digital(do_table[j].drv_info, ((digital_output_area.head) +(do_table[j].base)), do_table[j].lenght);
				}
			}
			if ( drv_types[i]->flush_io ) 
				drv_types[i]->flush_io();
		}
		
		if ( drv_types[i]->read_analog != NULL )
		{
			for(j=0; ai_table[j].did != 0 ; j++ )
			{
				if ( ai_table[j].did == drv_types[i]->did)
				{
					//leggo ingressi analogici
					if (drv_types[i]->read_analog(ai_table[j].drv_info,(long *) buf, ai_table[j].lenght) == 0 )
					{
						int v;
						long *dt;
						dt = (long*)buf ;
						LOCK_SEM();
						//controlla conversioni
						for ( v=0; ai_transform[v].type != NULL; v++ )
						{
							if ( (ai_transform[v].addr >= ai_table[j].base) && 
							    (ai_transform[v].addr < (ai_table[j].base+ai_table[j].lenght)) )
							{
								dt[(ai_transform[v].addr-ai_table[j].base)] = 
								    scale4_20(dt[(ai_transform[v].addr-ai_table[j].base)]);
							}
						}
						//trasferisco i dati in memoria
						memcpy( ((analog_input_area.head) + (ai_table[j].base)) ,buf,ai_table[j].lenght*sizeof(long));
						UNLOCK_SEM();
					}
				}
			}
			if ( drv_types[i]->flush_io ) 
				drv_types[i]->flush_io();
		}
		
		if ( (ro == 0) && (drv_types[i]->write_analog != NULL) )
		{
			for(j=0; ao_table[j].did != 0 ; j++ )
			{
				if ( ao_table[j].did == drv_types[i]->did)
				{
					//scrivo le uscite analogiche
					drv_types[i]->write_analog(ao_table[j].drv_info, ((analog_output_area.head) +(ao_table[j].base)), ao_table[j].lenght);
				}
			}
			if ( drv_types[i]->flush_io ) 
				drv_types[i]->flush_io();
		}
	}  //for
		
	return 0;
}


/*
	Gestisce i Device: legge gli ingressi e li copia in memoria.
	Legge le uscite dalla memoria e li scrive sui device
	Gestisce sia la parte digitale che la parte analogica
*/
int device_manager()
{
	
	//creo le aree di memoria
	create_memories();	
	
	//creazione di tutti i tipi di driver	
	//atv_load(drv_types[0],0); //Carico il tipo di driver Autovideo

	//inizializzo tutti i device tramite file di configurazione
	//parse_config(configfile);
	
	for(;;)
	{
		delay(100); //ciclo ogni 500 ms
		//scorro la lista dei drivers
		io_manager(IOMGR_RW);
	} //for
		
	return 0;
}




