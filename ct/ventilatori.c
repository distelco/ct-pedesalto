/*
    air_process.c
    Processo gestione ventilazione
    Written By Franco Novi
    Distelco s.r.l.
    $Id: ventilatori.c,v 1.4 2009-01-29 10:53:19 matteoc Exp $
*/
//#define DEBUG

//#undef DEBUG

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "mymemory.h"
#include "ventilatori.h"
#include "timer.h"

static struct S_fan_struct fan[FAN_NUM];


static void fan_up(struct S_fan_struct*f)
{
	f->up_cmd   = 1;
	f->down_cmd = 0;
	return;
}
static void fan_down(struct S_fan_struct*f)
{
	f->up_cmd   = 0;
	f->down_cmd = 1;
	return;
}
static void fan_stop(struct S_fan_struct*f)
{
	f->up_cmd   = 0;
	f->down_cmd = 0;
	return;
}

static int read_vent_data( struct S_fan_struct *pfans )
{
	int i;
	int automatic = 0; 
	int semiauto = 0;
	
	for( i=0;i< FAN_NUM; i++)
	{
		//selezione del modo operativo
		automatic = read_data((M_STATE_BASE + (i*Q_OFFSET))) 
			    && read_data((Q_ONOFF_BASE + (i*Q_OFFSET)));
		semiauto = read_data((SA_STATE_BASE + i));
		if( automatic == 0 )
		{
			pfans[i].mode = FAN_MODE_MANUAL;
			pfans[i].cmd  = FAN_CMD_OFF;
			pfans[i].up_cmd   = 0;
			pfans[i].down_cmd = 0;	
		} 
		else if(semiauto != 0) 
		{
			pfans[i].mode = FAN_MODE_SEMIAUTO;
			pfans[i].cmd  = FAN_CMD_OFF;
			pfans[i].up_cmd = read_data(SA_UPCMD_BASE + (i*SA_CMD_OFFSET));
			pfans[i].down_cmd = read_data(SA_DOWNCMD_BASE + (i*SA_CMD_OFFSET));
		} 
		else 
		{
			pfans[i].mode = FAN_MODE_AUTO;			
			pfans[i].cmd  = read_data(A_CMD_BASE + i);
		}
		
		pfans[i].rma = read_data((RMA_BASE + (i*IS_OFFSET)));
		pfans[i].rmi = read_data((RMI_BASE + (i*IS_OFFSET)));
		pfans[i].starter = read_data((S_ALARM_BASE + (i*Q_OFFSET)));	
		pfans[i].cc = read_data((CC_ALARM_BASE + (i*Q_OFFSET)));
		
		pfans[i].sor = read_data((SOR_BASE + i));
		pfans[i].svib = read_data(SVIB_BASE + i);
		pfans[i].vib_th = read_data(VIB_TH_BASE + i);
		
		pfans[i].alm_reset = read_data(ALM_RESET_BASE + (i*ALM_OFFSET));
		pfans[i].alm_ignore = read_data(ALM_DISABLE_BASE + (i*ALM_OFFSET));

		pfans[i].toff_count = read_data(TOFF_COUNT_BASE + i);
		pfans[i].trun_m_count = read_data(TRUN_M_COUNT_BASE + i);
		pfans[i].trun_h_count = read_data(TRUN_H_COUNT_BASE + i);
	}
	
	return 0;
}

static int write_vent_data( struct S_fan_struct *pfans )
{
	int i;
	for( i=0;i< FAN_NUM; i++)
	{
		write_data((CMD_UP_BASE+(i*CMD_OFFSET)), pfans[i].up_cmd);
		write_data((CMD_DOWN_BASE+(i*CMD_OFFSET)), pfans[i].down_cmd);
		write_data((TOFF_COUNT_BASE + i), pfans[i].toff_count );
		write_data((FAN_STATE_BASE + i), pfans[i].state );
		write_data((TRUN_H_COUNT_BASE + i), pfans[i].trun_h_count );
		write_data((TRUN_M_COUNT_BASE + i), pfans[i].trun_m_count );
		
		write_data((MEM_VIBR_ALM_BASE+(i*MEM_OFFSET)), pfans[i].vibr_alarm);
		write_data((MEM_START_ALM_BASE+(i*MEM_OFFSET)), pfans[i].starter_alarm);
		write_data((MEM_CC_ALM_BASE+(i*MEM_OFFSET)), pfans[i].cc_alarm);
		write_data((MEM_SOR_ALM_BASE+(i*MEM_OFFSET)), pfans[i].sor_alarm);
		
	}
	return 0;
}

