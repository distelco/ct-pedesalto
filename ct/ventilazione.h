/*
    ventilatore.h
    Contiene le dichiarazioni delle funzioni per la gestione del ventilatore
    e le definizioni necessarie
    Written By Franco Novi
    Written by Matteo Costa
    Distelco s.r.l.
    $Id: ventilazione.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/
#include "timer.h"

#if !defined(WORD)
#define WORD	short
#endif

#if !defined(BYTE)
#define BYTE	unsigned char
#endif


#define FAN_NUM	10

// CS

#define VIBR_ALARM_BASE	CS(220)
#define S_ALARM_BASE	CS(221)
#define CC_ALARM_BASE	CS(222)
#define SOR_ALARM_BASE	CS(223)
#define ALARM_IN_OFFSET	4

// CS
#define ALM_RESET_BASE		CS(320)
#define ALM_DISABLE_BASE	CS(321)
#define ALM_OUT_OFFSET		2

// IR

#define V_OP1_LEVEL	IR(40)
#define V_CO1_LEVEL	IR(41)
#define V_OP2_LEVEL	IR(42)
#define V_CO2_LEVEL	IR(43)

#define V_AN1_LEVEL	IR(52)
#define V_AN1_DIR	IS(400)
#define V_AN2_LEVEL	IR(54)
#define V_AN2_DIR	IS(401)

// HR

#define FAN_STATE_BASE		HR(2120)
#define TRUN_H_COUNT_BASE 	HR(2130)

#define FAN_CMD_BASE		HR(2100)

#define A_OP1_LEVEL	HR(76)
#define A_CO1_LEVEL	HR(77)
#define A_OP2_LEVEL	HR(78)
#define A_CO2_LEVEL	HR(79)

// il valore corretto assume valori positivi o negativi
// l'offset del segnale 4-20 viene rimosso
#define A_AN1_LEVEL	HR(80)
#define A_AN2_LEVEL	HR(81)


#define V_SCENE_ACTIVE_BASE	HR(2662)

#define V_SINGLE_DOUBLE		HR(2160)
#define VENT_MODO_OPERATIVO	HR(2161)

#define CO_TH_BASE		HR(2162)
#define OP_TH_BASE		HR(2166)

#define VENT_LEVEL		HR(2170)
#define FAN_NUM_BASE		HR(2171)

#define DIR_BASE		HR(2176)
#define NUM_ACTIVE		HR(2177)
#define SOGLIA_ANEMOMETRO	HR(2178)
#define V_SCENE_BASE		HR(2179)
#define V_SCENE_OFFSET		FAN_NUM
#define V_SCENE_NUM		7
#define V_SCENE_AUTO		0
#define V_SCENE_SLOW_UP		8
#define V_SCENE_FAST_UP		9
#define V_SCENE_SLOW_DN		10
#define V_SCENE_FAST_DN		11
#define V_SCENE_STOP_AL		12

/*
	Modo operativo
*/
enum E_vent_modo_operativo {
	MODO_DISTANZA = 0,
	MODO_lOCALE,
	MODO_MANUTENZIONE
};
/*
	Stato ventilatore
*/
enum E_fan_state {
	FAN_STATE_OFF,	//Spento
	FAN_STATE_STOP,	//Spegnimento
	FAN_STATE_START_UP,	//Avviamento avanti
	FAN_STATE_START_DOWN,	//Avviamento indietro
	FAN_STATE_RUN_UP,	//In moto avanti
	FAN_STATE_RUN_DOWN,	//In moto indietro
	FAN_STATE_BAD,	//Avaria
	FAN_STATE_MANUAL,	//Funz. manuale
	FAN_STATE_SEMIAUTO	//Funz. Semiautomatico
};

/*
	Tipo di gestione ventilatori:
	Singolo, a coppia
*/
enum E_fan_man {
	FAN_SINGLE = 0,
	FAN_DUAL
};

/*
	Direzione Avanti Indietro Ventilatore
*/
enum E_fan_cmd {
	FAN_CMD_OFF = 0,
	FAN_CMD_UP,
	FAN_CMD_DOWN
};

enum E_fan_mode {
	FAN_MODE_MANUAL,		//bypass dal quadro esterno
	FAN_MODE_SEMIAUTO,	//Comandato da consolle
	FAN_MODE_AUTO		//Comandato da logica interna
};

#define TMR_VENT_DN_CHANGE	900	// tempo in secondi prima di diminuzione di livello della ventilazione
#define TMR_VENT_UP_CHANGE	90	// tempo in secondi prima di aumento di livello della ventilazione
#define TMR_CHECK_INTERVAL	(120*60)
#define TMR_CHECK_ACTIVE	120

/*
	Struttura contenente le variabili per la regolazione del processo di ventilazione
*/
struct S_vent_reg {
	enum E_fan_man gestione;	//tipo di gestione dei ventilatori, singoli o a coppie
	WORD vent_level;	// livello di ventilazione
	WORD direction;		// direzione prefenziale
	WORD set_direction ;	// direzione impostata
	WORD num_attivi ;
        WORD modo_operativo ;
	WORD soglia_anemometro ;	// soglia anemometro per determinazione direzione

	WORD co_level_1;	//livello CO
	WORD op_level_1;	//livello opacimetro
	WORD co_level_2;	//livello CO
	WORD op_level_2;	//livello opacimetro
	WORD co_level_1_in[20];
	WORD op_level_1_in[20];
	WORD co_level_2_in[20];
	WORD op_level_2_in[20];
	
	long an_level_1 ;
	long an_level_2 ;

	long abs_an_level_1 ;
	long abs_an_level_2 ;
	
	WORD co_lev1_th;	//soglia di passaggio da livello 0 a 1 co
	WORD co_lev2_th;	//soglia di passaggio da livello 1 a 2 co
	WORD co_lev3_th;	//soglia di passaggio da livello 2 a 3 co
	WORD co_lev4_th;	//soglia di passaggio da livello 3 a 4 co
	
	WORD op_lev1_th;	//soglia di passaggio da livello 0 a 1 opacimentro
	WORD op_lev2_th;	//soglia di passaggio da livello 1 a 2 opacimentro
	WORD op_lev3_th;	//soglia di passaggio da livello 2 a 3 opacimentro
	WORD op_lev4_th;	//soglia di passaggio da livello 3 a 4 opacimentro

	WORD change_timer ;
	WORD fan_lev_num[5];	// numero ventilatori accesi per ogni livello di ventilazione

	WORD check_timer;	// timer per test direzione ventilazione
	int  check_flag ;	// flag per test direzione ventilazione in corso
	
	WORD active_scene;	// scenario attivo
	WORD scene[V_SCENE_NUM][FAN_NUM];	// impostazione scenari
};


/*
	Struttura contenente i dati di funzionamento dei singoli ventilatori
*/
struct S_fan_struct {
	enum E_fan_state state;	// stato ventilatore
	enum E_fan_cmd cmd;	// comando da inviare al ventilatore
	
	BYTE starter_alarm;	//allarme avviatore
	BYTE cc_alarm;		//allarme corto circuito
	BYTE sor_alarm;		//allarme orizzontalita'
	BYTE vibr_alarm;	//allarme vibrazione
	BYTE alm_reset ;	//reset allarmi
	BYTE alm_ignore ;	//ignora allarmi

	BYTE rma;	//ritorno marcia avanti
	BYTE rmi;	//ritorno marcia indietro
	
	long trun_h_count ;
};


struct S_fan_dual {
	int dx;
	int sx;
};


int ventilazione_process(void);
void init_ventilazione_data( void );

