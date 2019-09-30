/*
    air_process.c
    Processo gestione ventilazione
    Written By Franco Novi
    Distelco s.r.l.
    $Id: ventilazione.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/
//#define DEBUG

#undef DEBUG

#include <stdio.h>
#include <string.h>


#include "common.h"
#include "mymemory.h"
#include "ventilazione.h"
#include "timer.h"

static struct S_fan_struct fan[FAN_NUM];

static struct S_vent_reg vent_reg;

static inline int
read_vent_data (struct S_vent_reg *reg, struct S_fan_struct *pfans)
{
	int i, j;

	for (i = 0; i < FAN_NUM; i++)
	{
		pfans[i].starter_alarm = read_data ((S_ALARM_BASE + (i * ALARM_IN_OFFSET)));
		pfans[i].cc_alarm = read_data ((CC_ALARM_BASE + (i * ALARM_IN_OFFSET)));
		pfans[i].vibr_alarm = read_data ((VIBR_ALARM_BASE + (i * ALARM_IN_OFFSET)));
		pfans[i].sor_alarm = read_data ((SOR_ALARM_BASE + (i * ALARM_IN_OFFSET)));
		pfans[i].state = read_data (FAN_STATE_BASE + i);
		pfans[i].rma = 0;
		pfans[i].rmi = 0;

		pfans[i].trun_h_count = read_data (TRUN_H_COUNT_BASE + i);

	}
	/*
	   La direzione preferenziale viene utilizzata solo in modo automatico
	 */
	reg->gestione = read_data (V_SINGLE_DOUBLE);
	reg->set_direction = read_data (DIR_BASE);	// direzione prefenziale

	// shift data in media buffer
	if (read_data (PULSE_1SEC_ADDR) > 0)
	{
	    memmove( &(reg->co_level_1_in[1]), &(reg->co_level_1_in[0]),  19*sizeof(WORD)) ;
    	    memmove( &(reg->co_level_2_in[1]), &(reg->co_level_2_in[0]),  19*sizeof(WORD)) ;
	    reg->co_level_1_in[0] = scale4_20(read_data (V_CO1_LEVEL));	//soglia di passaggio da livello 0 a 1 co2
	    if (reg->co_level_1_in[0] > 32000)
		reg->co_level_1_in[0] = 0;
	    for ( j=0,i=0;i<20;i++)
		j += reg->co_level_1_in[i] ;
	    reg->co_level_1 = scale4_20(j / 20) ;
		
	    reg->co_level_2_in[0] = scale4_20(read_data (V_CO2_LEVEL));	//soglia di passaggio da livello 0 a 1 co2
	    if (reg->co_level_2_in[0] > 32000)
		reg->co_level_2_in[0] = 0;
    	    for ( j=0,i=0;i<20;i++)
		j += reg->co_level_2_in[i] ;
    	    reg->co_level_2 = scale4_20(j / 20) ;

	}

	reg->co_lev1_th = read_data (CO_TH_BASE);	//soglia di passaggio da livello 0 a 1 co2
	reg->co_lev2_th = read_data (CO_TH_BASE + 1);	//soglia di passaggio da livello 1 a 2 co2
	reg->co_lev3_th = read_data (CO_TH_BASE + 2);	//soglia di passaggio da livello 2 a 3 co2
	reg->co_lev4_th = read_data (CO_TH_BASE + 3);	//soglia di passaggio da livello 3 a 4 co2


	reg->soglia_anemometro = read_data(SOGLIA_ANEMOMETRO);
	
	// shift data in media buffer (size is 20 elements)
	if (read_data (PULSE_1SEC_ADDR) > 0)
	{
	    memmove( &(reg->op_level_1_in[1]), &(reg->op_level_1_in[0]),  19*sizeof(WORD)) ;
	    memmove( &(reg->op_level_2_in[1]), &(reg->op_level_2_in[0]),  19*sizeof(WORD)) ;
	    reg->op_level_1_in[0] = scale4_20(read_data (V_OP1_LEVEL));	//soglia di passaggio da livello 0 a 1 co2      
	    if (reg->op_level_1_in[0] > 32000)
		reg->op_level_1_in[0] = 0;
	    for ( j=0,i=0;i<20;i++)
		j += reg->op_level_1_in[i] ;
	    reg->op_level_1 = scale4_20(j / 20) ;
	
	    reg->op_level_2_in[0] = scale4_20(read_data (V_OP2_LEVEL));	//soglia di passaggio da livello 0 a 1 co2      
	    if (reg->op_level_2_in[0] > 32000)
		reg->op_level_1_in[0] = 0;
	    for ( j=0,i=0;i<20;i++)
		j += reg->op_level_2_in[i] ;
	    reg->op_level_2 = scale4_20(j / 20) ;
	}
	// calcolo per anemometro
	if ( read_data(V_AN1_DIR) == 1 )
	{
	    reg->an_level_1 = (scale4_20(read_data(V_AN1_LEVEL))-6550) ;
	    reg->abs_an_level_1 = (scale4_20(read_data(V_AN1_LEVEL))-6550) ;
	}
	else
	{
	    reg->an_level_1 = -(scale4_20(read_data(V_AN1_LEVEL))-6550) ;
	    reg->abs_an_level_1 = (scale4_20(read_data(V_AN1_LEVEL))-6550) ;
	}
	

	if ( read_data(V_AN2_DIR) == 1 )
	{
	    reg->an_level_2 = (scale4_20(read_data(V_AN2_LEVEL))-6550) ;
	    reg->abs_an_level_2 = (scale4_20(read_data(V_AN2_LEVEL))-6550) ;
	}
	else
	{
	    reg->an_level_2 = -(scale4_20(read_data(V_AN2_LEVEL))-6550) ;
	    reg->abs_an_level_2 = (scale4_20(read_data(V_AN2_LEVEL))-6550) ;
	}

	reg->op_lev1_th = read_data (OP_TH_BASE);	//soglia di passaggio da livello 0 a 1 opacimentro
	reg->op_lev2_th = read_data (OP_TH_BASE + 1);	//soglia di passaggio da livello 0 a 1 opacimentro
	reg->op_lev3_th = read_data (OP_TH_BASE + 2);	//soglia di passaggio da livello 0 a 1 opacimentro
	reg->op_lev4_th = read_data (OP_TH_BASE + 3);	//soglia di passaggio da livello 0 a 1 opacimentro
	for (i = 0; i <= 4; i++)
	{
		(reg->fan_lev_num)[i] = read_data (FAN_NUM_BASE + i);	// numero ventilatori a livello iesimo  
	}

	//Importazione dati scenari
	reg->active_scene = read_data (V_SCENE_ACTIVE_BASE);

	for (i = 0; i < V_SCENE_NUM; i++)
	{
		for (j = 0; j < FAN_NUM; j++)
			reg->scene[i][j] = read_data ((V_SCENE_BASE + (V_SCENE_OFFSET * i) + j));
	}

	return 0;
}