static int check_alarms(struct S_fan_struct * pfan)
{
	if ( (pfan->svib > pfan->vib_th) && (pfan->vib_th > 100) && 
                (pfan->state == FAN_STATE_RUN_UP || pfan->state == FAN_STATE_RUN_DOWN) )
	{
                pfan->vib_cnt++ ;
	} else {
                pfan->vib_cnt=0 ;
        }
	if ( pfan->vib_cnt > 10 )
        {
                pfan->vibr_alarm = 1 ;
        }
	
	if (pfan->sor != 0)
	{
//		pfan->sor_alarm = 1 ;
	}

	if ( pfan->cc != 0 )
	{
		pfan->cc_alarm = 1 ;
	}
	
	if ( pfan->starter != 0 )
	{
		pfan->starter_alarm = 1 ;
	}
	if ( pfan->alm_ignore != 0 )
	{
		return 0 ;
	} else {			
		return (pfan->starter_alarm != 0) || (pfan->cc_alarm != 0) || (pfan->sor_alarm != 0) || (pfan->vibr_alarm != 0);
	}
}

static void reset_alarms( struct S_fan_struct * pfan)
{
	pfan->vibr_alarm = 0 ;
	pfan->sor_alarm = 0 ;
	pfan->cc_alarm = 0 ;
	pfan->starter_alarm = 0 ;
	return ;
}

static int auto_mode(struct S_fan_struct * pfan)
{
	//Inizio gestione
	switch (pfan->state)
	{
		
		case FAN_STATE_OFF:
			// ventilatore fermo
			fan_stop(pfan);
			
			if( check_alarms(pfan) > 0 )			
			{
				//Ho un allarme attivo
				//Cambio lo stato del ventilatore
				pfan->state = FAN_STATE_BAD;	
				break ;
			} 
			
			if(pfan->cmd == FAN_CMD_UP) {
				//Comando avanti
				pfan->state = FAN_STATE_START_UP;
				pfan->start_time = FAN_ON_TIME ;
			} else if(pfan->cmd == FAN_CMD_DOWN) {
				//Comando di avviamento
				pfan->state = FAN_STATE_START_DOWN;
				pfan->start_time = FAN_ON_TIME ;
			}
			break;
		
		case FAN_STATE_STOP:
			//spegnimento ventilatore
			fan_stop(pfan);
			if( check_alarms(pfan) > 0 )			
			{
				pfan->state = FAN_STATE_BAD;
				
			} else if( (pfan->toff_count=decount1s_time(pfan->toff_count)) <= 0 ) {
				//Tempo spegnimento ultimato
				pfan->state = FAN_STATE_OFF;
				pfan->toff_count = 0;
			}
			break;
			
		case FAN_STATE_START_UP:
			//Avviamento avanti
			if(check_alarms(pfan) > 0)
			{
				//Allarme avviatore,fermo il ventilatore
				fan_stop(pfan);
				pfan->state = FAN_STATE_BAD;
			} else {
				fan_up(pfan);
				if( (pfan->start_time=decount1s_time(pfan->start_time)) <= 0 )
				{ 
					pfan->state = FAN_STATE_RUN_UP;
				}
			}
			break; 
			
		case FAN_STATE_RUN_UP:
			if( check_alarms(pfan) > 0 )			
			{
				fan_stop(pfan);
				pfan->state = FAN_STATE_BAD;
				break;
			}
			
			if (pfan->cmd == FAN_CMD_OFF){
				//Fermo il ventilatore
				fan_stop(pfan);
				pfan->state = FAN_STATE_STOP;
				pfan->toff_count = FAN_OFF_TIME;
			} else if(pfan->cmd == FAN_CMD_DOWN){
				//Fermo il ventilatore
				fan_stop(pfan);
				pfan->state = FAN_STATE_STOP;				
				pfan->toff_count = FAN_OFF_TIME;
			} else {
				fan_up(pfan);
			}
			break; 
			
		case FAN_STATE_START_DOWN:
			//Avviamento avanti
			if(check_alarms(pfan) > 0)
			{
				//Allarme avviatore,fermo il ventilatore
				fan_stop(pfan);
				pfan->state = FAN_STATE_BAD;
			} else {
				//Avviamento
				fan_down(pfan);
				if( (pfan->start_time=decount1s_time(pfan->start_time) ) <= 0)
				{ 
					pfan->state = FAN_STATE_RUN_DOWN;
				}
			}
			break; 
			
		case FAN_STATE_RUN_DOWN:
			if( check_alarms(pfan) > 0 )			
			{
				fan_stop(pfan);
				pfan->state = FAN_STATE_BAD;
				break ;
			}
			
			if (pfan->cmd == FAN_CMD_OFF){
				fan_stop(pfan);
				pfan->state = FAN_STATE_STOP;
				pfan->toff_count = FAN_OFF_TIME;
			} else if(pfan->cmd == FAN_CMD_UP){
				fan_stop(pfan);
				pfan->state = FAN_STATE_STOP;				
				pfan->toff_count = FAN_OFF_TIME;
			} else {
				fan_down(pfan);
			}
			
			break; 

		case FAN_STATE_BAD:
			//Arresto il ventilatore
			fan_stop(pfan);
			if(check_alarms(pfan) == 0) {
				pfan->state = FAN_STATE_OFF;
			}
			break; 
	
		default:
			fan_stop(pfan);
			pfan->state = FAN_STATE_OFF ;
	}
	return 0;
}

