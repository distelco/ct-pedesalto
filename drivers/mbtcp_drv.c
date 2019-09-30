/*
	modbus.c
	File per l'interfaccia tra il device manager e le unita' locali
	Written by Franco Novi
	$Id: mbtcp_drv.c,v 1.2 2009-01-29 10:32:55 matteoc Exp $
*/

#undef DEBUG

#include <stdio.h>
#include "modbus.h"
#include "common.h"
#include <errno.h>
#include "mb_drv.h"
#include "mymemory.h"

//static S_mb_pdu mb_req_pdu;
//static S_mb_pdu mb_rsp_pdu;



#define MB_TIMEOUT	1000
#define BUF_SIZE	1024

/*
	Funzione per la lettura degli ingressi digitali
	struct S_modbus_data definita in mb_drv.h contiene le info per la lettura degli input digitali
	buf contiene le porte lette (1 porta => 1byte)
	buflen e' la lunghezza di buf in byte
*/
static int modbus_read_digital( void * data, char *buf, int buflen )
{
	BYTE pdu[5];		// Application Data Unit
	BYTE resp[BUF_SIZE];	//contiene la risposta del server, cioe' i dati letti oppure errore
	struct S_req_details det;
	struct S_modbus_data *pdata = (struct S_modbus_data*) data;
	int how_many = (2*buflen < pdata->coil_num ? 2*buflen : pdata->coil_num);

	// gestione frequenza rinfresco
	if ( pdata->io_freq != 0 )
	{
	    pdata->io_last--;
	    if ( pdata->io_last )
		    return 1;
	    else
		pdata->io_last = pdata->io_freq;
	}	
	
	//creo il socket e mi connetto
	if ((tcp_connect(pdata->tcp)) < 0)
	{
		DEBUG_LOG("modbus_read_di:connect in progress %s",pdata->tcp->ip);
		return -1;
	}
	
	det.start_addr = pdata->starting_addr;	//remote data starting address
	det.num = how_many;		//request data number	

	//creo il pacchetto modbus
	if ( pdata->area == Input ) 
	{
		make_pdu( READ_DISCRETE_INPUT, &det, pdu );
	} else
	if ( pdata->area == Coil ) 
	{
		make_pdu( READ_COILS, &det, pdu );
	} else
	{
		ERR_LOG("modbus_read_di:Richiesta invalida" ) ;
	}
	
	//spedisco ed aspetto la risposta 
	if ( mbTCP_send( pdata->tcp->sock, pdu, resp, pdata->tcp->timeout) < 0)
	{
		//gestisco un eventuale errore
		ERR_LOG("modbus_read_di:Impossibile inviare il pacchetto");
		tcp_disconnect(pdata->tcp);
		return -1;
	}
	//spacchetto i dati ricevuti e ritorno il numero di dati letti
	unpack_bit((resp+2),buf,how_many);
	return 0;
}
/*
	buflen e' la lunghezza di buf in byte
	Return value: 0 Success, -1 error
*/

static int modbus_write_digital( void * data, char *buf, int buflen )
{
	BYTE pdu[32];		// Application Data Unit
	BYTE resp[BUF_SIZE];	//contiene la risposta del server, cioe' i dati letti oppure errore
	BYTE send[BUF_SIZE];	//dati da inviare
	struct S_req_details det;
	struct S_modbus_data *pdata = (struct S_modbus_data*) data;
	int how_many = (buflen < pdata->coil_num ? buflen : pdata->coil_num);

	// gestione frequenza rinfresco
	if ( pdata->io_freq != 0 )
	{
	    pdata->io_last--;
	    if ( pdata->io_last )
		    return 1;
	    else
		pdata->io_last = pdata->io_freq;
	}	
	

	//creo il socket e mi connetto se necessario
	if ((tcp_connect(pdata->tcp)) < 0)
	{
		DEBUG_LOG("modbus_write_do:connect in progress %s",pdata->tcp->ip);
		return -1;
	}
	det.start_addr = pdata->starting_addr;	//remote data starting address
	det.num = how_many;		//request data number
	det.byte_count = (how_many / 8) + ((how_many % 8) > 0);
	det.value = send; //(BYTE *) malloc( det.byte_count );
	memset(det.value,0,det.byte_count);
	//impacchetto i dati
	pack_byte(det.value,buf,how_many);
	//creo il pacchetto modbus
	if ( how_many > 1 )
		make_pdu( WRITE_MUL_COIL, &det, pdu );
	else {
		make_pdu( WRITE_SINGLE_COIL, &det, pdu );
	}
	//spedisco ed aspetto la risposta 
	if ( mbTCP_send( pdata->tcp->sock, pdu, resp, pdata->tcp->timeout) < 0)
	{
		//gestisco un eventuale errore
		//free(det.value);
		ERR_LOG("modbus_write_do:Impossibile inviare il pacchetto");
		tcp_disconnect(pdata->tcp);
		return -1;
	}
	return 0;
}
/*
	buflen e' il numero la lunghezza di buf in long
	Return value: 0 Success, -1 error
*/
static int modbus_read_analog( void * data, long *buf, int buflen )
{
	BYTE pdu[5];		// Application Data Unit
	BYTE resp[BUF_SIZE];	//contiene la risposta del server, cioe' i dati letti oppure errore
	struct S_req_details det;
	struct S_modbus_data *pdata = (struct S_modbus_data*) data;
	int how_many = ( buflen < pdata->coil_num ? buflen : pdata->coil_num);

	// gestione frequenza rinfresco
	if ( pdata->io_freq != 0 )
	{
	    pdata->io_last--;
	    if ( pdata->io_last )
		    return 1;
	    else
		pdata->io_last = pdata->io_freq;
	}	
	
	//creo il socket e mi connetto	
	if ((tcp_connect(pdata->tcp)) < 0)
	{
		DEBUG_LOG("modbus_read_ai:connect in progress %s", pdata->tcp->ip);
		return -1;
	}

	det.start_addr = pdata->starting_addr;	//remote data starting address
	det.num = how_many;		//request data number	

	//creo il pacchetto modbus
	if ( pdata->area == Analog ) 
	{
		make_pdu( READ_INPUT_REG, &det, pdu );
	} else
	if ( pdata->area == Register ) 
	{
		make_pdu( READ_MUL_REG, &det, pdu );
	} else
	{
		ERR_LOG("modbus_read_di:Richiesta invalida" ) ;
	}
	//spedisco ed aspetto la risposta 
	if ( mbTCP_send( pdata->tcp->sock, pdu, resp, pdata->tcp->timeout) < 0)
	{
		//gestisco un eventuale errore
		ERR_LOG("modbus_read_ai:Impossibile inviare il pacchetto");
		tcp_disconnect(pdata->tcp);
		return -1;
	}
	unpack_word((WORD*)(resp+2),buf,how_many);
	
	return 0;
}