static inline int
write_vent_data (struct S_vent_reg *reg, struct S_fan_struct *pfans)
{
	int i;
	for (i = 0; i < FAN_NUM; i++)
	{
		write_data ((FAN_CMD_BASE + i), pfans[i].cmd);
		//write_data ((ALM_RESET_BASE + (i * ALM_OUT_OFFSET)), pfans[i].alm_reset);
		//write_data ((ALM_DISABLE_BASE + (i * ALM_OUT_OFFSET)), pfans[i].alm_ignore);
	}
	write_data (A_CO1_LEVEL, reg->co_level_1 );
	write_data (A_OP1_LEVEL, reg->op_level_1 );
	write_data (A_CO2_LEVEL, reg->co_level_2 );
	write_data (A_OP2_LEVEL, reg->op_level_2 );
	write_data (VENT_LEVEL, reg->vent_level);
	write_data (NUM_ACTIVE, reg->num_attivi);
	write_data (A_AN1_LEVEL, reg->an_level_1);
	write_data (A_AN2_LEVEL, reg->an_level_2);
	return 0;
}

static int
check_alarms (struct S_fan_struct *pfan)
{
	return (pfan->starter_alarm != 0) || (pfan->cc_alarm != 0) || (pfan->sor_alarm != 0) || (pfan->vibr_alarm != 0);
}

