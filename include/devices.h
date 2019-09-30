
#if !defined(__DEVICES_H__)

#define __DEVICES_H__

#include <stdio.h>

#define AREA_DATA	256

struct S_driver_io_block {
	int 	base;		// address in memory area
	int	lenght;		// data length
	char	name[64];
	long	did;		// driver id
	int 	state;		// communication state
	void	*drv_info;	// ptr to driver data
};

#define TRANSFORM_INV	((void*)1)
#define TRANSFORM_4_20	((void*)2)
struct S_transform_io_block {
	int 	addr;
	void	*type;
};

struct S_driver_descriptor {
	char name[64];
	long did ;
	int (* read_digital)( void * data, char *buf, int buflen ); //ritorna 0 ok, != 0 error
	int (* write_digital)( void * data, char *buf, int buflen );
	int (* read_analog)( void * data, long *buf, int buflen );
	int (* write_analog)( void * data, long *buf, int buflen );
	int (* read_mem)  ( void * data, char *buf, int buflen );
	int (* write_mem) ( void * data, char *buf, int buflen );
	int (* init_block)( struct S_driver_io_block * 	 , FILE* );
	int (* init_drv)  ( struct S_driver_descriptor * , FILE* );
	int (* flush_io)  ( void );
};

int load_driver_did( char *name, int did );
int load_driver( char*name, int did, char*fname);

inline struct S_driver_descriptor *create_drv_type(void);
inline void destroy_drv_type( struct S_driver_descriptor * pd );


#endif
