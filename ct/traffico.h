/*
    traffico.c
    Processo gestione semafori
    Written By Franco Novi
    Distelco s.r.l.
    $Id: traffico.h,v 1.2 2006-01-30 18:58:50 matteoc Exp $
*/

#if !defined(__TRAFFICO_H__)

#define __TRAFFICO_H__

//#define DEBUG
#include <stdio.h>
#include <string.h>


#include "common.h"
#include "mymemory.h"
#include "timer.h"

#define CT_SEM_NUM 12
#define CT_SEM_SCENE_NUM	7

#define CT_SEM_BASE	100

#define CT_SA_SEM_BASE	120

#define CT_SA_RED_BASE	132
#define CT_SA_GRN_BASE	133
#define RED_OFFSET	2
#define GRN_OFFSET	2


#define BYTE	unsigned char
#define WORD	short

#define START_CODE	0x02
#define END_CODE	0x03

enum E_sem_cmd {
	OFF_CODE,
	GRN_CODE,
	RED_CODE
};

#define ACK	0x06
#define NAC	0x15


#define CL_SEM_NUM 12

#define SEM_SCENE_AUTO	0

#define SEM_SCENE_ACTIVE_BASE	2200
#define SEM_SCENE_BASE		2201
#define SEM_SCENE_OFFSET	CL_SEM_NUM


enum E_sem_state {
	S_AUTO = 0,
	S_SEMIAUTO
};


struct S_sm_struct {
	enum E_sem_state state;
	enum E_sem_cmd out_cmd;
	BYTE red_cmd;
	BYTE grn_cmd;
};

#endif

