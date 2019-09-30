/*
        plc.c
	main program
	written by VIP Franco Novi
	$Id: ct.c,v 1.3 2009-01-29 10:32:55 matteoc Exp $
*/

#include "../config.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <errno.h>

#define extern
#include "extern.h"
#undef extern
#include "common.h"
#include "commands.h"
#include "cfg.h"
#include "version.h"
#include "dev_mgr.h"
#include "server_modbus.h"
#include "mymemory.h"
#include "timer.h"
#include "illumination.h"
#include "scenari.h"
//#include "ax5032.h"

extern struct S_digital_memory digital_input_area;
extern struct S_digital_memory digital_output_area;
extern struct S_digital_memory digital_prev_input_area;
extern struct S_analog_memory analog_input_area;
extern struct S_analog_memory analog_prev_input_area;
extern struct S_analog_memory analog_output_area;

//#define EMULATOR

int make_daemon (void);
static int high_priority = 0;

#define SCHED_PRI_LOW	10
#define SCHED_PRI_MID	20
#define SCHED_PRI_HIGH	50

//static struct S_ax5032_data ax_1 = { 0x300 };


/*
    funzione per il cambio di priorita' del thread
*/
static void
change_priority (int priority)
{
	int th_resp;
#	if 0
	int policy;
#	endif
	struct sched_param th_schedparam;
	int th_handle;

	th_schedparam.sched_priority = priority;
	th_handle = pthread_self ();

	th_resp = pthread_setschedparam (th_handle, SCHED_RR, &th_schedparam);

	if (th_resp == EINVAL)
	{
		ERR_LOG (" SETSCHEDPARAM Livello di priorita' non valida per il processo %d", th_handle);
	}
	if (th_resp == EPERM)
	{
		ERR_LOG (" SETSCHEDPARAM Il processo %d non ha i permessi da super user", th_handle);
	}
	if (th_resp == ESRCH)
	{
		ERR_LOG (" SETSCHEDPARAM processo %d invalido o gia' terminato", th_handle);
	}
	if (th_resp == EFAULT)
	{
		ERR_LOG (" th_param punta fuori dallo spazio di  memoria del processo %d", th_handle);
	}
#if 0
	th_resp = pthread_getschedparam (pthread_self (), &policy, &th_schedparam);

	if (th_resp == ESRCH)
	{
		ERR_LOG (" GETSCHEDPARAM processo %d invalido o gia' terminato", th_handle);
	}
	if (th_resp == EFAULT)
	{
		ERR_LOG (" GETSCHEDPARAM out of memory %d", th_handle);
	}
	DEBUG_LOG ("SCHED_RR %d policy %d\n", SCHED_RR, policy);
#endif
	return;
}

static pthread_t th_dev_mgr = -1;
static pthread_t th_net = -1;
static pthread_t  th_mbs = -1;

static char pid_file[300];
/* signal catcher */
static void
closedown (int sig)
{
	/* must stop all thread */

	INFO_LOG ("closedown:Signal %d, exiting..", sig);

	if (th_net != -1)
	{
		DEBUG_LOG ("closedown:Cancelling th_net");
		pthread_cancel (th_net);
		pthread_join (th_net, NULL);
	}

	if (th_mbs != -1)
	{
		DEBUG_LOG ("closedown:Cancelling th_mbs");
		pthread_cancel (th_mbs);
		pthread_join (th_mbs, NULL);
	}

	unlink (pid_file);
	exit (0);
}

/*

*/
static void
version (char *progname)
{
	fprintf (stderr, "CT (Castei) version %d.%d.%d build %d\n", VMAJOR, VMINOR, VSUBMINOR, VBUILD);
}

/*

*/
static void
Usage (char *progname)
{
	version (progname);
	fprintf (stderr, "CT daemon v.%d.%d.%d build %d\n", VMAJOR, VMINOR, VSUBMINOR, VBUILD);
	fprintf (stderr, "Usage: %s [options] <filename>\n", progname);
	fprintf (stderr, "where options are:\n");
	fprintf (stderr, "   -a	        Set high priority scheduling\n");
	fprintf (stderr, "   -d         run in foreground\n");
	fprintf (stderr, "   -f Path    Memory Backup file absolute Path \n");
	fprintf (stderr, "   -p Path    plc.thread  dir\n");
	fprintf (stderr, "   -s Script  DiskOnChip parameters saving\n");
	fprintf (stderr, "   -v         show program version \n");
	fprintf (stderr, "   -w         disable WDT\n");
}

/*

*/
static void *
th_dev_manager (void *arg)
{
	int pid;
	// Incremento il contatore dei thread attivi
	th_count++;
	pid = getpid ();
	write_pidfilename (pidpath, "plc.2.pid", pid);

	DEBUG_LOG ("Device Manager  pid %d(LWP %ld)", pid, pthread_self ());

	if (high_priority > 0)
	{
		DEBUG_LOG ("Device Manager, high priority!");
		change_priority (SCHED_PRI_LOW);
	}
	device_manager ();

	pthread_exit (0);
}

