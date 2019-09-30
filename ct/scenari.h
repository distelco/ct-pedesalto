/*

*/

#if !defined(__SCENARI_H__)
#define __SCENARI_H__
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "mymemory.h"
#include "timer.h"

#include "illumination.h"
#include "ventilazione.h"
//#include "traffico.h"

#if !defined BYTE
#define BYTE unsigned char
#endif

#if !defined WORD
#define WORD short
#endif

#define NUM_SCENARI	7

struct S_qg {
	BYTE on_off;
	BYTE scattato;
	BYTE apertura;
	BYTE chiusura;
};

#define SOS_NUM		5
#define QG_NUM		13

struct S_sos {
	BYTE soccorso; 	//(118)
	BYTE incendio;	//(115)
	BYTE polizia;	//(113)
	BYTE guasto;	//(veneto strade)
	BYTE estintore;	//
	BYTE merci_pericolose ;
	BYTE incidente ;
	char enable_soccorso ;
	char enable_incendio ;
	char enable_polizia ;
	char enable_guasto ;
	char enable_estintore ;
	char reset_alarm ;
};

#define SMOKE_ALM_NUM	8
struct S_smoke {
	BYTE alarm;
	char enable;
	char reset_alarm;
};

struct S_scenario_alarms {
	int priority ;
	char active ;
	char enable ;
	char scen_ventilazione ;
	char immediato_ventilazione ;
	char scen_illuminazione ;
	char immediato_illuminazione ;
	char scen_traffico ;
	char immediato_traffico ;
} ;

#define MAX_SCENARIO_ALARMS	20
#define ALM_SOS_SOCCORSO	0
#define ALM_SOS_INCENDIO	1
#define ALM_SOS_POLIZIA		2
#define ALM_SOS_GUASTO		3
#define ALM_SOS_ESTINTORE	4
#define ALM_MAX_CO		5
#define ALM_FUMO_1		6
#define ALM_FUMO_2		7
#define ALM_FUMO_3		8

struct S_scenari {
	struct 	S_sos sos[SOS_NUM];
	struct  S_smoke smoke[SMOKE_ALM_NUM];
	struct 	S_scenario_alarms alarms[MAX_SCENARIO_ALARMS] ;

	WORD	co_th_max;
	WORD	op_th_max;

	WORD	ill_scenario_proposto;
	WORD	vent_scenario_proposto;
	WORD	traffico_scenario_proposto;

	int 	current_priority ;	//priorita scenario attuale ( 0 = nessuno attivo )
	int	current_active ;	//allarme attivo attuale
	int	alarm_change ;
	unsigned int ritardo_att;	//tempo di attesa attivazione scenario proposto	
	
	WORD	ill_scenario;		//scenario illuminazione impostato
	int 	ill_confermato;		//flag per scenari confermati
	int 	ill_annullato;
	WORD	vent_scenario;		//scenario ventilazione impostato
	int 	vent_confermato;		//flag per scenari confermati
	int	vent_annullato;
	WORD	traffico_scenario;	//scenario traffico impostato
	int 	traff_confermato;		//flag per scenari confermati
	int	traff_annullato;
	
	int 	confermato;		//flag per scenari confermati
	
	WORD	co_level;
	WORD	op_level;
	
	int alarm_bit;		// 150
	int conferma_scenario;	// 151
	int annulla_scenario;	// 152
	
	int vent_conferma_scenario ;
	int ill_conferma_scenario ;
	int traff_conferma_scenario ;
	int vent_annulla_scenario ;
	int ill_annulla_scenario ;
	int traff_annulla_scenario ;
	
#define CARTELLI_TIMEOUT 600
	int cartello_incidente_timeout ;
	int cartello_merci_timeout ;
};

//Indirizzi IS
#define SMOKE_ALM_BASE	IS(0)


/* POTENZE */
#define QB_N_PA_S1	IR(100)
#define QB_N_PR_S1	IR(102)
#define QB_N_PA_S2	IR(108)
#define QB_N_PR_S2	IR(110)
#define QB_N_PA_S3	IR(116)
#define QB_N_PR_S3	IR(118)

#define QB_S_PA_S2	IR(124)
#define QB_S_PR_S2	IR(126)
#define QB_S_PA_S3	IR(132)
#define QB_S_PR_S3	IR(134)

#define CT_N_PA_S1	HR(2050)
#define CT_N_PR_S1	HR(2051)
#define CT_N_PA_S2	HR(2052)
#define CT_N_PR_S2	HR(2053)
#define CT_N_PA_S3	HR(2054)
#define CT_N_PR_S3	HR(2055)

#define CT_S_PA_S2	HR(2056)
#define CT_S_PR_S2	HR(2057)
#define CT_S_PA_S3	HR(2058)
#define CT_S_PR_S3	HR(2059)


/* CONSUMI */
#define QB_N_EA_S1	IR(104)
#define QB_N_ER_S1	IR(106)
#define QB_N_EA_S2	IR(112)
#define QB_N_ER_S2	IR(114)
#define QB_N_EA_S3	IR(120)
#define QB_N_ER_S3	IR(122)

#define QB_S_EA_S2	IR(128)
#define QB_S_ER_S2	IR(130)
#define QB_S_EA_S3	IR(136)
#define QB_S_ER_S3	IR(138)

#define CT_N_EA_S1	HR(2070)
#define CT_N_ER_S1	HR(2072)
#define CT_N_EA_S2	HR(2074)
#define CT_N_ER_S2	HR(2076)
#define CT_N_EA_S3	HR(2078)
#define CT_N_ER_S3	HR(2080)

