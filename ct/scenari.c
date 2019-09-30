/*
	scenari.c
	Contine la gestione degli scenari del CT
	Written By Franco Novi & Matteo Costa
	Distelco s.r.l.
	$Id: scenari.c,v 1.3 2009-01-29 10:32:55 matteoc Exp $
*/

#include "common.h"
#include "scenari.h"


struct S_scenari scenari;
#define CO_TIMER_INI	300
static int co_timer = CO_TIMER_INI;


static void
read_dati_scenari (struct S_scenari *s)
{
	int i;

	for(i=0;i<SOS_NUM;i++)
	{
		if ( trig_data (SOS_SOCCORSO_BASE + i * SOS_OFFSET) 
		    && (read_data(SOS_SOCCORSO_BASE + i * SOS_OFFSET) == 1) )
		{
		    s->sos[i].soccorso = 1;
		}
		if ( trig_data (SOS_INCENDIO_BASE + i * SOS_OFFSET) 
		    && (read_data(SOS_INCENDIO_BASE + i * SOS_OFFSET) == 1) )
		{
		    s->sos[i].incendio = 1;
		}
		if ( trig_data (SOS_POLIZIA_BASE + i * SOS_OFFSET) 
		    && (read_data(SOS_POLIZIA_BASE + i * SOS_OFFSET) == 1) )
		{
		    s->sos[i].polizia = 1;
		}
		if ( trig_data (SOS_GUASTO_BASE + i * SOS_OFFSET) 
		    && (read_data(SOS_GUASTO_BASE + i * SOS_OFFSET) == 1) )
		{
		    s->sos[i].guasto = 1;
		}
		if ( trig_data (SOS_ESTINTORE_BASE + i * SOS_OFFSET) 
		    && (read_data(SOS_ESTINTORE_BASE + i * SOS_OFFSET) == 1) 
		    && (read_data(SOS_PORTA_BASE + i * SOS_OFFSET) == 1))
		{
		    s->sos[i].estintore = 1;
		}
		if ( trig_data (SOS_MERCI_BASE + i * SOS_OFFSET) 
		    && (read_data(SOS_MERCI_BASE + i * SOS_OFFSET) == 1) )
		{
		    s->sos[i].merci_pericolose = 1;
		    s->cartello_merci_timeout = CARTELLI_TIMEOUT ;
		}
		if ( trig_data (SOS_INCIDENTE_BASE + i * SOS_OFFSET) 
		    && (read_data(SOS_INCIDENTE_BASE + i * SOS_OFFSET) == 1) )
		{
		    s->sos[i].incidente = 1;
		    s->cartello_incidente_timeout = CARTELLI_TIMEOUT ;
		}


		s->sos[i].enable_soccorso = read_data (SOS_EN_SOCCORSO_BASE + i * SOS_EN_OFFSET);
		s->sos[i].enable_incendio = read_data (SOS_EN_INCENDIO_BASE + i * SOS_EN_OFFSET);
		s->sos[i].enable_polizia = read_data (SOS_EN_POLIZIA_BASE + i * SOS_EN_OFFSET);
		s->sos[i].enable_guasto = read_data (SOS_EN_GUASTO_BASE + i * SOS_EN_OFFSET);
		s->sos[i].enable_estintore = read_data (SOS_EN_ESTINTORE_BASE + i * SOS_EN_OFFSET);
	}

	if ( trig_data (SOS_C_SOCCORSO_BASE) && (read_data(SOS_C_SOCCORSO_BASE) == 1) )
	{
		    s->sos[2].soccorso = 1;
	}
	if ( trig_data (SOS_C_INCENDIO_BASE) && (read_data(SOS_C_INCENDIO_BASE) == 1) )
		{
		    s->sos[2].incendio = 1;
		}
	if ( trig_data (SOS_C_POLIZIA_BASE) && (read_data(SOS_C_POLIZIA_BASE) == 1) )
		{
		    s->sos[2].polizia = 1;
		}
	if ( trig_data (SOS_C_GUASTO_BASE) && (read_data(SOS_C_GUASTO_BASE) == 1) )
		{
		    s->sos[2].guasto = 1;
		}
	if ( trig_data (SOS_C_ESTINTORE_BASE) && (read_data(SOS_C_ESTINTORE_BASE) == 1) )
		{
		    s->sos[2].estintore = 1;
		}

	for ( i=0; i<MAX_SCENARIO_ALARMS; i++)
	{
		s->alarms[i].enable = read_data( SCEN_ALM_ENABLE_BASE + i * SCEN_ALM_CS_OFFSET ) ;	
		s->alarms[i].priority =  read_data( SCEN_ALM_PRIO_BASE + i * SCEN_ALM_HR_OFFSET ) ;
		s->alarms[i].scen_ventilazione = read_data( SCEN_ALM_VENT_BASE + i * SCEN_ALM_HR_OFFSET ) ;
		s->alarms[i].scen_illuminazione = read_data( SCEN_ALM_ILL_BASE + i * SCEN_ALM_HR_OFFSET ) ;
		s->alarms[i].scen_traffico = read_data( SCEN_ALM_CTR_BASE + i * SCEN_ALM_HR_OFFSET ) ;
		s->alarms[i].immediato_ventilazione = read_data( SCEN_ALM_IMM_VENT_BASE + i * SCEN_ALM_IMM_OFFSET ) ;
		s->alarms[i].immediato_illuminazione = read_data( SCEN_ALM_IMM_ILL_BASE + i * SCEN_ALM_IMM_OFFSET ) ;
		s->alarms[i].immediato_traffico = read_data( SCEN_ALM_IMM_CTR_BASE + i * SCEN_ALM_IMM_OFFSET ) ;
	}
	
	s->conferma_scenario = (trig_data (S_CONFERMA_SCENARIO) & read_data (S_CONFERMA_SCENARIO));
	s->annulla_scenario = (trig_data (S_ANNULLA_SCENARIO) & read_data (S_ANNULLA_SCENARIO));
	
	s->ill_conferma_scenario = (trig_data (S_ILL_CONF_SCEN) & read_data (S_ILL_CONF_SCEN));
	s->vent_conferma_scenario = (trig_data (S_VENT_CONF_SCEN) & read_data (S_VENT_CONF_SCEN));
	s->traff_conferma_scenario = (trig_data (S_CTR_CONF_SCEN) & read_data (S_CTR_CONF_SCEN));
	
	s->ill_annulla_scenario = (trig_data (S_ILL_ANN_SCEN) & read_data (S_ILL_ANN_SCEN));
	s->vent_annulla_scenario = (trig_data (S_VENT_ANN_SCEN) & read_data (S_VENT_ANN_SCEN));
	s->traff_annulla_scenario = (trig_data (S_CTR_ANN_SCEN) & read_data (S_CTR_ANN_SCEN));
	
	return;
}


