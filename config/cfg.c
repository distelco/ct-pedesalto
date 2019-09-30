/*
	cfg.c
	$Id: cfg.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $

*/

#undef DEBUG
#include <pthread.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#include "../config.h"
#include "common.h"
#include "devices.h"
#include "autovideo.h"
#include "mb_drv.h"
#include "ax10412.h"

#if 0
static char driver_name[256];
static char driver_file[256];
#endif

/****************************************************/
//Strutture Driver Descriptor dei vari dispositivi

extern struct S_driver_descriptor atv_driver_descriptor;
extern struct S_driver_descriptor ul_driver_descriptor;
extern struct S_driver_descriptor remote_io_driver_descriptor;

struct S_driver_descriptor *drv_types[] =
{
//	&atv_driver_descriptor,
//	&ul_driver_descriptor,
	&remote_io_driver_descriptor,
	NULL
} ;

/*
	Aree di memoria riservate
	Digital Input	0-7	//Input digitali ax10412
	Digital Output  0-7	//Output digitali ax10412
	Analog  Input	0-7 (0-0x0007)	//Input analogici ax10412
	Analog	Output	0-89 (0-0x0059) //Pannello messaggio variabile, semafori
*/
/******************************************************/
//Strutture dati dispositivi TCP

struct S_tcp atv1_tcp = {"192.168.1.154",15433,-1,1000} ; //autovideo

struct S_tcp ul1_tcp = {"192.168.1.201",1502,-1,1000} ;	//Unita' locali
struct S_tcp ul2_tcp = {"192.168.1.202",1502,-1,1000} ;	//Unita' locali
struct S_tcp ul3_tcp = {"192.168.1.203",1502,-1,1000} ;	//Unita' locali
struct S_tcp ul4_tcp = {"192.168.1.204",1502,-1,1000} ;	//Unita' locali

struct S_tcp rio1_tcp = {"192.168.1.103",502,-1,1000} ;	//IO Remoti QV (Quadro ventilazione)
struct S_tcp rio2_tcp = {"192.168.1.102",502,-1,1000} ;	//IO Remoti QI (Quadro illuminazione)
struct S_tcp rio3_tcp = {"192.168.1.101",502,-1,1000} ;	//IO Remoti QBT(Quadro bassa tensione)

/******************************************************/
// Strutture Driver Info per gli io_block

struct S_atv_data atv_1 = { (struct S_tcp *)&atv1_tcp, 0, "" } ;
struct S_atv_data atv_2 = { (struct S_tcp *)&atv1_tcp, 0, "" } ;
struct S_atv_data atv_3 = { (struct S_tcp *)&atv1_tcp, 0, "" } ;

//struct S_modbus_data ul1_di_1 = { &ul1_tcp,0,8}; 
//struct S_modbus_data ul1_do_1 = { &ul1_tcp,0,8};

struct S_modbus_data ul2_di_1 = { &ul2_tcp,0,1}; // IS-21 Sensore orizzontalita' ventilatore 1
struct S_modbus_data ul2_di_2 = { &ul2_tcp,1,1}; // IS-27 Sensore orizzontalita' ventilatore 2
struct S_modbus_data ul3_di_1 = { &ul3_tcp,0,1}; // IS-33 Sensore orizzontalita' ventilatore 1
struct S_modbus_data ul3_di_2 = { &ul3_tcp,1,1}; // IS-39 Sensore orizzontalita' ventilatore 2
struct S_modbus_data ul4_di_1 = { &ul4_tcp,0,1}; // IS-45 Sensore orizzontalita' ventilatore 1
struct S_modbus_data ul4_di_2 = { &ul4_tcp,1,1}; // IS-51 Sensore orizzontalita' ventilatore 2

struct S_modbus_data ul1_ai_1 = { &ul1_tcp,2,1}; // IR-16 Anemometro UL1
struct S_modbus_data ul2_ai_1 = { &ul2_tcp,0,2}; // IR-17,18 Sensore vibrazione 1 e 2 UL2
struct S_modbus_data ul3_ai_1 = { &ul3_tcp,0,2}; // IR-19,20 Sensore vibrazione 1 e 2 UL3
struct S_modbus_data ul4_ai_1 = { &ul4_tcp,0,2}; // IR-21,22 Sensore vibrazione 1 e 2 UL4