/*
	buflen e' il numero la lunghezza di buf in long
	Return value: 0 Success, -1 error
*/
static int modbus_write_analog( void * data, long *buf, int buflen )
{
	BYTE pdu[32];		// Application Data Unit
	BYTE resp[BUF_SIZE];	//contiene la risposta del server, cioe' i dati letti oppure errore
	BYTE send[BUF_SIZE];   
	struct S_req_details det;
	struct S_modbus_data*pdata = (struct S_modbus_data*) data;
	int how_many = ((buflen) < pdata->coil_num ? (buflen) : pdata->coil_num);

	// gestione frequenza rinfresco
	if ( pdata->io_freq != 0 )
	{
	    pdata->io_last--;
	    if ( pdata->io_last )
		    return 1;
	    else
		pdata->io_last = pdata->io_freq;
	}	
	
	//creo il socket e mi connetto
	if ((tcp_connect(pdata->tcp)) < 0)
	{
		DEBUG_LOG("modbus_write_ao:connect in progress %s",pdata->tcp->ip);
		return -1;
	}
	det.start_addr = pdata->starting_addr;	//remote data starting address
	det.num = how_many;		//request data number
	det.byte_count = how_many * 2;
	det.value = send; //(BYTE *) malloc( det.byte_count );
	memset(det.value,0,how_many);
	//impacchetto i dati per spedirli
	pack_word((WORD *)(det.value),buf,how_many);
	//creo il pacchetto modbus
	make_pdu( WRITE_MUL_REG, &det, pdu );
	//spedisco ed aspetto la risposta 
	if ( mbTCP_send( pdata->tcp->sock, pdu, resp, pdata->tcp->timeout) < 0)
	{
		//gestisco un eventuale errore
		//free(det.value);
		ERR_LOG("modbus_write_ao:Impossibile inviare il pacchetto");
		tcp_disconnect(pdata->tcp);
		return -1;
	}
	return 0;
}


static int modbus_init_block()
{
	return 0;
}

struct S_driver_descriptor remote_io_driver_descriptor =
{
	"io remoti",		//	char name[64];
	drv_remote_io_did,	//	long did ;
	modbus_read_digital,	//	int (* read_di)   ( void * data, char *buf, int buflen )
	modbus_write_digital,	//	int (* write_do)  ( void * data, char *buf, int buflen );
	modbus_read_analog,	//	int (* read_ai)   ( void * data, long *buf, int buflen );
	modbus_write_analog,	//	int (* write_ao)  ( void * data, long *buf, int buflen );
	NULL,			//	int (* read_mem)  ( void * data, char *buf, int buflen );
	NULL,			//	int (* write_mem) ( void * data, char *buf, int buflen );
	modbus_init_block,	//	int (* init_block)( struct S_driver_io_block * 	 , FILE* );
	NULL,			//	int (* init_drv)  ( struct S_driver_descriptor * , FILE* );
	NULL			//	int (* flush_io)  ( void );
};


struct S_driver_descriptor mbtcp_io_driver_descriptor =
{
	"io remoti",		//	char name[64];
	drv_mbtcp_io_did,	//	long did ;
	modbus_read_digital,	//	int (* read_di)   ( void * data, char *buf, int buflen )
	modbus_write_digital,	//	int (* write_do)  ( void * data, char *buf, int buflen );
	modbus_read_analog,	//	int (* read_ai)   ( void * data, long *buf, int buflen );
	modbus_write_analog,	//	int (* write_ao)  ( void * data, long *buf, int buflen );
	NULL,			//	int (* read_mem)  ( void * data, char *buf, int buflen );
	NULL,			//	int (* write_mem) ( void * data, char *buf, int buflen );
	modbus_init_block,	//	int (* init_block)( struct S_driver_io_block * 	 , FILE* );
	NULL,			//	int (* init_drv)  ( struct S_driver_descriptor * , FILE* );
	NULL			//	int (* flush_io)  ( void );
};

