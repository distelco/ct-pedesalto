/*
	illumination.c
	Contiene la gestione dell'illuminazione per il ct
	Written By Franco Novi
	Distelco s.r.l.
	$Id: illumination.c,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#include <stdio.h>

#include "common.h"
#include "illumination.h"
#include "mymemory.h"
#include "timer.h"

//#define DEBUG

static struct S_regulator reg[REGULATOR_NUM];

static struct S_illumination ill;

// orario di luce ridotta notturna
static int light_off_time[] = {
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23
};
static int light_on_time[] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};


// calendario ora solare
static int sun_rise[] = {
	8, 8, 7, 7, 6, 5, 5, 6, 6, 7, 7, 8
};
static int sun_set[] = {
	17, 17, 18, 19, 19, 20, 20, 19, 18, 17, 17, 17
};


static WORD
get_op_max_value (void)
{
	return (reg[0].opacimetro_in > reg[1].opacimetro_in ? reg[0].opacimetro_in : reg[1].opacimetro_in);
}

static int
read_ill_data (struct S_illumination *pill, struct S_regulator *preg)
{
	int i, j;
	short sval ;

	if (preg == NULL)
	{
		ERR_LOG ("Bad regulator pointer");
		return -1;
	}

	for (i = 0; i < REGULATOR_NUM; i++)
	{
		preg[i].int_generale = read_data (ILL_GENERALE_BASE + i * ILL_GENERALE_OFFSET);	//comando interruttore generale scattato
		preg[i].bypass_cmd = read_data (ILL_BYPASS_BASE + i * ILL_BYPASS_OFFSET);	//contatto comando bypass
		preg[i].luce_max_in = read_data (ILL_LUCE_MAX_IN_BASE + i * ILL_LUCE_MAX_IN_OFFSET);	//contatto segnalazione luce massima
		preg[i].luce_rid_in = read_data (ILL_LUCE_RID_IN_BASE + i * ILL_LUCE_RID_IN_OFFSET);	//contatto segnalazione luce ridotta
		preg[i].luce_max_cmd = read_data (ILL_LUCE_MAX_CMD_BASE + i * ILL_LUCE_MAX_CMD_OFFSET);	//comando forzatura luce massima
		preg[i].luce_esterna_out = read_data (ILL_LUCE_OUT_BASE + i);	//segnale di uscita al regolatore  
		preg[i].livello_potenza = read_data (ILL_LIV_POT_BASE + i);	//livello potenza del regolatore              
	}
        


	for (i = 0; i < ILL_SCENE_NUM; i++)
	{
		for (j = 0; j < REGULATOR_NUM; j++)
		{
			pill->scenario[i].livello_luce[j] = read_data (ILL_SCENE_DATA_BASE + i * ILL_SCENE_DATA_STEP + j);	//dati scenari
		}
		for (j = 0; j < ILL_TELERUTTORI; j++)
		{
			pill->scenario[i].teleruttori[j] = read_data (ILL_SCENE_DATA_BASE + i * ILL_SCENE_DATA_STEP + 3 + j);	//dati scenari
		}
	}


	if (read_data (PULSE_10SEC_ADDR) > 0)
	{
                sval = (short)scale4_20(read_data (AN_LUCE_EST_IN_BASE_I));  // data is 16 bit signed
                memmove( &(preg[0].luce_esterna_data[1]), &(preg[0].luce_esterna_data[0]), 9*sizeof(long)) ;
                preg[0].luce_esterna_data[0] = sval ;
	}
	for ( j=0,i=0;i<10;i++)
		j += preg[0].luce_esterna_data[i] ;
	preg[0].luce_esterna_in = j / 10 ;

	if (read_data (PULSE_10SEC_ADDR) > 0)
	{
                sval = (short)scale4_20(read_data (AN_LUCE_EST_IN_BASE_I + 1));
                memmove( &(preg[1].luce_esterna_data[1]), &(preg[1].luce_esterna_data[0]), 9*sizeof(long)) ;
                preg[1].luce_esterna_data[0] = sval ;
	}
	for ( j=0,i=0;i<10;i++)
		j += preg[1].luce_esterna_data[i] ;
	preg[1].luce_esterna_in = j / 10 ;

        // verifica sonda difettosa
	if (preg[0].luce_esterna_in <= 1000)
	{
		preg[0].luce_esterna_in = 32767;
	}

	if (preg[1].luce_esterna_in <= 1000)
	{
		preg[1].luce_esterna_in = 32767;
	}

	// regolatore permanenti (regolati a valore fisso)
	preg[2].luce_esterna_in = 0x7fff;

	for (i = 0; i < ILL_TELERUTTORI; i++)
	{
		pill->teleruttori[i] = read_data (ILL_TELERUTTORI_BASE + (i));
		pill->semiauto_chiusura[i] = read_data (ILL_SEMIAUTO_CHIUSURA + (i));
		pill->trun_m_count[i] = read_data(ILL_MINUTI_BASE + i);
		pill->trun_h_count[i] = read_data(ILL_ORE_BASE + i);
	}


	preg[0].soglia_1 = read_data( ILL_SOGLIA_1 );
	preg[0].soglia_2 = read_data( ILL_SOGLIA_2 );
	preg[0].soglia_3 = read_data( ILL_SOGLIA_3 );
	preg[0].soglia_4 = read_data( ILL_SOGLIA_4 );

	preg[1].soglia_1 = read_data( ILL_SOGLIA_1 );
	preg[1].soglia_2 = read_data( ILL_SOGLIA_2 );
	preg[1].soglia_3 = read_data( ILL_SOGLIA_3 );
	preg[1].soglia_4 = read_data( ILL_SOGLIA_4 );
	
	pill->active_scene = read_data (ILL_SCENE_ACTIVE);
	pill->modo_operativo = read_data (ILL_MODO_OPERATIVO);

	return 0;
}

static int
write_ill_data (struct S_illumination *pill, struct S_regulator *preg)
{
	int i;

	for (i = 0; i < REGULATOR_NUM; i++)
	{
		write_data (ILL_LUCE_OUT_BASE + i, preg[i].luce_esterna_out);
                write_data(AN_LUCE_EST_IN_BASE + i, preg[i].luce_esterna_in);
		write_data ((ILL_LUCE_MAX_CMD_BASE + (i * ILL_LUCE_MAX_CMD_OFFSET)), preg[i].luce_max_cmd);
	}
	for (i = 0; i < ILL_TELERUTTORI; i++)
	{
		write_data (ILL_TELERUTTORI_BASE + (i), pill->teleruttori[i]);
		write_data (ILL_MINUTI_BASE + i, pill->trun_m_count[i]) ; 
		write_data (ILL_ORE_BASE + i, pill->trun_h_count[i]) ;
	}

	return 0;
}

#define HIST5(A)	(A+((A)*5/100))
static int
update_regulator (struct S_illumination *pill, struct S_regulator *preg)
{

	if (preg == NULL)
	{
		ERR_LOG ("Bad regulator pointer");
		return -1;
	}
	
	
	switch ( preg->rinf_lev )
	{
	 case 0:
		if ( preg->luce_esterna_in > HIST5(preg->soglia_1) )
		{
			preg->rinf_lev = 1 ;
		}
	 	break ;
		
	 case 1:
		if ( preg->luce_esterna_in < preg->soglia_1 )
		{
			preg->rinf_lev = 0 ;
		}
		if ( preg->luce_esterna_in > HIST5(preg->soglia_2) )
		{
			preg->rinf_lev = 2 ;
		}
		break ;
		
	 case 2:
		if ( preg->luce_esterna_in < preg->soglia_2 )
		{
			preg->rinf_lev = 1 ;
		}
		if ( preg->luce_esterna_in > HIST5(preg->soglia_3) )
		{
			preg->rinf_lev = 3 ;
		}
	 	break ;
		
	 case 3:
		if ( preg->luce_esterna_in < preg->soglia_3 )
		{
			preg->rinf_lev = 2 ;
		}
		if ( preg->luce_esterna_in > HIST5(preg->soglia_4) )
		{
			preg->rinf_lev = 4 ;
		}
	 	break ;
		
	 case 4:
		if ( preg->luce_esterna_in < preg->soglia_4 )
		{
			preg->rinf_lev = 3 ;
		}
		break;
	 default:	
	 	preg->rinf_lev = 0 ;
		break ;
	}
	
	
	if ( preg->rinf_lev == 4 )
	{
		preg->luce_esterna_out = 32767;
	}
	else if (  preg->rinf_lev == 3 )
	{
	    if ( preg->soglia_4 > preg->soglia_3 )
		preg->luce_esterna_out = (preg->luce_esterna_in - preg->soglia_3) * (32767-6500) / (preg->soglia_4 - preg->soglia_3) + 6500 ;
	    else
		preg->luce_esterna_out = 32767;		
	}
	else if (  preg->rinf_lev == 2 )
	{
	    if ( preg->soglia_3 > preg->soglia_2 )
		preg->luce_esterna_out = (preg->luce_esterna_in - preg->soglia_2) * (32767-6500) / (preg->soglia_3 - preg->soglia_2) + 6500 ;
	    else
		preg->luce_esterna_out = 32767;		
	}
	else if (  preg->rinf_lev == 1 )
	{
	    if ( preg->soglia_2 > preg->soglia_1 )
		preg->luce_esterna_out = (preg->luce_esterna_in - preg->soglia_1) * (32767-6500) / (preg->soglia_2 - preg->soglia_1) + 6500 ;
	    else
		preg->luce_esterna_out = 32767;		
	}
	else
	{
		preg->luce_esterna_out = 32767 ;
	}
	return 0;
}


int
illuminazione_process (void)
{
	int i;
	time_t tt;
	struct tm pt;

	read_ill_data (&ill, &reg[0]);
/*
	if (ill.modo_operativo == ILL_MODE_SEMIAUTO)
	{
		for (i = 0; i < ILL_TELERUTTORI; i++)
		{
			ill.teleruttori[i] = ill.semiauto_chiusura[i];
		}

	}
	else
*/
        if (ill.active_scene == ILL_SCENE_AUTO)
	{
		// accende tutto
		for (i = 0; i < ILL_TELERUTTORI; i++)
		{
			ill.teleruttori[i] = 1;
		}

		// gestione regolatori di tensione 
		for (i = 0; i < REGULATOR_NUM; i++)
		{
			update_regulator (&ill, &reg[i]);
		}
		
		// update teleruttori rinforzo [0]
		switch ( reg[0].rinf_lev )
		{
		 case 0:
			ill.teleruttori[ILL_R01] = 0;
			ill.teleruttori[ILL_R02] = 0;
			ill.teleruttori[ILL_R03] = 0;
			break ;
		 case 1:
			ill.teleruttori[ILL_R01] = 0;
			ill.teleruttori[ILL_R02] = 1;
			ill.teleruttori[ILL_R03] = 0;
			break ;
		 case 2:
			ill.teleruttori[ILL_R01] = 1;
			ill.teleruttori[ILL_R02] = 1;
			ill.teleruttori[ILL_R03] = 0;
			break ;
		 case 3:
			ill.teleruttori[ILL_R01] = 1;
			ill.teleruttori[ILL_R02] = 1;
			ill.teleruttori[ILL_R03] = 1;
			break ;
		 case 4:
			ill.teleruttori[ILL_R01] = 1;
			ill.teleruttori[ILL_R02] = 1;
			ill.teleruttori[ILL_R03] = 1;
			break ;
		}	

		// update teleruttori rinforzo [0]
		switch ( reg[1].rinf_lev )
		{
		 case 0:
			ill.teleruttori[ILL_R04] = 0;
			ill.teleruttori[ILL_R05] = 0;
			ill.teleruttori[ILL_R06] = 0;
			break ;
		 case 1:
			ill.teleruttori[ILL_R04] = 0;
			ill.teleruttori[ILL_R05] = 1;
			ill.teleruttori[ILL_R06] = 0;
			break ;
		 case 2:
			ill.teleruttori[ILL_R04] = 0;
			ill.teleruttori[ILL_R05] = 1;
			ill.teleruttori[ILL_R06] = 1;
			break ;
		 case 3:
			ill.teleruttori[ILL_R04] = 1;
			ill.teleruttori[ILL_R05] = 1;
			ill.teleruttori[ILL_R06] = 1;
			break ;
		 case 4:
			ill.teleruttori[ILL_R04] = 1;
			ill.teleruttori[ILL_R05] = 1;
			ill.teleruttori[ILL_R06] = 1;
			break ;
		}	

		// controllo orario dei rinforzi
		tt = time (NULL);
		localtime_r (&tt, &pt);
		if ((pt.tm_hour <= light_on_time[pt.tm_mon]) || (pt.tm_hour >= light_off_time[pt.tm_mon]))
		{
			// spegne i rinforzi
			reg[2].luce_esterna_in = 7000;
			ill.teleruttori[ILL_R01] = 0;
			ill.teleruttori[ILL_R02] = 0;
			ill.teleruttori[ILL_R03] = 0;
			ill.teleruttori[ILL_R04] = 0;
			ill.teleruttori[ILL_R05] = 0;
			ill.teleruttori[ILL_R06] = 0;

		}

		//
		for (i = 0; i < ILL_TELERUTTORI; i++)
		{
			ill.semiauto_chiusura[i] = ill.teleruttori[i];
		}

	}
	else
	{			// scenario attivo
		for (i = 0; i < REGULATOR_NUM; i++)
		{
			// override input luminance
			//reg[i].luce_esterna_in = ill.scenario[ill.active_scene].livello_luce[i];
			reg[i].luce_max_cmd = 0;
			reg[i].luce_esterna_out = ill.scenario[ill.active_scene].livello_luce[i];
			if (reg[i].luce_esterna_out > 32700)
		    	{
				reg[i].luce_esterna_out = 32767;
				reg[i].luce_max_cmd = 1;
			}
			else
			{
				reg[i].luce_max_cmd = 0;
			}
		}

		for (i = 0; i < ILL_TELERUTTORI; i++)
		{
			ill.teleruttori[i] = ill.scenario[ill.active_scene].teleruttori[i];
		}

	}


	// calcolo ore funzionamento
	if (read_data (PULSE_60SEC_ADDR) > 0)
	{
		for (i = 0; i < ILL_TELERUTTORI; i++)
		{
			//aggiorno contatore minuti di funzionamento
			if (ill.teleruttori[i] != 0)
			{
				(ill.trun_m_count[i])++;
			}
			//aggiorno contatore ore di funzionamento
			if ( ill.trun_m_count[i] == 60 )
			{
				ill.trun_m_count[i] = 0 ;
				(ill.trun_h_count[i])++ ;
			}
	    	}
	}

	write_ill_data (&ill, &reg[0]);

	return 0;
}

void 
init_illuminazione_data(void)
{
//    ill.modo_operativo = ILL_MODO_LOCALE ;
    ill.active_scene = ILL_SCENE_AUTO ;
    return ;
}