struct S_modbus_data ul1_ao_1 = { &ul1_tcp,0,4}; // Comandi semafori 1 - 4


struct S_modbus_data rio_di_1 = { &rio1_tcp,0x00,5};
struct S_modbus_data rio_di_2 = { &rio1_tcp,0x05,5};
struct S_modbus_data rio_di_3 = { &rio1_tcp,0x0a,5};
struct S_modbus_data rio_di_4 = { &rio1_tcp,0x0f,5};
struct S_modbus_data rio_di_5 = { &rio1_tcp,0x14,5};
struct S_modbus_data rio_di_6 = { &rio1_tcp,0x19,5};

//struct S_modbus_data rio_di_7 = { &rio2_tcp,0x00,5};


struct S_modbus_data rio_do_1 = { &rio1_tcp,0,12};

struct S_modbus_data rio_ai_1 = { &rio1_tcp,0,8};

/******************************************************/
// IO BLOCK

static struct S_driver_io_block di_blocks[] =
{
	{ 16,5, "", drv_remote_io_did , 0, &rio_di_1  },
	{ 22,5, "", drv_remote_io_did , 0, &rio_di_2  },
	{ 28,5, "", drv_remote_io_did , 0, &rio_di_3  },
	{ 34,5, "", drv_remote_io_did , 0, &rio_di_4  },
	{ 40,5, "", drv_remote_io_did , 0, &rio_di_5  },
	{ 46,5, "", drv_remote_io_did , 0, &rio_di_6  },	
	//
	{ 21, 1, "", drv_ul_did , 0, &ul2_di_1  },
	{ 27, 1, "", drv_ul_did , 0, &ul2_di_2  },
	{ 33, 1, "", drv_ul_did , 0, &ul3_di_1  },
	{ 39, 1, "", drv_ul_did , 0, &ul3_di_2  },
	{ 45, 1, "", drv_ul_did , 0, &ul4_di_1  },
	{ 51, 1, "", drv_ul_did , 0, &ul4_di_2  },
//	{ 52,5, "", drv_remote_io_did , 0, &rio_di_7  }

//	{ 08, 3, "", drv_atv_did, 0, &atv_1 },
//	{ 14, 3, "", drv_atv_did, 0, &atv_2 },
//	{ 17, 3, "", drv_atv_did, 0, &atv_3 },
};

static struct S_driver_io_block do_blocks[] =
{
	{ 16,12, "", drv_remote_io_did , 0, &rio_do_1  },
//	{ 0, 8, "", drv_ul_did, 0, &ul_do_1 },
};

static struct S_driver_io_block ai_block[]=
{
	{ 16, 1, "", drv_ul_did, 0, &ul1_ai_1 },
	{ 17, 2, "", drv_ul_did, 0, &ul2_ai_1 },
	{ 19, 2, "", drv_ul_did, 0, &ul3_ai_1 },
	{ 21, 2, "", drv_ul_did, 0, &ul4_ai_1 },
};

static struct S_driver_io_block ao_block[]=
{
	{ 100, 4, "", drv_ul_did, 0, &ul1_ao_1 },
};


/******************************************************/
// TABELLE IO

struct S_driver_io_block   	*di_table[] = 
{
	&di_blocks[0], // rio
	&di_blocks[1], // rio
	&di_blocks[2], // rio
	&di_blocks[3], // rio
	&di_blocks[4], // rio
	&di_blocks[5], // rio
	&di_blocks[6], // ul
	&di_blocks[7], // ul
	&di_blocks[8], // ul
	&di_blocks[9], // ul
	&di_blocks[10],// ul
	&di_blocks[11],// ul
	
	NULL	
};


struct S_driver_io_block   	*do_table[] = 
{ 
	&do_blocks[0],	//rio
//	&do_blocks[1],
	NULL 
};


struct S_driver_io_block   	*ai_table[] = 
{ 	
	&ai_block[0], 	//ul1
	&ai_block[1], 	//ul2
	&ai_block[2], 	//ul3
	&ai_block[3], 	//ul4
	NULL 
};



struct S_driver_io_block  	*ao_table[] = { 
	&ao_block[0],
	NULL 
};


#if 0