static void
write_dati_scenari (struct S_scenari *s)
{
	int i ;

	for(i=0;i<SOS_NUM;i++)
	{
		write_data( SOS_M_SOCCORSO_BASE + i * SOS_M_OFFSET, s->sos[i].soccorso) ;
		write_data( SOS_M_INCENDIO_BASE + i * SOS_M_OFFSET, s->sos[i].incendio) ;
		write_data( SOS_M_POLIZIA_BASE + i * SOS_M_OFFSET, s->sos[i].polizia) ;
		write_data( SOS_M_GUASTO_BASE + i * SOS_M_OFFSET, s->sos[i].guasto) ;
		write_data( SOS_M_ESTINTORE_BASE + i * SOS_M_OFFSET, s->sos[i].estintore) ;
		write_data( SOS_M_MERCI_BASE + i * SOS_M_OFFSET, s->sos[i].merci_pericolose) ;
		write_data( SOS_M_INCIDENTE_BASE + i * SOS_M_OFFSET, s->sos[i].incidente) ;

		if ( 
s->sos[i].merci_pericolose == 1 )
		{
    		    write_data( CARTELLO_MERCI, s->sos[i].merci_pericolose) ;
		}
		if ( 
s->sos[i].incidente == 1 )
		{
    		    write_data( CARTELLO_INCIDENTE, s->sos[i].incidente) ;
		}
	}

	for ( i=0; i<MAX_SCENARIO_ALARMS; i++)
	{
		write_data( SCEN_ALM_ACTIVE_BASE + i * SCEN_ALM_CS_OFFSET, s->alarms[i].active ) ;
	}

	write_data (S_PROPOSTA_SCENARIO, s->alarm_bit);
	write_data (ILL_SCEN_PROP, s->ill_scenario_proposto);
	write_data (VENT_SCEN_PROP, s->vent_scenario_proposto);
	write_data (TRAF_SCEN_PROP, s->traffico_scenario_proposto);
	write_data (RITARDO_ATT_SCEN, s->ritardo_att);
	write_data (S_VENT_CONFERMATO, s->vent_confermato);
	write_data (S_ILL_CONFERMATO, s->ill_confermato);
	write_data (S_CTR_CONFERMATO, s->traff_confermato);

	if (s->current_active != -1 && s->vent_confermato) 
	{
		write_data (VENT_ATT_SCENARIO, read_data (VENT_SCEN_PROP));
	} else {
		write_data (VENT_ATT_SCENARIO, read_data (VENT_MAN_SCENARIO));
	}
	
	if (s->current_active != -1 && s->ill_confermato) 
	{
		write_data (ILL_ATT_SCENARIO, read_data (ILL_SCEN_PROP));
	} else {
		write_data (ILL_ATT_SCENARIO, read_data (ILL_MAN_SCENARIO));
	}

	if (s->current_active != -1 && s->traff_confermato) 
	{
		write_data (CTR_ATT_SCENARIO, read_data (TRAF_SCEN_PROP));
	} else {
		write_data (CTR_ATT_SCENARIO, read_data (CTR_MAN_SCENARIO));
	}

	write_data (S_CURRENT_SCENE_ACT, s->current_active);
	write_data (S_CURRENT_SCENE_PRIO, s->current_priority);

	return;
}