#define CT_S_EA_S2	HR(2082)
#define CT_S_ER_S2	HR(2084)
#define CT_S_EA_S3	HR(2086)
#define CT_S_ER_S3	HR(2088)

#define SOS_SOCCORSO_BASE	IS(220)
#define SOS_INCENDIO_BASE	IS(221)
#define SOS_GUASTO_BASE		IS(222)
#define SOS_POLIZIA_BASE	IS(223)
#define SOS_MERCI_BASE		IS(224)
#define SOS_INCIDENTE_BASE	IS(225)
#define SOS_ESTINTORE_BASE	IS(226)
#define SOS_PORTA_BASE		IS(227)
#define SOS_OFFSET	8

//Indirizzi CS
#define QBT_RESET_KWH           CS(70)

#define SOS_C_SOCCORSO_BASE	CS(400)
#define SOS_C_INCENDIO_BASE	CS(401)
#define SOS_C_GUASTO_BASE	CS(402)
#define SOS_C_POLIZIA_BASE	CS(403)
#define SOS_C_ESTINTORE_BASE	CS(404)

#define S_PROPOSTA_SCENARIO	CS(150)
#define S_CONFERMA_SCENARIO	CS(151)
#define S_ANNULLA_SCENARIO	CS(152)

#define S_VENT_CONF_SCEN	CS(153)
#define S_VENT_ANN_SCEN		CS(154)
#define S_ILL_CONF_SCEN		CS(155)
#define S_ILL_ANN_SCEN		CS(156)
#define S_CTR_CONF_SCEN		CS(157)
#define S_CTR_ANN_SCEN		CS(158)

#define S_VENT_CONFERMATO	CS(159)
#define S_ILL_CONFERMATO	CS(160)
#define S_CTR_CONFERMATO	CS(161)

#define CARTELLO_INCIDENTE	CS(181)
#define CARTELLO_MERCI		CS(182)

#define SOS_M_SOCCORSO_BASE	CS(410)
#define SOS_M_INCENDIO_BASE	CS(411)
#define SOS_M_GUASTO_BASE	CS(412)
#define SOS_M_POLIZIA_BASE	CS(413)
#define SOS_M_MERCI_BASE	CS(414)
#define SOS_M_INCIDENTE_BASE	CS(415)
#define SOS_M_ESTINTORE_BASE	CS(416)
#define SOS_M_OFFSET	7

#define SMOKE_M_ALM_BASE	CS(450)


#define SOS_EN_SOCCORSO_BASE	CS(2500)
#define SOS_EN_INCENDIO_BASE	CS(2501)
#define SOS_EN_POLIZIA_BASE	CS(2502)
#define SOS_EN_GUASTO_BASE	CS(2503)
#define SOS_EN_ESTINTORE_BASE	CS(2504)
#define SOS_EN_OFFSET	5

#define SCEN_ALM_ACTIVE_BASE	CS(2550)
#define SCEN_ALM_ENABLE_BASE	CS(2551)
#define SCEN_ALM_CS_OFFSET	2

#define SMOKE_EN_ALM_BASE	CS(2600)

#define SCEN_ALM_IMM_VENT_BASE	CS(2650)
#define SCEN_ALM_IMM_ILL_BASE	CS(2651)
#define SCEN_ALM_IMM_CTR_BASE	CS(2652)
#define SCEN_ALM_IMM_OFFSET	3


//Indirizzi IR
#define CT_CO1_LEVEL		IR(40)
#define CT_OP1_LEVEL		IR(41)
#define CT_CO2_LEVEL		IR(42)
#define CT_OP2_LEVEL		IR(43)


//Indirizzi HR
#define KWH_N_S1		HR(2050)
#define KVA_N_S1		HR(2052)
#define KWH_N_S2		HR(2054)
#define KVA_N_S2  		HR(2056)
#define KWH_N_S3		HR(2058)
#define KVA_N_S3		HR(2060)
#define KWH_S_S2		HR(2062)
#define KVA_S_S2  		HR(2064)
#define KWH_S_S3		HR(2066)
#define KVA_S_S3		HR(2068)

#define CO_MAX_TH		HR(2450)
#define OP_MAX_TH		HR(2451)

#define ILL_SCEN_PROP		HR(2652)
#define VENT_SCEN_PROP		HR(2653)
#define TRAF_SCEN_PROP		HR(2654)
#define RITARDO_ATT_SCEN	HR(2655)
#define S_CURRENT_SCENE_ACT	HR(2657)
#define S_CURRENT_SCENE_PRIO	HR(2658)

#define CTR_CL1_ATT_SCENARIO	HR(41)
#define CTR_CL2_ATT_SCENARIO	HR(42)

#define CTR_ATT_SCENARIO	HR(2660)
#define ILL_ATT_SCENARIO	HR(2661)
#define VENT_ATT_SCENARIO	HR(2662)

#define CTR_MAN_SCENARIO	HR(2670)
#define ILL_MAN_SCENARIO	HR(2671)
#define VENT_MAN_SCENARIO	HR(2672)

#define SCEN_ALM_PRIO_BASE	HR(3000)
#define SCEN_ALM_VENT_BASE	HR(3001)
#define SCEN_ALM_ILL_BASE	HR(3002)
#define SCEN_ALM_CTR_BASE	HR(3003)
#define SCEN_ALM_HR_OFFSET	4


int gestione_scenari(void);
int gestione_servizio(void);
void init_gestione_scenari(void);


#endif



