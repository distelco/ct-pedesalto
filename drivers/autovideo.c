/*
    autovideo.c driver
    Written by Franco Novi
    $Id: autovideo.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#include "../config.h"
#include "common.h"
#include <stdio.h>
#include "autovideo.h"
#include <ctype.h>
#include <errno.h>
#include <sys/poll.h>

enum E_atv_req {
	STATE,
	GRAPH1,
	GRAPH2	
};

int AV_TCP_PORT;

//--------------------------------------------------------
/*
Invia comandi al client per controllare lo stato
*/
//--------------------------------------------------------
static inline int
client_request (int sockfd, enum E_atv_req req_type)
{
	int ret = -1;
	struct pollfd ufds;

	ufds.fd = sockfd;
	ufds.events = POLLOUT;

	/* send request to autovideo */
	if (sockfd > -1)
	{
		int nreq;
		char state[16] ;

		nreq = poll (&ufds, 1, 200);
		if (nreq <= 0)
		{
			return -1;
		}
		if (ufds.revents & POLLHUP || ufds.revents & POLLERR || ufds.revents & POLLNVAL)
		{
			return -1;
		}

		if (ufds.revents & POLLOUT)
		{
			switch (req_type)
			{
			case STATE:
				strncpy(state, "1201 0 ", 16) ;
				break ;
			case GRAPH1:
				strncpy(state, "1404 0 ", 16) ;
				break ;
			case GRAPH2:
				strncpy(state, "1405 0 ", 16) ;
				break ;
			default:
				req_type=STATE;
			}
				ret = send (sockfd, state, strlen(state), 0);
		}
	}
	else
	{
		return -1;
	}
	return ret;
}


//--------------------------------------------------------
/* 
Processa le risposte ai comandi inviati ad autovideo
il socket DEVE contenere dati da leggere o sospende il processo
il socket deve essere gia' connesso. 
il socket non viene chiuso all'uscita.
*/
//--------------------------------------------------------
int
client_process (int sockfd, char *nebbia)
{

	//int ret;

	//char *p ;
	int lenbuf;
	int cmd, ch, val[5];
	int res;
	char atv_resp[1024];

	//memset (buf, 0, buf_size);
	lenbuf = recv (sockfd, atv_resp, 1024, 0);
	if (lenbuf < 0)
	{
		if (errno == ECONNRESET)
		{
			INFO_LOG (" client:connection reset :%s\n", strerror (errno));
			return -1;
		}
		else
		{
			ERR_LOG ("client:%s\n", strerror (errno));
			return -1;
		}
	} else if (lenbuf == 0)
	{
		DEBUG_LOG ("client: connection closed\n");
		return -1;
	}
	DEBUG_LOG ("client<<%s\n", atv_resp);

	res = 0;
	res = sscanf (atv_resp, "%d %d %d %d %d %d %d", &cmd, &ch, &val[0], &val[1], &val[2], &val[3], &val[4]);
	/*------------ leggo lo stato iniziale ------------ */

	switch (cmd)
	{
	case 1201:
		// cmd          = identificativo del comando, per la lettura dello stato 1201
		// ch           = identificativo del canale, per il sistema nebbia e' il canale 0
		// val[0]       = stato globale del sistema nebbia
		// val[1]       = stato di presenza nebbia all'istante corrente
		// val[2]       = stato di scorretto posizionamento del target 
		// val[3]       = stato di presenza nebbia per un prolungato periodo di tempo
		// val[4]       = numero di client connessi ad Autovideo
		
		*nebbia = (char) val[1];		 
		break;
	case 1404:
#if 0
		p = buf ;
		P_I(i)	;	// cmd
		P_I(i)	;	// ch
		P_I(stato.dyn_th) ;
		P_I(stato.white_avg) ;
		P_I(stato.ext_avg) ;
		P_F(stato.k_factor) ;
		P_F(stato.count_th) ;
#endif
		break;
	case 1405:
#if 0
		p = buf ;
		P_I(i)	;	// cmd
		P_I(i)	;	// ch
		for (i=0;i<10;i++)
		{
			P_F(stato.edge_dmin[i])	;
			P_F(stato.edge_val[i]) ;
		}
#endif
		break;
	default:
		break;
	}
	return 0;
}


