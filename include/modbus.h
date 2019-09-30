
/*
	modbus.h
	Contiene definizioni delle funzioni del protocollo modbus
	Written by Franco Novi
	Release: $Id: modbus.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#if !defined(__MODBUS_H__)

#define  __MODBUS_H__

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "devices.h"
#include "common.h"


#define HEADER_SIZE	7
#define MB_PORT	502
#define SERVER_MB_PORT	502

#define BYTE	unsigned char
#define WORD	short

typedef enum {
	read_coils = 1,
	read_input_discrete = 2,
	read_mul_reg = 3,
	read_input_reg = 4,
	write_single_coil = 5,
	write_single_reg = 6,
	write_multiple_coils = 15,
	write_mul_reg = 16,
	rw_mul_reg = 23,
	mask_write_reg = 22,
	read_file_rec = 20,
	write_file_rec = 21,
	read_dev_id = 43
} E_fcode;

#define READ_COILS 		0x01
#define	READ_DISCRETE_INPUT	0x02
#define READ_MUL_REG		0x03
#define READ_INPUT_REG		0x04
#define WRITE_SINGLE_COIL	0x05
#define WRITE_SINGLE_REG	0x06
#define WRITE_MUL_COIL		0x0f
#define WRITE_MUL_REG		0x10
#define RW_MUL_REG		0x17
#define MASK_WRITE_REG		0x16
//#define READ_FILE_REG		0x14
//#define WRITE_FILE_REG	0x15
//#define READ_DEV_ID		0x2b

struct S_req_details {
	WORD		start_addr;	//remote data starting address
	WORD 		num;		//request data number	
	BYTE 		byte_count;	//Multiple read e write
	BYTE		*value;		//Multiple read e write
};

struct S_mb_req {
	BYTE	function_code;	//request modbus code
	struct S_req_details req_details;
} ;

#define ERR_CODE_BASE	0x80

struct S_mb_excep_rsp_pdu{
	BYTE 	function_code;
	BYTE	exception_code;
} ;

struct S_mb_excep_rsp_pdu mb_excep_rsp_pdu;


// 	Data type		Type of access			Comments
//
//	Coils		 (BIT)	Read - Write		This type of data can be alterable by an application program
//	Discretes Input	 (BIT)	Read - Only		This type of data can be provided by an I/O system
//	Input Registers  (WORD)	Read - Only		This type of data can be provided by an I/O system		
//	Holding Registers(WORD)	Read - Write		This type of data can be alterable by an application program

//int make_mb_header( BYTE*header, BYTE code, BYTE header_id, BYTE unit_id);
//char *make_mb_tcpip_pdu( void*header,BYTE*pdu);

void make_pdu( BYTE func_code, struct S_req_details *arg, BYTE*ppdu );

static inline const BYTE get_func_code( BYTE * pdu)
{
	return pdu[0];
}

static inline BYTE get_byte_count(BYTE * pdu)
{
	BYTE f_code = get_func_code(pdu);
	
	if( f_code == WRITE_MUL_COIL)
	{
			return pdu[5];
	} 
	if( f_code == WRITE_MUL_REG)
	{
			return pdu[5];
	} 
	ERR_LOG("Codice %x non implementato", f_code ) ;
	return 0;
}


static inline WORD get_starting_addr( BYTE * pdu )
{
	WORD * p;
	p = (WORD *)(pdu+1);
	return ntohs(*p);
}

static inline WORD get_quantity( BYTE * pdu )
{
	WORD * p;
	p = (WORD *) (pdu+3);
	return ntohs(*p);
}
static inline void *get_data_pointer( BYTE * pdu )
{

	switch(pdu[0])
	{
		case WRITE_MUL_REG:
		case WRITE_MUL_COIL:
			return (pdu+6);
			break;
		case WRITE_SINGLE_COIL:
		case WRITE_SINGLE_REG:
			return (pdu+3);
		default:
			break;
	}
	return 0;
}

int sep_mb_header(BYTE * adu, BYTE * pdu, BYTE * header, int adu_len);

int mbTCP_send( int ,BYTE* , char*, long );
int mbRTUtun_send( int ,int, BYTE* , char*, long );
int mbRTU_send( int , int, BYTE* , char*, long );


int mbrecv(int sockid, BYTE*pdu, long timeout);
int debug_mbsend_eth( int ,BYTE* , char*, long );
int unpack_bit( BYTE*pack, BYTE*unpack, int n);
int pack_byte(BYTE*pack, BYTE*unpack, int n);
int pack_word(WORD*, long*, int);
int unpack_word( WORD*, long*, int );


#endif

