#ifndef __driver_h
#define __driver_h

#include "stm32f10x.h"
#include "delay.h"

unsigned char icm20948_init(void);

unsigned char icm20948_get(short *buf, const int length);


#endif
