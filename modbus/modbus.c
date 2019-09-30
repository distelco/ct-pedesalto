/*
	modbus.c
	Contiene le definizioni delle funzioni del protocollo modbus
	per implementare il client modbus
	Written by Franco Novi
	Release: $Id: modbus.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <errno.h>
#include "common.h"
#include "modbus.h"
#include "memory.h"

static const int get_pdu_len( BYTE *pdu )
{
	int ret = 5;
	switch(get_func_code(pdu))
	{
	case WRITE_MUL_COIL:
	case WRITE_MUL_REG:
	case RW_MUL_REG:
		{
		ret = 5 + get_byte_count(pdu) + 1;
		break;
		}
	default:
		ret = 5;
		break;
	}
	return ret;
}


//Funzioni per l'implementazione del client modbus

/*
	Data una function code in ingresso crea il PDU (Protocol Data Unit)
*/
void make_pdu( BYTE func_code, struct S_req_details *arg, BYTE *ppdu)
{
	switch(func_code)
	{
	case WRITE_SINGLE_COIL:
		{
		short arr[2];
		ppdu[0] = func_code;
		arr[0] = htons(arg->start_addr);
		arr[1] = htons((WORD)((*(arg->value))>0 ? 0xff00 : 0x0000 ));
		memcpy( &ppdu[1], (BYTE *)arr, 2*sizeof(short));
		break;
		}
	case WRITE_SINGLE_REG:
		{
		short arr[2];
		ppdu[0] = func_code;
		arr[0] = htons(arg->start_addr);
		arr[1] = htons((WORD)*(arg->value));
		memcpy( &ppdu[1], (BYTE *)arr, 2*sizeof(short));		
		break;
		}
	case WRITE_MUL_REG:
	case WRITE_MUL_COIL:
		{
		short arr[2];
		ppdu[0] = func_code;
		arr[0] = htons(arg->start_addr);
		arr[1] = htons((short)arg->num);
		memcpy( &ppdu[1], (BYTE *)arr, 2*sizeof(short));
		ppdu[5] = arg->byte_count;
		memcpy(&ppdu[6],arg->value,arg->byte_count);
		break;
		}
	case RW_MUL_REG:
		break;
	case READ_COILS:
	case READ_DISCRETE_INPUT:
	case READ_MUL_REG:
	case READ_INPUT_REG:
		{
		short arr[2];
		ppdu[0] = func_code;
		arr[0] = htons(arg->start_addr);
		arr[1] = htons((short)arg->num);
		memcpy( &ppdu[1], (BYTE *)arr, 2*sizeof(short));
		break;
		}
	}
	return;
}

/*
	Crea un header per il protocollo modbus su ethernet
	L'header MODBUS ha sempre lunghezza fissa 7 byte
*/

/*
	Dato un pdu (Protocol Data Unit) crea l'Application Data Unit (ADU) aggiungendo l'header
*/
int add_mb_header( BYTE *adu, BYTE *pdu, BYTE header_id, BYTE unit_id)
{
	short *len;
	BYTE pdu_len = get_pdu_len(pdu);
	if(adu == NULL)
	{
		ERR_LOG("Bad header pointer");
		return -1;
	}
	
	adu[0] = pdu[0];	//transaction identifier high
	adu[1] = header_id;	//transaction identifier low
	adu[2] = 0;		//protocol identifier = 0x0000 high
	adu[3] = 0;		//protocol identifier = 0x0000 low
	len = (short*) &adu[4]; //puntatore short di appoggio
	*len = htons( (1+pdu_len)); // setto il byte count a seconda del codice funzione
	adu[6] = unit_id;	//identificativo di unita' , non utilizzato
	
	//aggiungo pdu al pacchetto finale
	memcpy(adu+HEADER_SIZE,pdu, pdu_len );
	//aggiungo il terminatore ala fine
	adu[HEADER_SIZE+pdu_len+1] = '\0';
	
	return 0;
}
/*
	Dato un Application Data Unit (ADU) separa l'header ed il pdu (Protocol Data Unit)
*/
int sep_mb_header(BYTE * adu, BYTE * pdu, BYTE * header, int adu_len)
{
	memcpy(header, adu, HEADER_SIZE);
	memcpy(pdu, (adu+HEADER_SIZE),adu_len);
	return 0;
}

/*
*/
static int check_TID_code( BYTE *sent, BYTE *resp)
{
    if ( sent[0] == resp[0] && sent[1] == resp[1] )
	return 1;
	
    return 0;
}

#define RESP_SIZE 	256
#define MSG_SIZE	256
 

extern int make_resp( BYTE * request, BYTE * answer);

