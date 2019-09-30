/* 
	Implementa il server per il protocollo modbus
 	Written By: Franco Novi
	Distelco s.r.l.
 	$Id: server_modbus.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "common.h"
#include "modbus.h"
#include "extern.h"
#include <sys/poll.h>
#include "mymemory.h"

//extern struct S_digital_memory memory_area;
extern struct S_digital_memory digital_input_area;
extern struct S_digital_memory digital_output_area;
extern struct S_digital_memory digital_prev_input_area;
extern struct S_digital_memory digital_prev_output_area;
extern struct S_analog_memory analog_input_area;
extern struct S_analog_memory analog_prev_input_area;
extern struct S_analog_memory analog_output_area;

/*
	Controlla che il function code sia corretto
	Ritorna 0 se ok, -1 errore
*/
int check_func_code (BYTE f_code)
{
	switch (f_code)
	{
	case READ_COILS:
	case WRITE_SINGLE_COIL:
	case WRITE_MUL_COIL:
	case READ_INPUT_REG:
	case READ_MUL_REG:
	case WRITE_SINGLE_REG:
	case WRITE_MUL_REG:
	case READ_DISCRETE_INPUT:
		break;
	default:
		return -1;
	}

	return 0;
}

/*
	Controlla che la richiesta non generi out of memory
	Ritorna 0 se ok, -1 errore
*/
static int check_out_of_mem (BYTE * req)
{
	WORD s_addr = get_starting_addr (req);
	WORD num = get_quantity (req);
	WORD end_addr;

	switch (req[0])
	{
	case WRITE_SINGLE_COIL:
		num = 1;
		break;
	case WRITE_SINGLE_REG:
		num = 2;
		break;
	case READ_COILS:
	case READ_DISCRETE_INPUT:
	case WRITE_MUL_COIL:
		break;
	case READ_INPUT_REG:
	case READ_MUL_REG:
	case WRITE_MUL_REG:
		num <<= 1;	//moltiplico per 2 perche' sono word
		break;
	default:
		return -1;
	}
	end_addr = s_addr + num;

	if (end_addr >= (WORD) MEM_SIZE)
		return -1;

	return 0;
}

static int check_quantity (WORD quantity)
{
	return (((quantity < 0x0001) || (quantity > 0x07d0)) ? -1 : 0);
}

/*
	Controlla se la richiesta e' corretta
*/
int check_request (BYTE * req)
{
	int err_code = 0;
	//DEBUG_LOG("Function code: %d",req[0]);
	//DEBUG_LOG("Quantita'  : %d",get_quantity(req));
	//DEBUG_LOG("Start Address: %d",get_starting_addr(req));

	if (check_func_code (req[0]) < 0)
	{
		err_code = 1;
		DEBUG_LOG ("Errore function code");
		goto end;
	}
	if ((req[0] != WRITE_SINGLE_COIL) && (req[0] != WRITE_SINGLE_REG))
	{
		if (check_quantity (get_quantity (req)) < 0)
		{
			err_code = 3;
			DEBUG_LOG ("Errore quantita' dati");
			goto end;

		}
	}
	else if (check_out_of_mem (req) < 0)
	{
		err_code = 2;
		DEBUG_LOG ("Errore out of memory");
		goto end;
	}
      end:
	return err_code;
}



/*
	Gestisce la richiesta del client e crea la risposta del server
	Ritorna la lunghezza della risposta in byte.
*/

#define BUFFER_SIZE	256

