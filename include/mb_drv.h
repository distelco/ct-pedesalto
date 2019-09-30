/*
	mb_drv.h
	File include per l'interfaccia tra il device manager e le unita' con protocollo modbus
	Written by Franco Novi
	$Id: mb_drv.h,v 1.3 2009-01-29 10:32:55 matteoc Exp $
*/

#if !defined(__MB_DRV_H__)

#define __MB_DRV_H__

#include "comunication.h"

typedef enum {
	Coil = 1,
	Input,
	Analog,
	Register
} E_areacode;


struct S_modbus_data {
	struct S_tcp 	*tcp;
	int		slaveid ;
	E_areacode	area;
	int		starting_addr;
	int		coil_num;
	long		io_freq;	// frequenza rinfresco in cicli
	long		io_last;		
};

#define drv_remote_io_did 0x21bc
#define drv_mbtcp_io_did	0x21bc
#define drv_mbrtutun_io_did	0x34ae
#define drv_mbrtu_io_did	0x349d

#endif