static int atv_read(void * data , char *pbuf, int pbuflen)
{
	int ret, i=0;
	struct pollfd ufds;
	struct S_atv_data  *atv_data = (struct S_atv_data  *) data;
	if (data == NULL)
	{
		ERR_LOG("NULL data pointer");
		return -1;
	}

	ufds.fd = -1;
	ufds.events = POLLIN | POLLPRI;
	AV_TCP_PORT = atv_data->tcp->port;

	//apre il socket
	//mi connetto ad autovideo
	if( (ufds.fd = tcp_connect(atv_data->tcp))< 0)
	{
	    ERR_LOG("Errore di connessione ad autovideo");
	    return -1;
	}
	/*
		i = 0 = STATE
		i = 1 = GRAPH1
		i = 2 = GRAPH2
	*/
	for (i = 0; i < pbuflen; i++)
	{
		if (client_request(ufds.fd, i) < 0)
		{
			DEBUG_LOG ("REQUEST_FAIL: %d",i);
			continue;
		}				

		/* poll the sockets for activity */
		ret = poll (&ufds, 1, 100);
		if (ret > 0)
		{
			if (ufds.revents & POLLHUP || ufds.revents & POLLERR || ufds.revents & POLLNVAL)
			{
				DEBUG_LOG ("CLIENT_DISCONNECT\n");
				tcp_disconnect(atv_data->tcp);
				ufds.fd = -1;
			}
			if (ufds.revents & POLLIN)
			{
				// leggo lo stato dal client solo se ho inviato la richiesta precedentemente
				if (client_process (ufds.fd, &pbuf[i] )< 0)
				{
					tcp_disconnect(atv_data->tcp);
				}

			}
		}
	}
	
	return 0;
}

static int atv_write(void * data, char *pbuf, int pbuflen)
{
	if (data == NULL)
	{
		ERR_LOG("NULL data pointer");
		return -1;
	}
	
	return 0;
}

static inline int parse_atv( struct S_driver_io_block *pb, FILE *f)
{
	char line[1024];
	char cmd[64];
	const char *p;
	int errors, line_num;
	struct S_atv_data *pdata = (struct S_atv_data *) pb->drv_info;
	
	if( pdata == NULL)
	{
		ERR_LOG("parse_atv: NULL input pointer");
		return -1;
	}

	errors = 0;
	line_num = 0;
	for(;;) 
	{
		if (fgets(line, sizeof(line), f) == NULL)
			break ;
		line_num++ ;
		p = line ;
		while (isspace(*p))
			p++;
		if (*p == '\0' || *p == '#')
			continue;

		get_arg(cmd, sizeof(cmd), &p) ;
		
		if (!strcasecmp(cmd, "base")) {
			SET_NUM( pb->base );
		}
		if (!strcasecmp(cmd, "lenght")) {
			SET_NUM( pb->lenght );
		}				
		if (!strcasecmp(cmd, "IP Address")) {
			SET_STRING( pdata->tcp->ip );
		}
		if (!strcasecmp(cmd, "channel")) {
			SET_NUM( pdata->channel );
		}
		if (!strcasecmp(cmd, "CMDPORT")) {
			SET_NUM( pdata->tcp->port );
			AV_TCP_PORT = pdata->tcp->port;
		}
//		if (!strcasecmp(cmd, "CMD code")) {
//			SET_STRING( pdata->cmd_code );
//		}		
	}
	return 0;
}
/*
	inizializza i dati del device autovideo
	Nome e did devono essere gia' impostati da dev_mgr.c, load_driver()
*/
static int atv_init( struct S_driver_io_block *pblock, FILE*f)
{
	if( f == NULL )
	{
		ERR_LOG("File di configurazione inesistente");
		return -1;
	}
	if(pblock == NULL)
	{
		ERR_LOG("Puntatore ad io_block nullo");
		return -1;
	}
		
	//alloco lo spazio in memoria per i dati
	pblock->drv_info = (struct S_atv_data *) malloc(sizeof(struct S_atv_data));
	// copio tutti i dati da file di configurazione ad io_block
	parse_atv(pblock, f);
	
	return 0;
}

int atv_load( struct S_driver_descriptor * pdd, int id)
//int atv_load( struct S_driver_descriptor * pdd)
{
	pdd = create_drv_type();
	strcpy( pdd->name, "autovideo" );
	pdd->did = id;
	pdd->read_digital = atv_read;
	pdd->init_block = atv_init;
	pdd->write_digital = atv_write;
	return 0;
}

struct S_driver_descriptor atv_driver_descriptor =
{
	"autovideo",		//	char name[64];
	drv_atv_did,		//	long did ;
	atv_read,		//	int (* read_di)   ( void * data, char *buf, int buflen )
	atv_write,		//	int (* write_do)  ( void * data, char *buf, int buflen );
	NULL,			//	int (* read_ai)   ( void * data, long *buf, int buflen );
	NULL,			//	int (* write_ao)  ( void * data, long *buf, int buflen );
	NULL,			//	int (* read_mem)  ( void * data, char *buf, int buflen );
	NULL,			//	int (* write_mem) ( void * data, char *buf, int buflen );
	atv_init,		//	int (* init_block)( struct S_driver_io_block * 	 , FILE* );
	NULL,			//	int (* init_drv)  ( struct S_driver_descriptor * , FILE* );
	NULL			//	int (* flush_io)  ( void );
};