/*
	Spacchetta i dati letti da bit a byte
	pack e' il pdu, unpack contiene i dati spacchettati
	n e' la dimensione di unpack in byte
	Ritorna il numero di byte spacchettati
*/
int unpack_bit( BYTE*pack, BYTE*unpack, int n)
{
	int num=0;
	int i,j;
	//BYTE byte_count = pack[1];
	//BYTE *data = &pack[2]; //punta ai dati saltando function code e byte_count
	BYTE byte_count = n / 8 + ((n%8)>0);
	for(i=0; i<(int)byte_count;i++)
	{
		int k = i*8;
		for(j=0;j<8;j++)
		{
			if (num >= n)
				break;

			unpack[k+j] = pack[i] & 0x01;
			pack[i] >>= 1;
			num++;
		}
	}
	return num;
}

/*
	Spacchetta i dati letti da formato network a formato host
	pack e' l'pdu, unpack contiene i dati spacchettati
	Ritorna il numero di word spacchettate
*/
int unpack_word( WORD *pack, long *unpack, int n)
{
	int i;
	for(i=0;i<n;i++)
	{
		unpack[i] = (long) ntohs(pack[i]);
		//unpack[i] = (long) pack[i];
	}
	return (i);
}
/*
	Impacchetta n dati digitali da byte a bit
	Ritorna il numero di byte utilizzati per l'impacchettamento
*/
int pack_byte(BYTE*pack, BYTE*unpack, int n)
{
	BYTE mask = 0x01;
	int i,k;
	int pack_num = (n/8)+((n%8)!=0);
	
	for(i=0; i< pack_num; i++)
	{
		pack[i]=0;
	}
	
	for(i=0;i<n;i++)
	{
		k = i % 8;
		pack[(i/8)] |= ( unpack[i]>0 ? mask<<k : 0 );
	}

	return pack_num;
}
/*
	Prepara i valori delle uscite analogiche nel formato network
	n = numero di word da ordinare
	Ritorna il numero di word ordinate
*/
int pack_word(WORD *pack, long *unpack, int n)
{
	int i;
	for(i=0;i<n;i++)
	{
		pack[i] = htons((WORD)unpack[i]);
		//pack[i] = (WORD)unpack[i];
	}

	return (i);
}

/*
    Modbus TCP
    Spedisce il PDU (protocol data unit) via ethernet tramite socket sockid
    Il socket deve essere gia' aperta e connessa all'host remoto
*/
static unsigned char mbsend_counter ;
int mbTCP_send( int sockid, BYTE* ppdu, char*resp, long timeout)
{
	int ret, nread=0, ppdu_len = get_pdu_len(ppdu);
	struct pollfd ufds;
	BYTE msg[MSG_SIZE];
	BYTE server_resp[RESP_SIZE];
	
	//azzero i buffer
	memset(msg,0,256);
	memset(server_resp,0,256);
	mbsend_counter++ ;
	//copio pdu su msg e gli aggiungo l'header ed il teminatore '\0'
	add_mb_header( msg, ppdu, mbsend_counter, 1);
	
	//spedisco il pacchetto
	ufds.fd = sockid;
	ufds.events = POLLOUT;
	if ((ret = poll (&ufds, 1, timeout)) > 0)
	{
		if (ufds.revents & POLLHUP || ufds.revents & POLLERR || ufds.revents & POLLNVAL)
		{
			DEBUG_LOG ("mbTCP_send:CLIENT_DISCONNECT");
			return -1;
		}
		if ( ufds.revents & POLLOUT )
		{
			if ( send( ufds.fd, msg, (HEADER_SIZE+ppdu_len), 0) < 0 )
			{
				if ( errno != EAGAIN )
				{
					ERR_LOG("mbTCP_send:Send Error %d: socket %d",errno, sockid );
					return -1;
				} else {
					ERR_LOG("mbTCP_send:Send Error EAGAIN: socket %d",sockid );
				}
			}
		}
	} else {
		DEBUG_LOG( "mbTCP_send:write poll error %d: socket %d",errno, sockid );
		return -1 ;
	}

	/* poll the sockets for activity */
	ufds.fd = sockid;
	ufds.events = POLLIN | POLLPRI;
	if ((ret = poll (&ufds, 1, timeout)) > 0)
	{
		if (ufds.revents & POLLHUP || ufds.revents & POLLERR || ufds.revents & POLLNVAL)
		{
			DEBUG_LOG ("mbTCP_send:CLIENT_DISCONNECT");
			return -1;
		}
		if (ufds.revents & POLLIN)
		{
			//ricevo la risposta
			nread = recv (sockid, server_resp, RESP_SIZE, 0);
			if( nread < HEADER_SIZE )
			{
				ERR_LOG("mbTCP_send:recv error : socket %d",sockid );
				return -1;
			}
		}
	} else {
		ERR_LOG("mbTCP_send:read poll error %d: socket %d",errno, sockid );
		return -1;
	}
	
	if( (unsigned char)server_resp[7] >= ERR_CODE_BASE )
	{
		ERR_LOG("mbTCP_send:Server error code");
		return -1;
	} 
	
	if( check_TID_code( msg, server_resp) == 0 )
	{
		ERR_LOG("mbTCP_send:Bad TID in response");
		return -1;
	} 
	memcpy(resp, (server_resp+HEADER_SIZE), (nread-HEADER_SIZE)); //tolgo l'header e copio la risposta nel buffer di risposta
	
	return (nread-HEADER_SIZE);
}

