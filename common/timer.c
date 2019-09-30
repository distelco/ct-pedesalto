//#undef DEBUG

#include <stdio.h>
#include <sys/time.h>

#include "common.h"
#include "timer.h"
#include "mymemory.h"

/* timestamp e' espresso in millisecondi*/
unsigned long get_time(unsigned long *tt)
{
	struct timeval temp;
	unsigned long t ;
	gettimeofday(&temp, NULL);
	t = (temp.tv_sec * 1000) + (temp.tv_usec / 1000);
	if ( tt )
		*tt = t ;
	return t;
}


int timer_init (struct S_timer *tm, short *signal, unsigned long time_th)
{
	struct timeval t;

	if (tm == NULL)
	{
		ERR_LOG ("Bad pointer");
		return -1;
	}

	if (time_th < 0)
	{
		ERR_LOG ("Bad time threshold");
		return -1;
	}
	tm->actual = signal;
	tm->precedent = -1;
	tm->counter = 0;
	tm->time_th = time_th;
	tm->detect = 0;
	gettimeofday (&t, NULL);
	tm->timestamp = (unsigned long) t.tv_sec * 1000 + t.tv_usec / 1000;
	return 0;
}

int timer_state (struct S_timer *tm)
{
	struct timeval t;
	unsigned long time;

	if (tm == NULL)
	{
		//ERR_LOG("Bad box pointer");
		return -1;
	}

	gettimeofday (&t, NULL);
	// trasformo il tempo in millisecondi
	time = t.tv_sec * 1000 + t.tv_usec / 1000;

	if ((time - tm->timestamp) >= (unsigned long) tm->time_th)
	{
		tm->timestamp = time;
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned long count1s_time(unsigned long actual)
{
	if( read_data(PULSE_1SEC_ADDR) != 0)
		actual++;

	return actual;
}

unsigned long decount1s_time(unsigned long actual)
{
	if( read_data(PULSE_1SEC_ADDR) != 0 && actual != 0)
		actual--;
		
	return actual;
}

