#if ! defined(__DEV_MGR_H__)

#define __DEV_MGR_H__

#include "devices.h"

#define IOMGR_RO	1
#define IOMGR_RW	0
int device_manager(void);
int io_manager(int read_only);
#endif