void init_fan_data( void )
{
	int i;
	for( i=0;i< FAN_NUM; i++)
	{
		memset(&fan[i], 0, sizeof(struct S_fan_struct));
		fan[i].state = FAN_STATE_OFF;
		fan[i].mode = FAN_MODE_AUTO;
		fan[i].cmd = FAN_CMD_OFF;
		fan[i].starter_alarm = 0;
		fan[i].cc_alarm = 0;
		fan[i].vibr_alarm = 0;
		fan[i].rma = 0;
		fan[i].rmi = 0;
		fan[i].sor = 0;
		fan[i].svib = 0;
                fan[i].vib_cnt = 0;
	}
	return;
}


int fan_process()
{
	int i;

	//Acquisisco i dati dalla memoria
	read_vent_data( &fan[0] );

	
	for(i=0; i<FAN_NUM; i++)
	{
		//Discrimino il modo di funzionamento
		if (fan[i].alm_reset)
		{
			reset_alarms(&fan[i]) ;
		}
		switch(fan[i].mode)
		{
			case FAN_MODE_MANUAL:	//imposto lo stato OFF
				fan[i].state = FAN_STATE_MANUAL;
				fan_stop(&fan[i]);
				break;
			case FAN_MODE_SEMIAUTO:	// manutenzione
				if( fan[i].up_cmd && fan[i].down_cmd )
				{
					//gestisce la configurazione errata di entrambi i comandi attivi
					fan[i].up_cmd   = 0;
					fan[i].down_cmd = 0;	
				}
				fan[i].state = FAN_STATE_SEMIAUTO;
				break;
			case FAN_MODE_AUTO:
				auto_mode(&fan[i]);
				break;
			default:
				return -1;
		}
		if ( fan[i].rmi || fan[i].rma )
		{
    		    //aggiorno contatore secondi di funzionamento
		    fan[i].trun_s_count = count1s_time( fan[i].trun_s_count );
		    //aggiorno contatore minuti di funzionamento
		    if (fan[i].trun_s_count == 60)
		    {
			fan[i].trun_s_count = 0 ;
			fan[i].trun_m_count++;
		    }
		    //aggiorno contatore ore di funzionamento
		    if ( fan[i].trun_m_count == 60 )
		    {
			fan[i].trun_m_count = 0 ;
			fan[i].trun_h_count++ ;
		    }
		}
	}
	//Ricopio i dati in memoria
	write_vent_data(&fan[0]);
	
	return 0;
}