static void
check_co_op_alarm (struct S_scenari *s)
{
	if ((s->co_level <= s->co_th_max) && (s->op_level <= s->op_th_max))
	{
		co_timer = CO_TIMER_INI;
	}

	if ((read_data (PULSE_1SEC_ADDR) > 0) && (co_timer > 0))
	{
		co_timer--;
	}

	if (co_timer == 0)
	{
		co_timer = -1;
		s->alarms[ALM_MAX_CO].active = 1;
	}
	return;
}
static void
clear_co_op_alarm (struct S_scenari *s)
{
    s->alarms[ALM_MAX_CO].active = 0;
    return ;
}

static void
check_incendio_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		if ( (s->sos[i].incendio && s->sos[i].enable_incendio) ) 
				s->alarms[ALM_SOS_INCENDIO].active = 1 ;
	}
	return;
}
static void
clear_incendio_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		s->sos[i].incendio = 0 ;
	}
	s->alarms[ALM_SOS_INCENDIO].active = 0 ;
	return;
}

static void
check_guasto_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		if ( (s->sos[i].guasto && s->sos[i].enable_guasto) ) 
				s->alarms[ALM_SOS_GUASTO].active = 1 ;
	}
	return;
}
static void
clear_guasto_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		s->sos[i].guasto = 0;
	}
	s->alarms[ALM_SOS_GUASTO].active = 0 ;
	return;
}


static void
check_soccorso_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		if ( (s->sos[i].soccorso && s->sos[i].enable_soccorso) ) 
				s->alarms[ALM_SOS_SOCCORSO].active = 1 ;
	}
	return;
}
static void
clear_soccorso_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		s->sos[i].soccorso = 0;
	}
	s->alarms[ALM_SOS_SOCCORSO].active = 0 ;
	return;
}


