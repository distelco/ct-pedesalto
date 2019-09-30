#if !defined( __EXTERN_H__ )

#define __EXTERN_H__ 

extern int th_count 
#if defined(extern)
= 0;
#else
;
#endif

extern char savefile[256];
extern char savedefault[256];

extern int high_prioriry;
extern char scriptname[256];

extern char configfile[256] 
#if defined(extern)
= "/etc/plc.conf";
#else
;
#endif

extern char pidpath[256]
#if defined(extern) 
= "/var/run/";
#else
;
#endif

extern char memorypath[256]
#if defined(extern) 
= "/etc/";
#else
;
#endif

extern char dig_mem_file[512];
extern char an_mem_file[512];

extern int end_flag
# if defined(extern)
=0;
#else
;
#endif


#endif