/*
	Funzione che calcola il nuovo livello di ventilazione
	Il livello di ventilazione cresce o cala ad ogni ciclo di massimo 1 livello
	Cresce se almeno uno dei due livelli (co o opacimetro) e' superiore alla rispettiva soglia
	Cala se entrambi i livelli (co o opacimetro) sono inferiori alle rispettiva soglie
*/
// macro per calcolo isteresi 5%
#define HIST5(A)	(A+((A)/100*5))

static int
set_vent_level (void)
{
	int co_level, op_level;


	if (vent_reg.co_level_1 > vent_reg.co_level_2)
		co_level = vent_reg.co_level_1;
	else
		co_level = vent_reg.co_level_2;


	if (vent_reg.op_level_1 > vent_reg.op_level_2)
		op_level = vent_reg.op_level_1;
	else
		op_level = vent_reg.op_level_2;

	if (op_level > 32000)
		op_level = 0;

	if ( vent_reg.check_flag )
	{
		vent_reg.check_timer = count1s_time(vent_reg.check_timer) ;
		if (vent_reg.check_timer == TMR_CHECK_ACTIVE ) 
		{
			vent_reg.check_timer = 0 ;	// reset timer
			vent_reg.check_flag = 0 ;	// stop check
		}
	} else {
		// not in check phase
		vent_reg.check_timer = count1s_time(vent_reg.check_timer) ;
		if (vent_reg.check_timer == TMR_CHECK_INTERVAL ) 
		{
			vent_reg.check_timer = 0 ;
			vent_reg.check_flag = 1 ;
		}
	}	
	
	vent_reg.change_timer = count1s_time(vent_reg.change_timer) ;
	switch (vent_reg.vent_level)
	{
	case 0:
		if ((co_level >= vent_reg.co_lev1_th) || (op_level >= vent_reg.op_lev1_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_UP_CHANGE )
			{
				vent_reg.vent_level = 1;
				DEBUG_LOG("level up");
			}
		} else {
			vent_reg.change_timer = 0 ;	
		}
		break;
	case 1:
		if ((co_level >= vent_reg.co_lev2_th) || (op_level >= vent_reg.op_lev2_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_UP_CHANGE )
			{
				vent_reg.vent_level = 2;
			}
		} else	if ((co_level < vent_reg.co_lev1_th) && (op_level < vent_reg.op_lev1_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_DN_CHANGE )
			{
				vent_reg.vent_level = 0;
			}
		} else
		{
			vent_reg.change_timer = 0 ;		
		}
		break;
	case 2:
		if ((co_level >= vent_reg.co_lev3_th) || (op_level >= vent_reg.op_lev3_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_UP_CHANGE )
			{
				vent_reg.vent_level = 3;
			}
		}
		else if ((co_level < vent_reg.co_lev2_th) && (op_level < vent_reg.op_lev2_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_DN_CHANGE )
			{
				vent_reg.vent_level = 1;
			}
		} else
		{
			vent_reg.change_timer = 0 ;		
		}
		break;
	case 3:
		if ((co_level >= vent_reg.co_lev4_th) || (op_level >= vent_reg.op_lev4_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_UP_CHANGE )
			{
				vent_reg.vent_level = 4;
			}
		}
		else if ((co_level < vent_reg.co_lev3_th) && (op_level < vent_reg.op_lev3_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_DN_CHANGE )
			{
				vent_reg.vent_level = 2;
			}
		} else
		{
			vent_reg.change_timer = 0 ;		
		}
		break;
	case 4:
		if ((co_level < vent_reg.co_lev4_th) && (op_level < vent_reg.op_lev4_th))
		{
			if ( vent_reg.change_timer > TMR_VENT_UP_CHANGE )
			{
				vent_reg.vent_level = 3;
			}
		} else
		{
			vent_reg.change_timer = 0 ;
		}
		break;
	default:
		return -1;
	}
	return vent_reg.vent_level;
}

static inline int
count_fan_running (struct S_fan_struct *pfans)
{
	int i, running = 0;

	for (i = 0; i < FAN_NUM; i++)
	{
		running += (((pfans[i].cmd == FAN_CMD_UP) || (pfans[i].cmd == FAN_CMD_DOWN)) ? 1 : 0);
	}

	return running;
}

