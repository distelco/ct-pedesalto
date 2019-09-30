/*
    ventilatore.h
    Contiene le dichiarazioni delle funzioni per la gestione del ventilatore
    e le definizioni necessarie
    Written By Franco Novi
    Distelco s.r.l.
    $Id: ventilatori.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/
#include "timer.h"

#if !defined(WORD)
#define WORD	short
#endif

#if !defined(BYTE)
#define BYTE	unsigned char
#endif

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

enum E_fan_mode {
	FAN_MODE_MANUAL,		//bypass dal quadro esterno
	FAN_MODE_SEMIAUTO,	//Comandato da consolle
	FAN_MODE_AUTO		//Comandato da logica interna
};

enum E_fan_cmd {
	FAN_CMD_OFF = 0,
	FAN_CMD_UP,
	FAN_CMD_DOWN
};

#define FAN_NUM	10



#define CMD_OFFSET			2
#define CMD_DOWN_BASE		CS(200) // sud
#define CMD_UP_BASE			CS(201) // nord

#define SA_CMD_OFFSET		2
#define SA_UPCMD_BASE		CS(300)	// nord
#define SA_DOWNCMD_BASE		CS(301) // sud

#define SA_STATE_BASE		CS(260)

#define MEM_OFFSET			4
#define MEM_VIBR_ALM_BASE	CS(220)	//
#define MEM_START_ALM_BASE	CS(221)	//
#define MEM_CC_ALM_BASE		CS(222)	//
#define MEM_SOR_ALM_BASE	CS(223)	//

#define ALM_OFFSET			2
#define ALM_RESET_BASE		CS(320)	//
#define ALM_DISABLE_BASE	CS(321)	//


#define IS_OFFSET			2
#define RMA_BASE			IS(152)	// ritorno marcia avanti (NORD)
#define RMI_BASE			IS(153)	// ritorno marcia indietro (SUD)



#define	Q_OFFSET			4
#define Q_ONOFF_BASE		IS(112)	// sezionatore linea
#define M_STATE_BASE		IS(113)	// selettore manuale quadro
#define S_ALARM_BASE		IS(114)	// starter alarm
#define CC_ALARM_BASE		IS(115)	// corto circuito

#define SOR_BASE		IS(230)	// sensore distacco

#define SVIB_BASE		IR(30)	// sensore vibrazione

#define A_CMD_BASE		HR(2100)
#define TOFF_COUNT_BASE		HR(2110)
#define FAN_STATE_BASE		HR(2120)
#define TRUN_H_COUNT_BASE 	HR(2130)
#define TRUN_M_COUNT_BASE 	HR(2140)
#define VIB_TH_BASE		HR(2150)

#define FAN_ON_TIME		60 //Secondi
#define FAN_OFF_TIME		60 //Secondi

/*
	Struttura contenente i dati di funzionamento dei ventilatori
*/
struct S_fan_struct {
	enum E_fan_mode mode;	// Modo operativo , Manuale, Automatico o SemiAutomatico
	enum E_fan_state state;	// stato ventilatore
	enum E_fan_cmd cmd;	// comando da inviare al ventilatore
	long toff_count;	// contatore tempo spegnimento
	long trun_h_count;	// contatore ore funzionamento
	long trun_s_count;	// contatori secondi di funzionamento
	long trun_m_count;	// contatori secondi di funzionamento
//	int  no_stop_delay;	//contatore per i minuti minimi di funzionamento e i minuti minimi di reavviamento
//	int  no_rerun_delay;	//contatore per i minuti minimi di funzionamento e i minuti minimi di reavviamento
	long start_time;	// tempo di avviamento
	
	BYTE starter_alarm;	//allarme avviatore
	BYTE cc_alarm;		//allarme corto circuito
	BYTE sor_alarm;		//allarme orizzontalita'
	BYTE vibr_alarm;	//allarme vibrazione
	BYTE alm_reset ;	//reset allarmi
	BYTE alm_ignore ;	//ignora allarmi

	//comandi da inviare ai ventilatori
	BYTE up_cmd;	//comando avanti
	BYTE down_cmd;	//comando indietro

	BYTE rma;	//ritorno marcia avanti
	BYTE rmi;	//ritorno marcia indietro

	BYTE starter ;	// ingresso allarme avviatore
	BYTE cc ;	// ingresso corto circuito
	BYTE sor;	//sensore orizzontalita'
	WORD svib;	//sensore vibrazione
	WORD vib_th;	//soglia vibrazione
        int  vib_cnt;   //contatore per permanenza allarme di vibrazione
};

void init_fan_data( void );
int fan_process(void);