static void
check_polizia_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		if ( (s->sos[i].polizia && s->sos[i].enable_polizia) ) 
				s->alarms[ALM_SOS_POLIZIA].active = 1 ;
	}
	return;
}
static void
clear_polizia_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		s->sos[i].polizia = 0;
	}
	s->alarms[ALM_SOS_POLIZIA].active = 0 ;
	return;
}

static void
check_estintore_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		if ( (s->sos[i].estintore && s->sos[i].enable_estintore) ) 
				s->alarms[ALM_SOS_ESTINTORE].active = 1 ;
	}
	return;
}
static void
clear_estintore_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SOS_NUM; i++)
	{
		s->sos[i].estintore = 0;
	}
	s->alarms[ALM_SOS_ESTINTORE].active = 0 ;
	return;
}

static void
check_smoke_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SMOKE_ALM_NUM; i++)
	{
		if ( s->smoke[i].alarm ) 
			s->alarms[ALM_FUMO_1+i].active = 1 ;
	}
	return;
}
static void
clear_smoke_alarm (struct S_scenari *s)
{
	int i;
	for (i = 0; i < SMOKE_ALM_NUM; i++)
	{
		s->smoke[i].alarm = 0; 
		s->alarms[ALM_FUMO_1+i].active = 0 ;
	}
	return;
}