//*************************** funzioni per singolo di ventilatore ****
static inline int
find_fan2start (struct S_fan_struct *pfans)
{
	int i;
	int found = -1;
	long trun = 10000000L;

	for (i = 0; i < FAN_NUM; i++)
	{
		if ((pfans[i].state == FAN_STATE_OFF) && (pfans[i].trun_h_count < trun))
		{
			found = i;
			trun = pfans[i].trun_h_count;
		}
	}
	return found;
}

//
//
static inline int
find_fan2stop (struct S_fan_struct *pfans)
{
	int i;
	int found = -1;
	long trun = -1;

	for (i = 0; i < FAN_NUM; i++)
	{
		if (((pfans[i].cmd == FAN_CMD_UP) || (pfans[i].cmd == FAN_CMD_DOWN)) && (pfans[i].trun_h_count > trun))
		{
			found = i;
			trun = pfans[i].trun_h_count;
		}
	}
	return found;
}

// cerca se deve ciclare un ventilatore per ore di funzionamento
static inline int
find_fan2cicle (struct S_fan_struct *pfans)
{
	int i;
	int found_running = -1;
	int found_stopped = -1;
	long trun_running = -1;
	long trun_stopped = -1;

	for (i = 0; i < FAN_NUM; i++)
	{
		if (((pfans[i].cmd == FAN_CMD_UP) || (pfans[i].cmd == FAN_CMD_DOWN)) && (pfans[i].trun_h_count > trun_running))
		{
			found_running = i;
			trun_running = pfans[i].trun_h_count;
		}
	}

	trun_stopped = 10000000L;
	for (i = 0; i < FAN_NUM; i++)
	{
		if ((pfans[i].state == FAN_STATE_OFF) && (pfans[i].trun_h_count < trun_stopped))
		{
			found_stopped = i;
			trun_stopped = pfans[i].trun_h_count;
		}
	}

	if ((found_running != -1) && (found_stopped != -1) && (trun_stopped < trun_running))
	{
		return found_running;
	}

	return -1;
}

//
//
static void
do_fan_start_stop (void)
{
	int run_num, i;

	run_num = count_fan_running (fan);
	if (run_num < vent_reg.fan_lev_num[(vent_reg.vent_level)])
	{
		//Cerco il ventilatore con meno ore di funzionamento e gli do lo start
		if (((i = find_fan2start (fan)) != -1))
		{
			fan[i].cmd = vent_reg.direction;
		}
	}
	else if (run_num > vent_reg.fan_lev_num[(vent_reg.vent_level)])
	{
		//Cerco tra i ventilatori accesi quello con piu' ore di utilizzo e gli do lo stop
		if ((i = find_fan2stop (fan)) != -1)
		{
			fan[i].cmd = FAN_CMD_OFF;
		}
	}
	else
	{
		if ((i = find_fan2cicle (fan)) != -1)
		{
			fan[i].cmd = FAN_CMD_OFF;
		}
	}
	return;
}

// Inverte il senso di rotazione del ventilatore
static void
update_fan_cmd (struct S_fan_struct *pfan)
{
	int i;
	for (i = 0; i < FAN_NUM; i++)
	{
		if ((pfan[i].state == FAN_STATE_RUN_DOWN) && (vent_reg.direction == FAN_CMD_UP))
		{
			pfan[i].cmd = FAN_CMD_OFF;
		}
		if ((pfan[i].state == FAN_STATE_RUN_UP) && (vent_reg.direction == FAN_CMD_DOWN))
		{
			pfan[i].cmd = FAN_CMD_OFF;
		}
		if (check_alarms (&pfan[i]) || (pfan[i].state == FAN_STATE_BAD) || (pfan[i].state == FAN_STATE_MANUAL)
		    || (pfan[i].state == FAN_STATE_SEMIAUTO))
		{
			pfan[i].cmd = FAN_CMD_OFF;
		}
	}
	return;
}



//*************************** funzioni per coppia di ventilatori ****
static inline int
count_twin_running (struct S_fan_struct *pfans)
{
	int i, running = 0;

	for (i = 0; i < FAN_NUM; i++)
	{
		running += (((pfans[i].cmd == FAN_CMD_UP) || (pfans[i].cmd == FAN_CMD_DOWN)) ? 1 : 0);
	}
	return (running & 0xfffe) ;
}

