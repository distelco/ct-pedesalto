/*
	cfg.c
	$Id: ct_cfg.c,v 1.3 2009-01-29 10:32:55 matteoc Exp $

*/

#undef DEBUG
#include "../config.h"

#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#include "mymemory.h"
#include "devices.h"
#include "autovideo.h"
#include "mb_drv.h"

/****************************************************/
//Strutture Driver Descriptor dei vari dispositivi

extern struct S_driver_descriptor mbrtutun_io_driver_descriptor;
extern struct S_driver_descriptor remote_io_driver_descriptor;

struct S_driver_descriptor *drv_types[] =
{
	&remote_io_driver_descriptor,
	&mbrtutun_io_driver_descriptor,
	NULL
} ;

/******************************************************************
	Aree di memoria riservate
	Digital Input	0-7	//Input digitali ax10412
	Digital Output  0-7	//Output digitali ax10412
	Analog  Input	0-7 (0-0x0007)	//Input analogici ax10412
	Analog	Output	0-89 (0-0x0059) //Pannello messaggio variabile, semafori
*/

/******************************************************/
//  Descrittori connessioni TCP
//
struct S_tcp QGBT_tcp = {"192.168.11.10",502,-1,E_tcp_disconnected,1000} ;	//IO Remoti Quadro NORD (Quadro principale)
struct S_tcp QMT_tcp  = {"192.168.11.11",502,-1,E_tcp_disconnected,1000} ;	//IO Remoti Quadro NORD (Quadro principale)
struct S_tcp QV_tcp   = {"192.168.11.12",502,-1,E_tcp_disconnected,1000} ;	//IO Remoti Quadro NORD (Quadro principale)
struct S_tcp QBT_tcp  = {"192.168.11.13",502,-1,E_tcp_disconnected,1000} ;	//IO Remoti Quadro NORD (Quadro principale)

struct S_tcp sosul1_tcp = {"192.168.11.101",502,-1,E_tcp_disconnected,1000} ;	// SOS UL 1
struct S_tcp sosul2_tcp = {"192.168.11.102",502,-1,E_tcp_disconnected,1000} ;	// SOS UL 2
struct S_tcp sosul3_tcp = {"192.168.11.103",502,-1,E_tcp_disconnected,1000} ;	// SOS UL 3
struct S_tcp sosul4_tcp = {"192.168.11.104",502,-1,E_tcp_disconnected,1000} ;	// SOS UL 4

struct S_tcp QVC_N_tcp = {"192.168.11.105",502,-1,E_tcp_disconnected,1000} ;	//IO Remoti Quadro NORD (Quadro principale)
struct S_tcp QVC_S_tcp = {"192.168.11.106",502,-1,E_tcp_disconnected,1000} ;	//IO Remoti Quadro SUD(Quadro principale)


#define IOSTATUS_BASE HR(4000)
void update_device_iostatus(void)
{
        //write_data( IOSTATUS_BASE+0, ul1_tcp.state );
        //write_data( IOSTATUS_BASE+1, ul8_tcp.state );
        //write_data( IOSTATUS_BASE+2, ul12_tcp.state );

        write_data( IOSTATUS_BASE+3, QV_tcp.state );
        write_data( IOSTATUS_BASE+4, QBT_tcp.state );

        write_data( IOSTATUS_BASE+5, sosul1_tcp.state );
        write_data( IOSTATUS_BASE+6, sosul2_tcp.state );
        write_data( IOSTATUS_BASE+7, sosul3_tcp.state );
        write_data( IOSTATUS_BASE+8, sosul4_tcp.state );

        write_data( IOSTATUS_BASE+11, QVC_N_tcp.state );
        write_data( IOSTATUS_BASE+12, QVC_S_tcp.state );

        return ;
}

/******************************************************/
// Strutture Driver Info per gli io_block
//
//	{ < pointer to connection>, <start address of data>, <number of elements> };

// SOS-ul blocks
struct S_modbus_data sosul1_di_1 = { &sosul1_tcp,1,Input,0,3,0,0};	// pulsanti
struct S_modbus_data sosul1_di_2 = { &sosul1_tcp,1,Input,3,2,0,0};	// distacco
struct S_modbus_data sosul1_ai_1 = { &sosul1_tcp,1,Analog,0,2,100,10};	// vibrazione
struct S_modbus_data sosul1_ai_2 = { &sosul1_tcp,1,Analog,2,2,100,40};	// anemometro


struct S_modbus_data sosul2_di_1 = { &sosul2_tcp,1,Input,0,3,0,0};		// pulsanti
struct S_modbus_data sosul2_di_2 = { &sosul2_tcp,1,Input,3,6,0,0};		// distacco
struct S_modbus_data sosul2_ai_1 = { &sosul2_tcp,1,Analog,0,6,100,20};	// vibrazione
struct S_modbus_data sosul2_ai_2 = { &sosul2_tcp,1,Analog,6,2,100,30};	// OP CO