unsigned short CRC16 (unsigned char *puchMsg, unsigned short usDataLen);

/*
    Modbus RTU su porta seriale
*/
int mbRTU_send( int fd, int slaveid, BYTE*ppdu, char *resp, long timeout)
{
	return 0;
}

/*
    Modbus RTU su tunnel TCP-seriale
*/
int mbRTUtun_send( int sockid, int slaveid, BYTE *ppdu, char *resp, long timeout)
{
	int ret, nread=0, ppdu_len = get_pdu_len(ppdu);
	struct pollfd ufds;
	BYTE msg[MSG_SIZE];
	BYTE server_resp[RESP_SIZE];
	unsigned short crc;
	
	//azzero i buffer
	memset(msg,0,256);
	memset(server_resp,0,256);
	
	// preparo il dato da inviare
	msg[0] = slaveid ;
	memcpy( &(msg[1]), ppdu, ppdu_len ) ;
	// calcolo il CRC16 del pacchetto
	crc = CRC16( msg, ppdu_len+1 ) ;	
	
	msg[ppdu_len+1] = (crc & 0xff);
	msg[ppdu_len+2] = ((crc>>8) & 0xff);
	
	//spedisco il pacchetto
	ufds.fd = sockid;
	ufds.events = POLLOUT;
	if ((ret = poll (&ufds, 1, timeout)) > 0)
	{
		if (ufds.revents & POLLHUP || ufds.revents & POLLERR || ufds.revents & POLLNVAL)
		{
			DEBUG_LOG ("mbRTUtun_send:CLIENT_DISCONNECT");
			return -1;
		}
		if ( ufds.revents & POLLOUT )
		{
			if ( send( ufds.fd, msg, (ppdu_len+3), 0) < 0 )
			{
				if ( errno != EAGAIN )
				{
					ERR_LOG("mbRTUtun_send:Send Error %d: socket %d",errno, sockid );
					return -1;
				} else {
					ERR_LOG("mbRTUtun_send:Send Error EAGAIN: socket %d",sockid );
				}
			}
		}
	} else {
		DEBUG_LOG( "mbRTUtun_send:write poll error %d: socket %d",errno, sockid );
		return -1 ;
	}
	DEBUG_LOG ("mbRTUtun_send:data sent len %d", ppdu_len+3);

	/* poll the sockets for activity */
	ufds.fd = sockid;
	ufds.events = POLLIN | POLLPRI;
	if ((ret = poll (&ufds, 1, timeout)) > 0)
	{
		if (ufds.revents & POLLHUP || ufds.revents & POLLERR || ufds.revents & POLLNVAL)
		{
			DEBUG_LOG ("mbRTUtun_send:CLIENT_DISCONNECT");
			return -1;
		}
		if (ufds.revents & POLLIN)
		{
			//ricevo la risposta
			nread = recv (sockid, server_resp, RESP_SIZE, 0);
			if( nread < 2 )  //////// FIXME
			{
				ERR_LOG("mbRTUtun_send:recv error : socket %d",sockid );
				return -1;
			}
		}
	} else if ( ret == 0 ) {
		ERR_LOG("mbRTUtun_send:read poll timeout : socket %d",sockid );
		return -1;	    
	} else {
		ERR_LOG("mbRTUtun_send:read poll error %d: socket %d",errno, sockid );
		return -1;
	}
	
	if( (unsigned char)server_resp[1] >= ERR_CODE_BASE )
	{
		ERR_LOG("mbRTUtun_send:Server error code");
		return -1;
	} 
	
	memcpy(resp, (server_resp+1), (nread-1)); //tolgo l'header e copio la risposta nel buffer di risposta
	
	return (nread-1);

}