int make_resp (BYTE * request, BYTE * answer)
{
	int resp_len = 0;
	int data_number;
	int err_code = 0;
	WORD start_addr;

	if ((err_code = check_request (request)) != 0)
	{
		answer[0] = get_func_code (request) + 0x80;
		answer[1] = err_code;
		resp_len = 2;
	}
	else
	{
		data_number = (int) get_quantity (request);
		start_addr = get_starting_addr (request);

		answer[0] = get_func_code (request);

		switch (answer[0])
		{
		case READ_MUL_REG:	//Read Write // Flag interni e uscite analogiche
			{
				//leggo gli ingressi //creo la risposta
				answer[1] = 2 * pack_word ((WORD *) (&answer[2]), ((analog_output_area.head) + start_addr), data_number);
				resp_len = 2 + answer[1];
				break;
			}
		case READ_INPUT_REG:	//Read Only ingressi analogici
			{
				//leggo gli ingressi //creo la risposta
				answer[1] = 2 * pack_word ((WORD *) (&answer[2]), ((analog_input_area.head) + start_addr), data_number);
				resp_len = 2 + answer[1];
				break;
			}
		case READ_COILS:	//R/W uscite digitali
			{
				answer[1] = pack_byte (&answer[2], ((digital_output_area.head) + start_addr), data_number);
				resp_len = 2 + answer[1];
				break;
			}
		case READ_DISCRETE_INPUT:	//Read Only Ingressi digitali
			{
				answer[1] = pack_byte (&answer[2], ((digital_input_area.head) + start_addr), data_number);
				resp_len = 2 + answer[1];
				break;
			}
		case WRITE_SINGLE_REG:	//R/W Uscite analogiche + flag interni
			{
				//scrivo le uscite come da richiesta
				WORD *pdata = (WORD *) get_data_pointer (request);
				WORD value = ntohs ((*pdata));
				write_data (start_addr, value);
				//creo la risposta
				memcpy (answer, request, 5);
				resp_len = 5;
				break;
			}
		case WRITE_SINGLE_COIL:	//R/W Uscite digitali
			{
				BYTE *pdata = (BYTE *) get_data_pointer (request);
				//scrivo le uscite come da richesta
				((*pdata) > 0 ? set_data (CS (start_addr)) : reset_data (CS (start_addr)));
				//creo la risposta
				memcpy (answer, request, 5);
				resp_len = 5;
				break;
			}
		case WRITE_MUL_COIL:	//R/W Uscite digitali
			//scrivo le uscite come da richesta
			unpack_bit ((BYTE *) get_data_pointer (request), (digital_output_area.head + start_addr), data_number);
			//creo la risposta
			memcpy (answer, request, 5);
			resp_len = 5;
			break;
		case WRITE_MUL_REG:	//R/W Uscite analogiche + flag interni
			unpack_word ((WORD *) get_data_pointer (request), ((analog_output_area.head) + start_addr), data_number);
			//creo la risposta
			memcpy (answer, request, 5);
			resp_len = 5;
			break;
		default:
			{
				ERR_LOG ("Bad Request function code");
				return -1;
			}
		}
	}
	return resp_len;
}

static void change_length (BYTE * res, int length)
{
	WORD *p = (WORD *) (res + 4);
	*p = htons ((WORD) (length + 1));
	return;
}


static int check_adu (BYTE * padu, int adu_len)
{
	WORD *p = (WORD *) padu;

	if ((int) ntohs ((short) p[3]) < (adu_len - HEADER_SIZE - 1))
	{
		ERR_LOG ("check_adu: of byte in header");
		return -1;
	}
	return 0;
}


#define ADU_MAX_LEN	256
#define RESP_MAX_SIZE	256
/*
	Gestisce una richiesta di un client
	Ritorna il numero di Byte ricevuti
	Il socket deve avere dati disponibili 
*/
int mbrecv (int sockid, BYTE * pdu, long timeout)
{
	int num_bytes = 0;
	BYTE adu[ADU_MAX_LEN];
	int pdu_len = 0;
	BYTE resp[RESP_MAX_SIZE];

	//ricevo la richiesta del client
	if ((num_bytes = recv (sockid, adu, ADU_MAX_LEN, 0)) < 0)
	{
		ERR_LOG ("mbrecv:receive error %d socket %d", errno, sockid);
		return -1;
	}
	else if (num_bytes == 0)
	{
		ERR_LOG ("mbrecv:No data received");
		return 0;
	}

	//testo la correttezza dell'header
	if (check_adu (adu, num_bytes) < 0)
	{
		ERR_LOG ("mbrecv:adu error");
		return -1;
	}
	//separo l'header dal pdu e lo copio sulla risposta al client
	sep_mb_header (adu, pdu, resp, num_bytes);

	//gestisco la richiesta e creo la risposta di seguito all'header
	pdu_len = make_resp (pdu, (resp + HEADER_SIZE));
	//modifico l'header in base alla risposta
	change_length (resp, pdu_len);

	//mando la risposta al client
	if (send (sockid, resp, (HEADER_SIZE + pdu_len), 0) <= 0)
	{
		ERR_LOG ("mbrecv:Send Error");
		return -1;
	}
	return pdu_len;
}

#define MAX_RESP 1024
#define MAX_CMD	 1024

/**************************************************************************
 Questo thread si occupa di ascoltare le richieste MODBUS, permette multiconnessioni 
***************************************************************************/
#define MAX_SOCKET	10