void
set_alarm (struct S_scenari *s)
{
	int i;
	int alm_priority ;
	int alm_idx;

	// test se allarme attuale ancora attivo
	if ( s->current_active != -1 )
	{
		if (s->alarms[s->current_active].active == 0 || s->alarms[s->current_active].enable == 0)
		{
			switch ( s->current_active )
			{
			    case ALM_SOS_SOCCORSO:
				clear_soccorso_alarm(s) ;
				break;
			    case ALM_SOS_INCENDIO:
				clear_incendio_alarm(s) ;
				break;
			    case ALM_SOS_POLIZIA:
				clear_polizia_alarm(s) ;
				break;
			    case ALM_SOS_GUASTO:
				clear_guasto_alarm(s) ;
				break;
			    case ALM_SOS_ESTINTORE:
				clear_estintore_alarm(s) ;
				break;
			}
			s->current_active = -1;
			s->current_priority = 0 ;
		}
	}
	
	check_guasto_alarm(s);
	check_soccorso_alarm(s);
	check_estintore_alarm (s);
	check_incendio_alarm (s);
	check_polizia_alarm (s);
	
	//GESTIONE PRIORITA' ALLARMI
	alm_idx = -1 ;
	alm_priority = s->current_priority ;
	for ( i=0; i<MAX_SCENARIO_ALARMS; i++)
	{
		if ( s->alarms[i].active && s->alarms[i].enable && s->alarms[i].priority > alm_priority )
		{
			alm_idx = i;
			alm_priority = s->alarms[i].priority ;
		}
	}
	
	// attiva nuovo allarme
	if (((alm_priority != s->current_priority) || (alm_idx != s->current_active)) && (alm_idx != -1))	// nuovo allarme con priorita' maggiore
	{
		s->current_priority = alm_priority ;
		s->current_active = alm_idx ;
		s->alarm_change = 1 ;
	} else {
		s->alarm_change = 0 ;
	}
		
	// Gestione proposta nuovi allarmi
	if ( s->current_active == -1 )
	{
		s->ill_scenario_proposto = 0;
		s->vent_scenario_proposto = 0;
		s->traffico_scenario_proposto = 0;
		s->alarm_bit = 0;
		s->ritardo_att = 90 ;
		s->vent_confermato = 0;
		s->ill_confermato = 0;
		s->traff_confermato = 0;
	}
	
	if (s->alarm_change == 1 && s->current_active != -1 )
	{
		s->ill_scenario_proposto = s->alarms[s->current_active].scen_illuminazione ;
		s->ill_confermato = s->alarms[s->current_active].immediato_illuminazione;
		s->ill_annullato = 0 ;
		s->vent_scenario_proposto = s->alarms[s->current_active].scen_ventilazione ;
		s->vent_confermato = s->alarms[s->current_active].immediato_ventilazione;
		s->vent_annullato = 0 ;
		s->traffico_scenario_proposto = s->alarms[s->current_active].scen_traffico ;
		s->traff_confermato = s->alarms[s->current_active].immediato_traffico;
		s->traff_annullato = 0 ;
		s->alarm_bit = 1;
		s->ritardo_att = 90 ;
	}
	
	if ( read_data (PULSE_1SEC_ADDR) && s->alarm_bit != 0 && s->ritardo_att > 0 )
	{
		s->ritardo_att--;
	}

	///////////
	if ( (s->vent_conferma_scenario != 0 || (s->ritardo_att == 0 && s->vent_annullato == 0)) && s->current_active != -1 )
	{
		//Attivo lo scenario proposto
		//s->vent_scenario = s->vent_scenario_proposto;
		s->vent_confermato = 1 ;
		s->vent_annullato = 0 ;
	}	

	if ( (s->ill_conferma_scenario != 0 || (s->ritardo_att == 0 && s->ill_annullato == 0)) && s->current_active != -1 )
	{
		//Attivo lo scenario proposto
		//s->ill_scenario = s->ill_scenario_proposto;
		s->ill_confermato = 1 ;
		s->ill_annullato = 0 ;
	}
	
	
	if ( (s->traff_conferma_scenario != 0 || (s->ritardo_att == 0 && s->traff_annullato == 0)) && s->current_active != -1 )
	{
		//Attivo lo scenario proposto
		//s->traffico_scenario = s->traffico_scenario_proposto;
		s->traff_confermato = 1 ;
		s->traff_annullato = 0 ;
	}
	
	if ( s->vent_annulla_scenario && s->current_active != -1 )
	{
		//Attivo lo scenario proposto
		s->vent_confermato = 0 ;
		s->vent_annullato = 1 ;
	}	
	if ( s->ill_annulla_scenario && s->current_active != -1 )
	{
		//Attivo lo scenario proposto
		s->ill_confermato = 0 ;
		s->ill_annullato = 1 ;
	}	
	
	if ( s->traff_annulla_scenario && s->current_active != -1 )
	{
		//Attivo lo scenario proposto
		s->traff_confermato = 0 ;
		s->traff_annullato = 1 ;
	}	
	
	if (s->annulla_scenario != 0 && s->current_active != -1)
	{
		//Annullo lo scenario di allarme
		s->alarms[s->current_active].active = 0 ;
		s->alarm_bit = 0;
		s->vent_confermato = 0;
		s->ill_confermato = 0;
		s->traff_confermato = 0;
	}

	return;
}

void
init_gestione_scenari(void)
{
    int i ;
    
    scenari.current_active = -1 ;
    scenari.current_priority = 0 ;
    scenari.alarm_change = 0 ;
    scenari.alarm_bit = 0 ;
    scenari.ill_scenario_proposto = 0 ;
    scenari.traffico_scenario_proposto = 0 ;
    scenari.vent_scenario_proposto = 0 ;
    scenari.ill_scenario = 0 ;
    scenari.vent_scenario = 0 ;
    scenari.traffico_scenario = 0 ;
    scenari.confermato = 0 ;
    
    for ( i=0; i<MAX_SCENARIO_ALARMS; i++)
    {
	scenari.alarms[i].active = 0 ;
    }

    return ;
}