/////// NON USATA //////
static int parse_config(const char *filename)
{
	FILE *f;
	char line[1024];
	char cmd[64];
	const char *p;
	int errors, line_num;

	f = fopen(filename, "r");
	if (!f) {
		perror(filename);
		return -1;
	}

	errors = 0;
	line_num = 0;
	for(;;) {
		if (fgets(line, sizeof(line), f) == NULL)
			break ;
		line_num++ ;
		p = line ;
		while (isspace(*p))
			p++;
		if (*p == '\0' || *p == '#')
			continue;

		get_arg(cmd, sizeof(cmd), &p) ;

		if (!strcasecmp(cmd, "Port")) {
		}
		if (!strcasecmp(cmd, "NoDaemon")) {
		}
		
		if (!strcasecmp(cmd, "driver_type"))
		{
			SET_STRING( driver_name );
			SET_NUM( did );
			load_driver_did( driver_name, did );
		}
		
		if (!strcasecmp(cmd, "driver_block")) {
			SET_STRING( driver_name );
			SET_NUM( did );
			SET_STRING( driver_file );
			load_driver(driver_name, did, driver_file);
		}
#if 0

		if (!strcasecmp(cmd, "video_0_channel")) {
			SET_NUM( video_device[0].channel );
		}
		if (!strcasecmp(cmd, "video_0_fifo")) {
			SET_NUM( video_device[0].fifo[0]);
			SET_NUM( video_device[0].fifo[1]);
			SET_NUM( video_device[0].fifo[2]);
			SET_NUM( video_device[0].fifo[3]);
		}
		if (!strcasecmp(cmd, "video_1_name")) {
			SET_STRING( video_device[1].name );
		}
		if (!strcasecmp(cmd, "video_1_channel")) {
			SET_NUM( video_device[1].channel );
		}
		if (!strcasecmp(cmd, "video_1_fifo")) {
			SET_NUM( video_device[1].fifo[0]);
			SET_NUM( video_device[1].fifo[1]);
			SET_NUM( video_device[1].fifo[2]);
			SET_NUM( video_device[1].fifo[3]);		
		}
		if (!strcasecmp(cmd, "video_2_name")) {
			SET_STRING( video_device[2].name );
		}
		if (!strcasecmp(cmd, "video_2_channel")) {
			SET_NUM( video_device[2].channel );
		}
		if (!strcasecmp(cmd, "video_2_fifo")) {
			SET_NUM( video_device[2].fifo[0]);
			SET_NUM( video_device[2].fifo[1]);
			SET_NUM( video_device[2].fifo[2]);
			SET_NUM( video_device[2].fifo[3]);		
		}
		if (!strcasecmp(cmd, "video_3_name")) {
			SET_STRING( video_device[3].name );
		}
		if (!strcasecmp(cmd, "video_3_channel")) {
			SET_NUM( video_device[3].channel );
		}
		if (!strcasecmp(cmd, "video_3_fifo")) {
			SET_NUM( video_device[3].fifo[0]);
			SET_NUM( video_device[3].fifo[1]);
			SET_NUM( video_device[3].fifo[2]);
			SET_NUM( video_device[3].fifo[3]);		
		}
		if (!strcasecmp(cmd, "video_data_0_name")) {
			SET_STRING( video_data[0].name );
		}
		if (!strcasecmp(cmd, "video_data_0_fifonum")) {
			SET_NUM( video_data[0].fifonum );
		}
		if (!strcasecmp(cmd, "video_data_1_name")) {
			SET_STRING( video_data[1].name );
		}
		if (!strcasecmp(cmd, "video_data_1_fifonum")) {
			SET_NUM( video_data[1].fifonum );
		}
		if (!strcasecmp(cmd, "video_data_2_name")) {
			SET_STRING( video_data[2].name );
		}
		if (!strcasecmp(cmd, "video_data_2_fifonum")) {
			SET_NUM( video_data[2].fifonum );
		}
		if (!strcasecmp(cmd, "video_data_3_name")) {
			SET_STRING( video_data[3].name );
		}
		if (!strcasecmp(cmd, "video_data_3_fifonum")) {
			SET_NUM( video_data[3].fifonum );
		}
		if (!strcasecmp(cmd, "save_file")) {
			SET_STRING( savefile );
		}
		if (!strcasecmp(cmd, "save_default")) {
			SET_STRING( savedefault );
		}
#endif
	}
	fclose(f) ;
	if (errors)
		return -1 ;
	else
		return 0 ;
}

#endif

