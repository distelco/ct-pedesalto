/*
	illumination.h
	Contiene le definizioni di illumination.c
	Written By Franco Novi
	Distelco s.r.l.
	$Id: illumination.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/


#if !defined(__ILLUMINATION_H__)

#define __ILLUMINATION_H__

#if !defined BYTE
#define BYTE unsigned char
#endif

#if !defined WORD
#define WORD short
#endif

#define ILL_SCENE_NUM	7
#define ILL_SCENE_AUTO	0
#define ILL_TELERUTTORI 19

#define REGULATOR_NUM	3
#define SENSOR_NUM	3


struct S_regulator {
	BYTE int_generale; 	//comando interruttore generale scattato
	BYTE bypass_cmd;	//contatto comando bypass
	BYTE luce_max_in;	//contatto segnalazione luce massima
	BYTE luce_rid_in;	//contatto segnalazione luce ridotta
	BYTE luce_max_cmd;	//comando forzatura luce massima
	long luce_esterna_out;	//sensore luce esterna	
	long luce_esterna_in;	//sensore luce esterna
	long luce_esterna_data[10];	//sensore luce esterna
	long opacimetro_in;	//sensore opacita'
	long livello_potenza;
	int soglia_1 ;
        int soglia_2 ;
	int soglia_3 ;
        int soglia_4 ;
        int rinf_lev ;	// livello rinforzi attivi
};

struct S_ill_scene
{
    // dati per scenario
    WORD livello_luce[REGULATOR_NUM] ;
    char teleruttori[ILL_TELERUTTORI] ;
};

#define OP_NUM_LEVEL	10
struct S_illumination {
    char teleruttori[ILL_TELERUTTORI] ;	// comando teleruttori
    char semiauto[ILL_TELERUTTORI] ;	// selezione manutenzione teleruttori
    char semiauto_chiusura[ILL_TELERUTTORI] ;	// comando manutenzione teleruttori
    int trun_m_count[ILL_TELERUTTORI];
    int trun_h_count[ILL_TELERUTTORI];
    struct S_ill_scene scenario[ILL_SCENE_NUM];	//imposta l'illuminazione per lo scenario
    WORD modo_operativo ;
    WORD active_scene ;
};




///Indirizzi IS
#define ILL_GENERALE_BASE	IS(80)
#define ILL_GENERALE_OFFSET	4

#define ILL_BYPASS_BASE		IS(81)
#define ILL_BYPASS_OFFSET	ILL_GENERALE_OFFSET

#define ILL_LUCE_MAX_IN_BASE	IS(82)
#define ILL_LUCE_MAX_IN_OFFSET	ILL_GENERALE_OFFSET

#define ILL_LUCE_RID_IN_BASE	IS(83)
#define ILL_LUCE_RID_IN_OFFSET	ILL_GENERALE_OFFSET

//Indirizzi CS
#define ILL_LUCE_MAX_CMD_BASE	CS(32)
#define ILL_LUCE_MAX_CMD_OFFSET	1

#define ILL_TELERUTTORI_BASE	CS(35)
#define ILL_R01	2
#define ILL_R02	3
#define ILL_R03	4
#define ILL_R04	15
#define ILL_R05	16
#define ILL_R06	17

#define ILL_SEMIAUTO_CHIUSURA	CS(220)

//Indirizzi IR
#define AN_LUCE_EST_IN_BASE_I	IR(32)
#define ILL_LIV_POT_BASE	IR(35)

//Indirizzi HR
#define ILL_LUCE_OUT_BASE	HR(32)
#define AN_LUCE_EST_IN_BASE	HR(70)

#define ILL_MODO_OPERATIVO	HR(2100)

enum E_ill_modo_operativo {
	ILL_MODO_DISTANZA,	
	ILL_MODO_LOCALE,	
	ILL_MODO_MANUTENZIONE	
};

#define ILL_SOGLIA_4		HR(2101)
#define ILL_SOGLIA_3		HR(2102)
#define ILL_SOGLIA_2		HR(2103)
#define ILL_SOGLIA_1		HR(2104)
#define ILL_MINUTI_BASE		HR(2105)
#define ILL_ORE_BASE		HR(2123)
#define ILL_SCENE_ACTIVE	HR(2161)
#define ILL_SCENE_DATA_BASE	HR(2600)
#define ILL_SCENE_DATA_STEP	21
#endif


int illuminazione_process(void);
void init_illuminazione_data(void);