int
gestione_scenari (void)
{
	int i ;
	read_dati_scenari(&scenari);

	set_alarm(&scenari);
	
	scenari.cartello_merci_timeout = decount1s_time(scenari.cartello_merci_timeout) ;
	if (scenari.cartello_merci_timeout == 0 ) 
	{
	    for(i=0;i<SOS_NUM;i++)
	    {
		
scenari.sos[i].merci_pericolose = 0 ;
	    }
	    write_data( CARTELLO_MERCI, 0) ;
	}

	scenari.cartello_incidente_timeout = decount1s_time(scenari.cartello_incidente_timeout) ;
	if (scenari.cartello_incidente_timeout == 0 ) 
	{
	    for(i=0;i<SOS_NUM;i++)
	    {
		
scenari.sos[i].incidente = 0 ;
	    }
	    write_data( CARTELLO_INCIDENTE, 0) ;
	}
	
	write_dati_scenari(&scenari);
	return 0;
}

static unsigned long PWR_NORM( long reg )
{
    unsigned long pwr;
    pwr = read_data (reg+1);
    pwr += (read_data (reg))<<16;
    pwr = pwr / 100000L ;	
    return pwr ;
}

static unsigned long KW_NORM( long reg )
{
    unsigned long pwr;
    pwr = read_data (reg+1);
    pwr += (read_data (reg))<<16;
    pwr = pwr / 100L ;	
    return pwr ;
}

int
gestione_servizio (void)
{
    unsigned long kwh;
	

    // NORMALIZZAZIONE CONTATORI POTENZE 
    write_data (CT_N_PA_S1, PWR_NORM(QB_N_PA_S1));
    write_data (CT_N_PR_S1, PWR_NORM(QB_N_PR_S1));
    
    write_data (CT_N_PA_S2, PWR_NORM(QB_N_PA_S2));
    write_data (CT_N_PR_S2, PWR_NORM(QB_N_PR_S2));
    
    write_data (CT_N_PA_S3, PWR_NORM(QB_N_PA_S3));
    write_data (CT_N_PR_S3, PWR_NORM(QB_N_PR_S3));
    
    write_data (CT_S_PA_S2, PWR_NORM(QB_S_PA_S2));
    write_data (CT_S_PR_S2, PWR_NORM(QB_S_PR_S2));
    
    write_data (CT_S_PA_S3, PWR_NORM(QB_S_PA_S3));
    write_data (CT_S_PR_S3, PWR_NORM(QB_S_PR_S3));
    
	
	
    // gestione contatore KWh con conteggio impulsi 
    kwh = KW_NORM(QB_N_EA_S1);
    write_data (CT_N_EA_S1, kwh);
    write_data (CT_N_EA_S1+1, kwh>>16);

    kwh = KW_NORM(QB_N_ER_S1);
    write_data (CT_N_ER_S1, kwh);
    write_data (CT_N_ER_S1+1, kwh>>16);


    kwh = KW_NORM(QB_N_EA_S2);
    write_data (CT_N_EA_S2, kwh);
    write_data (CT_N_EA_S2+1, kwh>>16);

    kwh = KW_NORM(QB_N_ER_S2);
    write_data (CT_N_ER_S2, kwh);
    write_data (CT_N_ER_S2+1, kwh>>16);

    kwh = KW_NORM(QB_N_EA_S3);
    write_data (CT_N_EA_S3, kwh);
    write_data (CT_N_EA_S3+1, kwh>>16);

    kwh = KW_NORM(QB_N_ER_S3);
    write_data (CT_N_ER_S3, kwh);
    write_data (CT_N_ER_S3+1, kwh>>16);

    kwh = KW_NORM(QB_S_EA_S2);
    write_data (CT_S_EA_S2, kwh);
    write_data (CT_S_EA_S2+1, kwh>>16);

    kwh = KW_NORM(QB_S_ER_S2);
    write_data (CT_S_ER_S2, kwh);
    write_data (CT_S_ER_S2+1, kwh>>16);

    kwh = KW_NORM(QB_S_EA_S3);
    write_data (CT_S_EA_S3, kwh);
    write_data (CT_S_EA_S3+1, kwh>>16);

    kwh = KW_NORM(QB_S_ER_S3);
    write_data (CT_S_ER_S3, kwh);
    write_data (CT_S_ER_S3+1, kwh>>16);


    return 0;

}