static void listen_multinet_request_cancel (void *sockets)
{
	int i;
	int *sk;

	sk = (int *) sockets;

	DEBUG_LOG ("listen_multinet_request_cancel: executing");
	for (i = MAX_SOCKET - 1; i >= 0; i--)
	{
		if (sk[i] != -1)
		{
			shutdown (sk[i], SHUT_RDWR);
			close (sk[i]);
		}
	}
	return;

}

void *listen_multinet_request (void *arg)
{
	char *resp;
	char *mesg;
	struct pollfd ufds[MAX_SOCKET];
	struct sockaddr_in sock_from;
	int numbytes, i, res;
	/* La socket ID di indice 0 e' riservata alla listen socket */
	/* Le socket ID di indice > 0 fino a MAX_SOCKET - 1 sono riservate ai client        */
	int sockarrfd[MAX_SOCKET];
	time_t sockarrfd_timers[MAX_SOCKET];
	int client_number = 0;
	long poll_timeout = 100;
	long recv_timeout = 100;
	struct timeval tv;	//timeout della send (altrimenti si blocca fino all'arrivo di una richiesta)

	pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED, NULL);

	tv.tv_sec = 5;		// fisso il timeout della send a 5 secondi
	tv.tv_usec = 0;

	// Incremento il contatore dei thread attivi    
	th_count++;

	//inizializzazione array dei socket descriptor
	for (i = 0; i < MAX_SOCKET; i++)
	{
		sockarrfd[i] = -1;
	}

	pthread_cleanup_push (listen_multinet_request_cancel, sockarrfd);

	// create socket to host
	if ((sockarrfd[0] = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		ERR_LOG ("listen_multinet_request:socket creation error");
		pthread_exit (0);
	}

	if (fcntl (sockarrfd[0], F_SETFL, O_NONBLOCK) == -1)
	{
		close (sockarrfd[0]);
		sockarrfd[0] = -1;
		ERR_LOG ("listen_multinet_request:NONBLOCK socket setting error");
		pthread_exit (0);
	}

	/* setup source */
	i = 1;
	if (setsockopt (sockarrfd[0], SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i)) != 0)
	{
		ERR_LOG ("listen_multinet_request:cannot set socket options");
	}

	sock_from.sin_family = AF_INET;
	sock_from.sin_port = htons (SERVER_MB_PORT);
	sock_from.sin_addr.s_addr = INADDR_ANY;
	bzero (&(sock_from.sin_zero), 8);

	if (bind (sockarrfd[0], (struct sockaddr *) &sock_from, sizeof (struct sockaddr)) == -1)
	{
		close (sockarrfd[0]);
		perror (NULL);
		sockarrfd[0] = -1;
		ERR_LOG ("listen_multinet_request:port already in use");
		exit (0);
	}

	if (listen (sockarrfd[0], 1) != 0)
	{
		close (sockarrfd[0]);
		sockarrfd[0] = -1;
		ERR_LOG ("listen_multinet_request:Listen error");
		pthread_exit (0);
	}

	// Alloco lo spazio del buffer di risposta e del buffer del comando
	resp = (char *) malloc (MAX_RESP);
	mesg = (char *) malloc (MAX_CMD);

	if (resp == NULL)
	{
		ERR_LOG ("listen_multinet_request:Allocation of resp space failed");
		pthread_exit (0);
	}
	if (mesg == NULL)
	{
		ERR_LOG ("listen_multinet_request:Allocation of mesg space failed");
		pthread_exit (0);
	}

	DEBUG_LOG ("listen_multinet_request:Server Modbus Listen");
	while (1)
	{
		int i, j = 0, n_fd;

		n_fd = 0;

		for (i = 0; i < MAX_SOCKET; i++)
		{
			if (sockarrfd[i] > 0)
			{
				ufds[n_fd].fd = sockarrfd[i];
				ufds[n_fd].events = POLLIN | POLLPRI;
				n_fd++;
			}
		}

		//aggiorno il numero di client connessi
		client_number = n_fd - 1;

		res = poll (ufds, n_fd, poll_timeout);
#if 0
		if (res == 0)
		{
			// incremento il numero di cicli in cui non ho avuto richieste
			zero_poll_count++;

			// se zero_poll_count e' maggiore della soglia allora chiudo i socket aperti
			if ((zero_poll_count > poll_th) && (n_fd > 1))
			{
				for (j = 1; j < MAX_SOCKET; j++)
				{
					if (sockarrfd[j] > 0)
					{
						//lo chiudo e lo resetto
						close (sockarrfd[j]);
						DEBUG_LOG ("listen_multinet_request:Socket Listen Command Timeout");
						sockarrfd[j] = -1;
					}
				}
			}
			continue;
		}
		zero_poll_count = 0;
#endif
		if (res == -1)
		{
			ERR_LOG ("listen_multinet_request:Poll error");
			continue;
		}

		if (res > 0)
		{
			for (i = 0; i < n_fd; i++)
			{
				if (ufds[i].revents & POLLIN)
				{
					if (i == 0)
					{	/* Evento sulla Listen socket */
						j = 1;

						/* now accept connection */
						DEBUG_LOG ("listen_multinet_request:Server Modbus Accept");

						while (j < MAX_SOCKET)
						{
							if (sockarrfd[j] < 0)
								break;
							j++;
						}

						if ((j == (MAX_SOCKET - 1)) && (sockarrfd[j] > 0))
						{
							DEBUG_LOG ("listen_multinet_request:No socket avaliable");
							continue;
						}

						if ((sockarrfd[j] = accept (sockarrfd[0], NULL, 0)) == -1)
						{
							ERR_LOG ("listen_multinet_request:Cannot accept connection");
							continue;
						}
						// imposto il timeout della send al valore contenuto in tv
						setsockopt (sockarrfd[j], SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv));
						sockarrfd_timers[j] = time (NULL);

					}
					else
					{
						/* Evento sulle socket di lettura comando */
						/* Cerco il socket corrispondente in sockarrfd */
						for (j = 0; j < MAX_SOCKET; j++)
						{
							if (sockarrfd[j] == ufds[i].fd)
								break;
						}
						sockarrfd_timers[j] = time (NULL);
						memset (mesg, 0, MAX_CMD);
						if ((numbytes = mbrecv (ufds[i].fd, mesg, recv_timeout)) == -1)
						{
							/*lo chiudo e lo resetto */
							close (sockarrfd[j]);
							sockarrfd[j] = -1;
							ERR_LOG ("listen_multinet_request:No data was received from socket %d", j);
						}
						else if (numbytes == 0)
						{
							/* lo resetto e lo chiudo */
							close (sockarrfd[j]);
							sockarrfd[j] = -1;
							DEBUG_LOG ("listen_multinet_request:Command disconnect");
						}
					}
				}

				if (ufds[i].revents & POLLHUP)
				{
					for (j = 0; j < MAX_SOCKET; j++)
					{	/* Cerco il socket corrispondente in sockarrfd */
						if (sockarrfd[j] == ufds[i].fd)
						{
							close (sockarrfd[j]);
							sockarrfd[j] = -1;
							DEBUG_LOG ("listen_multinet_request:Command disconnect");
							break;
						}
					}
				}
				else if (ufds[i].revents & POLLERR)
				{
					for (j = 0; j < MAX_SOCKET; j++)
					{	/* Cerco il socket corrispondente in sockarrfd */
						if (sockarrfd[j] == ufds[i].fd)
						{
							DEBUG_LOG ("listen_multinet_request:Command ERR");
							close (sockarrfd[j]);
							sockarrfd[j] = -1;
							break;
						}
					}
				}
				else if (ufds[i].revents & POLLNVAL)
				{
					for (j = 0; j < MAX_SOCKET; j++)
					{	/* Cerco il socket corrispondente in sockarrfd */
						if (sockarrfd[j] == ufds[i].fd)
						{
							DEBUG_LOG ("listen_multinet_request:Command NVAL");
							close (sockarrfd[j]);
							sockarrfd[j] = -1;
							break;
						}
					}
				}
				else if (ufds[i].revents & POLLPRI)
				{
					DEBUG_LOG ("listen_multinet_request:Command PRI data");
				}
				else if (ufds[i].revents & POLLOUT)
				{
					DEBUG_LOG ("listen_multinet_request:Command POLLOUT");
				}

			}	// socket loop
		}		// if

		{
			time_t now;
			now = time (NULL);
			// controllo timeout su socket aperti
			for (j = 1; j < MAX_SOCKET; j++)
			{
				if (sockarrfd[j] > 0 && (now - sockarrfd_timers[j]) > 60)
				{
					//lo chiudo e lo resetto
					close (sockarrfd[j]);
					sockarrfd[j] = -1;
					DEBUG_LOG ("listen_multinet_request:Socket Timeout");
				}
			}
		}


	} // while (1)
	DEBUG_LOG ("listen_multinet_request:exit");
	free (resp);
	free (mesg);

	pthread_cleanup_pop (1);
	pthread_exit (0);
}