static inline int
find_twin2start (struct S_fan_struct *pfans)
{
	int i;
	int found = -1;
	long trun =    10000000L;
	long twinrun = 10000000L;

        // cerca se c'è un ventilatore singolo avviato
	for (i = 0; i < FAN_NUM; i += 2)
	{
		if ( ((pfans[i].cmd == FAN_CMD_UP) || (pfans[i].cmd == FAN_CMD_DOWN)) && (pfans[i + 1].state == FAN_STATE_OFF))
		{
                        return i+1 ;    // start next
		}
		if ( ((pfans[i + 1].cmd == FAN_CMD_UP) || (pfans[i + 1].cmd == FAN_CMD_DOWN)) && (pfans[i].state == FAN_STATE_OFF) )
		{
                        return i ;      // start previous
		}
	}

	for (i = 0; i < FAN_NUM; i += 2)
	{
		if ((pfans[i].state == FAN_STATE_OFF) && (pfans[i + 1].state == FAN_STATE_OFF))
		{
			twinrun = (pfans[i].trun_h_count + pfans[i + 1].trun_h_count) / 2;
			if ((twinrun < trun))
			{
			    found = i;
			    trun = twinrun;
			}
		}
	}
	return found;
}

//
//
static inline int
find_twin2stop (struct S_fan_struct *pfans)
{
	int i;
	int found = -1;
	long trun = -1;
	long twinrun = -1;

	for (i = 0; i < FAN_NUM; i += 2)
	{
		if (((pfans[i].cmd == FAN_CMD_UP) || (pfans[i].cmd == FAN_CMD_DOWN)) &&
		    ((pfans[i + 1].cmd == FAN_CMD_UP) || (pfans[i + 1].cmd == FAN_CMD_DOWN)))
		{
			twinrun = (pfans[i].trun_h_count + pfans[i + 1].trun_h_count) / 2;
			if ((twinrun > trun))
			{
			    found = i;
			    trun = twinrun;
			}
		}
	}
	return found;
}

// cerca se deve ciclare una coppia ventilatori per ore di funzionamento
static inline int
find_twin2cicle (struct S_fan_struct *pfans)
{
	int found_running = -1;
	int found_stopped = -1;
	long trun_running = -1;
	long trun_stopped = -1;

	found_running = find_twin2stop (pfans);
	if (found_running != -1)
		trun_running = (pfans[found_running].trun_h_count + pfans[found_running + 1].trun_h_count) / 2;

	found_stopped = find_twin2start (pfans);
	if (found_stopped != -1)
		trun_stopped = (pfans[found_stopped].trun_h_count + pfans[found_stopped + 1].trun_h_count) / 2;

	if ((found_running != -1) && (found_stopped != -1) && (trun_stopped < trun_running))
	{
		return found_running;
	}
	return -1;
}

//
//
static void
do_twin_start_stop (void)
{
	int run_num, i;

	run_num = count_twin_running (fan);
	if (run_num < vent_reg.fan_lev_num[(vent_reg.vent_level)])
	{
		//Cerco la coppia ventilatore con meno ore di funzionamento e gli do lo start
		if (((i = find_twin2start (fan)) != -1))
		{
			fan[i].cmd = vent_reg.direction;
			//fan[i + 1].cmd = vent_reg.direction;
		}
	}
	else if (run_num > vent_reg.fan_lev_num[(vent_reg.vent_level)])
	{
		//Cerco tra i ventilatori accesi quello con piu' ore di utilizzo e gli do lo stop
		if ((i = find_twin2stop (fan)) != -1)
		{
			fan[i].cmd = FAN_CMD_OFF;
			fan[i + 1].cmd = FAN_CMD_OFF;
		}
	}
	else
	{
		if ((i = find_twin2cicle (fan)) != -1)
		{
			fan[i].cmd = FAN_CMD_OFF;
			fan[i + 1].cmd = FAN_CMD_OFF;
		}
	}

	return;
}