int
real_main (int argc, char *argv[])
{
	int nodaemon = 0;
	int c;
	char filename2[30] = "plc.thread";
	char filename1[30] = "plc.6.pid";
	unsigned long t_1s = 0, t_10s = 0, t_30s = 0, t_60s = 0, now = 0, trun=0;
	time_t tt;
	struct tm t;
	struct tm *pt = &t;

	strcpy (dig_mem_file, "/dev/null");
	strcpy (an_mem_file, "/dev/null");

	while ((c = getopt (argc, argv, "awdhvf:e:s:p:")) != -1)
	{
		switch (c)
		{
		case 'h':
			Usage (argv[0]);
			exit (1);
			break;
		case 'v':
			version (argv[0]);
			exit (1);
			break;
		case 'f':
			strncpy (memorypath, optarg, 256);
			strcpy (dig_mem_file, memorypath);
			strcat (dig_mem_file, "ct_dig_mem");
			strcpy (an_mem_file, memorypath);
			strcat (an_mem_file, "ct_an_mem");
			break;
		case 's':
			strncpy (scriptname, optarg, 256);
			break;
		case 'p':
			strncpy (pidpath, optarg, 256);
			break;
		case 'w':
			break;

		case 'd':
			nodaemon = 1;
			break;
		case 'a':
			high_priority = 1;
			break;
		}
	}

	OPENLOG ();
	INFO_LOG ("CT (Castei) version %d.%d.%d build %d starting...", VMAJOR, VMINOR, VSUBMINOR, VBUILD);

	if (!nodaemon)
	{
		// go in background
		INFO_LOG ("Going in background");
		make_daemon ();
	}
	else
	{
		INFO_LOG ("Running interactive");
	}

	if (high_priority > 0)
		INFO_LOG ("High prioriry mode start");

	DEBUG_LOG ("Debug_Log attivo");

	//creo le aree di memoria
	create_memories ();

	ioperm (0x120, 2, 1);

	//creo il thread di gestione device
	pthread_create (&th_mbs, NULL, &listen_multinet_request, NULL);
	pthread_create (&th_net, NULL, &listen_multinet_command, NULL);

	//pthread_create (&th_dev_mgr, NULL, &th_dev_manager, NULL);

	usleep (200000);

	load_digital_mem (dig_mem_file, 2048);
	load_analog_mem (an_mem_file, 2048);

	// Incremento il contatore del thread attivi per tener conto del processo di gestione dei thread
	th_count++;
	//scrivo in FILEPATH/plc.thread il pid del processo principale
	snprintf (pid_file, 300, "%s/%s", pidpath, filename1);
	DEBUG_LOG ("Plc Pid %d salvato in: %s", getpid (), pid_file);
	save_runfile (pid_file);
	snprintf (pid_file, 300, "%s/%s", pidpath, filename2);
	save_threadnum (pid_file, th_count);
	DEBUG_LOG ("Num.processi attivi:%d,salvato in %s\n", th_count, pid_file);

	signal (SIGKILL, closedown);
	signal (SIGTERM, closedown);
	signal (SIGPIPE, closedown);
	signal (SIGQUIT, closedown);
	signal (SIGABRT, closedown);
	signal (SIGINT, closedown);
//      signal( SIGHUP, closedown ) ;

	//inizializzo la variabile d'appoggio per l'inpulso di un secondo
	get_time (&t_1s);
	get_time (&t_10s);
	get_time (&t_30s);
	get_time (&t_60s);

	init_fan_data();
	init_ventilazione_data ();
	init_illuminazione_data();
	init_gestione_scenari();
	
	io_manager (IOMGR_RO);

	// init WDT
	//outb (0x0a, 0x120);
	//outb (0x0b, 0x120);
	//outb (0x02, 0x120);
	//outb (0x05, 0x121);

	while (!end_flag)
	{
		// refresh WDT
		//outb (0x05, 0x121);
		//prendo la data di sistema e la scrivo negli ingressi analogici
		tt = time (NULL);
		pt = localtime (&tt);
		write_data (IR (20), pt->tm_year);
		write_data (IR (21), pt->tm_mon);
		write_data (IR (22), pt->tm_yday);
		write_data (IR (23), pt->tm_mday);
		write_data (IR (24), pt->tm_wday);
		write_data (IR (25), pt->tm_hour);
		write_data (IR (26), pt->tm_min);
		write_data (IR (27), pt->tm_sec);
		write_data (IR (28), pt->tm_isdst);

		get_time (&now);
		if (now >= (t_1s + 1000))
		{
			set_data (PULSE_1SEC_ADDR);
			t_1s = now;
		}
		else
		{
			reset_data (PULSE_1SEC_ADDR);
		}

		if (now >= (t_10s + 10000))
		{
			set_data (PULSE_10SEC_ADDR);
			t_10s = now;
		}
		else
		{
			reset_data (PULSE_10SEC_ADDR);
		}

		if (now >= (t_30s + 30000))
		{
			set_data (PULSE_30SEC_ADDR);
			t_30s = now;
		}
		else
		{
			reset_data (PULSE_30SEC_ADDR);
		}

		if (now >= (t_60s + 60000))
		{
			set_data (PULSE_60SEC_ADDR);
			t_60s = now;
		}
		else
		{
			reset_data (PULSE_60SEC_ADDR);
		}

		if (read_data (PULSE_60SEC_ADDR) > 0)
		{
			save_digital_mem (dig_mem_file, 2048);
			save_analog_mem (an_mem_file, 2048);
		}

		//ax5032_read_di( (digital_input_area.head) );
		//gestione_scenari ();
		//illuminazione_process ();
		ventilazione_process();
		fan_process();
		gestione_servizio ();

		//aggiorno ingressi e uscite
		get_time (&now);
		io_manager (IOMGR_RW);
		update_device_iostatus();

		//ax5032_write_do( (digital_output_area.head) );
		if (nodaemon)
		{
			delay (100);

		}
		else
		{
			delay (1);
		}
		get_time(&trun);
		trun = trun - now ;
		write_data(HR(3999), trun) ;
		
	}
	CLOSELOG ();
	return 0;
}