struct S_modbus_data sosul3_di_1 = { &sosul3_tcp,1,Input,0,3,0,0};		// pulsanti
struct S_modbus_data sosul3_di_2 = { &sosul3_tcp,1,Input,3,2,0,0};		// distacco
struct S_modbus_data sosul3_ai_1 = { &sosul3_tcp,1,Analog,0,2,100,20};	// vibrazione
struct S_modbus_data sosul3_ai_2 = { &sosul3_tcp,1,Analog,3,2,100,30};	// OP CO


struct S_modbus_data sosul4_di_1 = { &sosul4_tcp,1,Input,0,3,0,0};		// pulsanti
struct S_modbus_data sosul4_ai_2 = { &sosul4_tcp,1,Analog,0,2,100,60};	// anemometro


// QBT blocks
//struct S_modbus_data QBT_di_1 =  { &QNORD_tcp,1,Input,2,5,500,10};
//struct S_modbus_data QBT_di_2 =  { &QNORD_tcp,1,Input,7,43,500,20};
//struct S_modbus_data QBT_ai_1 = { &QBT_tcp,1,Analog,0,6,0,0};

// QV blocks
struct S_modbus_data QV_di_1 = { &QV_tcp,1,Input,0,42,0,0};		// ritorni comandi ventilatori
struct S_modbus_data QV_di_2 = { &QV_tcp,1,Input,42,20,0,0};		// CC ventilatori
struct S_modbus_data QV_do_1 = { &QV_tcp,1,Coil,0,20,10,10};		// comandi ventilatori


/******************************************************/
// IO BLOCK

struct S_driver_io_block di_table[] =
{

	//{ 32,5, "", drv_remote_io_did , 0, &QBT_di_1  },
	//{ 38,45, "", drv_remote_io_did , 0, &QBT_di_2  },
	//{ 100,53, "", drv_remote_io_did , 0, &QIN_di_1  },
	//{ 160,46, "", drv_remote_io_did , 0, &QIS_di_1  },
	//{ 220,8, "", drv_remote_io_did , 0, &sosul1_di_1  },
	//{ 228,8, "", drv_remote_io_did , 0, &sosul2_di_1  },
	//{ 236,8, "", drv_remote_io_did , 0, &sosul3_di_1  },
	//{ 244,8, "", drv_remote_io_did , 0, &sosul4_di_1  },
	//{ 252,8, "", drv_remote_io_did , 0, &sosul5_di_1  },
	{ 110,42, "", drv_remote_io_did , 0, &QV_di_1  },
	{ 152,20, "", drv_remote_io_did , 0, &QV_di_2  },
	{ 210,3, "", drv_remote_io_did , 0, &sosul1_di_1  },
	{ 213,3, "", drv_remote_io_did , 0, &sosul2_di_1  },
	{ 216,3, "", drv_remote_io_did , 0, &sosul2_di_1  },
	{ 219,3, "", drv_remote_io_did , 0, &sosul4_di_1  },

	{ 230,2, "", drv_remote_io_did , 0, &sosul1_di_2  },
	{ 232,6, "", drv_remote_io_did , 0, &sosul2_di_2  },
	{ 238,2, "", drv_remote_io_did , 0, &sosul3_di_2  },
	{ 0,  0, "", 0, 0, NULL }
} ;

struct S_transform_io_block di_transform[] =
{
	{ 32, TRANSFORM_INV },
	{ 0, NULL }
} ;

struct S_driver_io_block do_table[] =
{
//	{ 180, 3, "", drv_remote_io_did , 0, &QIN_do_1  },
//	{ 200, 20, "", drv_remote_io_did , 0, &QV_do_1  },
	{ 0,  0, "", 0, 0, NULL }
};

struct S_driver_io_block ai_table[]=
{
	{ 30, 2, "", drv_remote_io_did, 0, &sosul1_ai_1 },
	{ 32, 6, "", drv_remote_io_did, 0, &sosul2_ai_1 },
	{ 38, 2, "", drv_remote_io_did, 0, &sosul3_ai_1 },

	{ 40, 2, "", drv_remote_io_did, 0, &sosul2_ai_2 },
	{ 42, 2, "", drv_remote_io_did, 0, &sosul3_ai_2 },
	
	{ 52, 1, "", drv_remote_io_did, 0, &sosul1_ai_2 },
	{ 54, 1, "", drv_remote_io_did, 0, &sosul4_ai_2 },
	
	{ 0,  0, "", 0, 0, NULL }
};

struct S_transform_io_block ai_transform[] =
{
	{ 40, TRANSFORM_4_20 },
	{ 41, TRANSFORM_4_20 },
	{ 42, TRANSFORM_4_20 },
	{ 43, TRANSFORM_4_20 },
	{ 52, TRANSFORM_4_20 },
	{ 54, TRANSFORM_4_20 },
	{ 0, NULL }
} ;

struct S_driver_io_block ao_table[]=
{
	{ 0,  0, "", 0, 0, NULL }
};

struct S_driver_io_block mem_table[]=
{
	{ 0,  0, "", 0, 0, NULL }
};