// Inverte il senso di rotazione del ventilatore
static void
update_twin_cmd (struct S_fan_struct *pfan)
{
	int i;
	for (i = 0; i < FAN_NUM; i++)
	{
		if ((pfan[i].state == FAN_STATE_RUN_DOWN) && (vent_reg.direction == FAN_CMD_UP))
		{
			pfan[i].cmd = FAN_CMD_OFF;
		}
		if ((pfan[i].state == FAN_STATE_RUN_UP) && (vent_reg.direction == FAN_CMD_DOWN))
		{
			pfan[i].cmd = FAN_CMD_OFF;
		}
	}

	for (i = 0; i < FAN_NUM; i += 2)
	{
		if (check_alarms (&pfan[i]) || (pfan[i].state == FAN_STATE_BAD) || (pfan[i].state == FAN_STATE_MANUAL)
		    || (pfan[i].state == FAN_STATE_SEMIAUTO))
		{
			pfan[i].cmd = FAN_CMD_OFF;
			pfan[i + 1].cmd = FAN_CMD_OFF;
		}
		if (check_alarms (&pfan[i + 1]) || (pfan[i + 1].state == FAN_STATE_BAD) || (pfan[i + 1].state == FAN_STATE_MANUAL)
		    || (pfan[i + 1].state == FAN_STATE_SEMIAUTO))
		{
			pfan[i].cmd = FAN_CMD_OFF;
			pfan[i + 1].cmd = FAN_CMD_OFF;
		}
	}
	return;
}


//***********************************************************************************
void
init_ventilazione_data (void)
{
	int i;
	vent_reg.gestione = FAN_SINGLE;
	vent_reg.change_timer = 0;
	vent_reg.check_timer = 0 ;
	vent_reg.check_flag = 0 ;
	for (i = 0; i < FAN_NUM; i++)
	{
		memset (&fan[i], 0, sizeof (struct S_fan_struct));
		fan[i].state = FAN_STATE_OFF;
		fan[i].cmd = FAN_CMD_OFF;
	}

	for(i=0;i<10;i++)
	{
		vent_reg.co_level_1_in[i]=0;
		vent_reg.op_level_1_in[i]=0;
		vent_reg.co_level_2_in[i]=0;
		vent_reg.op_level_2_in[i]=0;
	}
	return;
}


//
//

