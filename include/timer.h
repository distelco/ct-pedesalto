#if !defined (__ISTERESI_H__)

#define __ISTERESI_H__


struct S_timer
{
	short *actual;		/* valore attuale segnale di riferimento */
	int precedent;		/* valore precedente segnale di riferimento */
	int counter;		/* contatore numero di cicli in cui il segnale e' costante */
	unsigned long time_th;	/* soglia di settaggio detect */
	int detect;		/* flag rilevazione allarme */
	unsigned long timestamp;
};

int timer_init (struct S_timer *, short *, unsigned long);
int timer_state (struct S_timer *);

/*
	ritorna il tempo assoluto in millisecondi
*/
unsigned long get_time(unsigned long *tt) ;


/*
	Incrementa il counter actual ad ogni secondo tramite PULSE_SEC_ADDR
*/

#define PULSE_1SEC_ADDR		CS(16)
#define PULSE_10SEC_ADDR	CS(17)
#define PULSE_30SEC_ADDR	CS(18)
#define PULSE_60SEC_ADDR	CS(19)

unsigned long count1s_time(unsigned long actual);
unsigned long decount1s_time(unsigned long actual);

#endif