int
ventilazione_process ()
{
	int run_num, i;
	//Acquisisco i dati dalla memoria
	read_vent_data (&vent_reg, fan);

	run_num = count_fan_running (fan);
	vent_reg.num_attivi = run_num;

//	DEBUG_LOG("ch timer %d, scene %d", vent_reg.change_timer, vent_reg.active_scene);
	//Se lo scenario e' automatico accende o spegne i ventilatori a seconda del livello
	switch (vent_reg.active_scene)
	{
	case V_SCENE_AUTO:
		set_vent_level ();
		if (read_data (PULSE_60SEC_ADDR) != 0)
		{
			// se ventilazione ferma controllo direzione del vento
			if ( run_num == 0 )
			{
				// test se segno uguale
				DEBUG_LOG("check vento");
				if ( vent_reg.abs_an_level_1 > vent_reg.abs_an_level_2 )
				{
					if ( vent_reg.abs_an_level_1 > vent_reg.soglia_anemometro )
					{
						if ( read_data(V_AN1_DIR) == 1 )
						{
    						    vent_reg.direction = FAN_CMD_UP;
						    DEBUG_LOG("check vento UP AN1");
						} else 
						{
						    vent_reg.direction = FAN_CMD_DOWN;
						    DEBUG_LOG("check vento DOWN AN1");
						}
					} else
					{
						vent_reg.direction = vent_reg.set_direction;
						DEBUG_LOG("check vento DEF AN1");
					}
				} else
				{
					if ( vent_reg.abs_an_level_2 > vent_reg.soglia_anemometro )
					{
						if ( read_data(V_AN2_DIR) == 1 )
						{
    						    vent_reg.direction = FAN_CMD_UP;
						    DEBUG_LOG("check vento UP AN2");
						} else 
						{
						    vent_reg.direction = FAN_CMD_DOWN;
						    DEBUG_LOG("check vento DOWN AN2");
						}
					} else
					{
						vent_reg.direction = vent_reg.set_direction;
						DEBUG_LOG("check vento DEF AN2");
					}
				}
			}
			
			if (vent_reg.check_flag)
			{
				int i;
				for (i = 0; i < FAN_NUM; i++)
				{
					fan[i].cmd = FAN_CMD_OFF;
				}

			} else {
				if (vent_reg.gestione == FAN_SINGLE)
				{
					//Gestione interna ventilazione
					do_fan_start_stop ();
				}
				if (vent_reg.gestione == FAN_DUAL)
				{
					//guardo il numero di coppie accese
					do_twin_start_stop ();
				}
			}
			
		}
		break;

	case V_SCENE_SLOW_UP:
		if (read_data (PULSE_60SEC_ADDR) != 0)
		{
			vent_reg.direction = FAN_CMD_UP;
			vent_reg.vent_level = 1;
			if (vent_reg.gestione == FAN_SINGLE)
			{
				//Gestione interna ventilazione
				do_fan_start_stop ();
			}
			if (vent_reg.gestione == FAN_DUAL)
			{
				//guardo il numero di coppie accese
				do_twin_start_stop ();
			}
		}
		break;

	case V_SCENE_FAST_UP:
		if (read_data (PULSE_60SEC_ADDR) != 0)
		{
			vent_reg.direction = FAN_CMD_UP;
			vent_reg.vent_level = 3;
			if (vent_reg.gestione == FAN_SINGLE)
			{
				//Gestione interna ventilazione
				do_fan_start_stop ();
			}
			if (vent_reg.gestione == FAN_DUAL)
			{
				//guardo il numero di coppie accese
				do_twin_start_stop ();
			}
		}
		break;

	case V_SCENE_SLOW_DN:
		if (read_data (PULSE_60SEC_ADDR) != 0)
		{
			vent_reg.direction = FAN_CMD_DOWN;
			vent_reg.vent_level = 1;
			if (vent_reg.gestione == FAN_SINGLE)
			{
				//Gestione interna ventilazione
				do_fan_start_stop ();
			}
			if (vent_reg.gestione == FAN_DUAL)
			{
				//guardo il numero di coppie accese
				do_twin_start_stop ();
			}
		}
		break;

	case V_SCENE_FAST_DN:
		if (read_data (PULSE_60SEC_ADDR) != 0)
		{
			vent_reg.direction = FAN_CMD_DOWN;
			vent_reg.vent_level = 3;
			if (vent_reg.gestione == FAN_SINGLE)
			{
				//Gestione interna ventilazione
				do_fan_start_stop ();
			}
			if (vent_reg.gestione == FAN_DUAL)
			{
				//guardo il numero di coppie accese
				do_twin_start_stop ();
			}
		}
		break;

	case V_SCENE_STOP_AL:
		for (i = 0; i < FAN_NUM; i++)
		{
        	    if ( !(check_alarms (&fan[i]) || (fan[i].state == FAN_STATE_BAD) 
				|| (fan[i].state == FAN_STATE_MANUAL) || (fan[i].state == FAN_STATE_SEMIAUTO)))
		    {
			fan[i].cmd = FAN_CMD_OFF ;
		    }
		}
		break;


	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		{
			//Scenario impostato dall'utente
			int index = (vent_reg.active_scene);
			if (read_data (PULSE_30SEC_ADDR) != 0)
			{
				for (i = 0; i < FAN_NUM; i++)
				{
					if (fan[i].cmd != vent_reg.scene[index][i] &&
					    !(check_alarms (&fan[i]) || (fan[i].state == FAN_STATE_BAD) || (fan[i].state == FAN_STATE_MANUAL)
					      || (fan[i].state == FAN_STATE_SEMIAUTO)))
					{
						fan[i].cmd = vent_reg.scene[index][i];
						break;
					}
				}
			}
			break;
		}
	default:
		vent_reg.active_scene = 0;
		break;
	}

	if (vent_reg.gestione == FAN_SINGLE)
	{
		//Gestione interna ventilazione
		update_fan_cmd (fan);
	}
	if (vent_reg.gestione == FAN_DUAL)
	{
		//guardo il numero di coppie accese
		update_twin_cmd (fan);
	}

	//Ricopio i dati in memoria
	write_vent_data (&vent_reg, fan);

	return 0;
}
