#ifndef __RBT_TYPES_H__
#define __RBT_TYPES_H__
#include <stdint.h>
#ifndef __GNUC__

typedef signed char             int8_t;
typedef short int               int16_t;
typedef int                     int32_t;
typedef long long int           int64_t;
typedef unsigned char           uint8_t;
typedef unsigned short int      uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long int  uint64_t;

//typedef long int                int64_t;      //for 64bit
//typedef unsigned long int       int64_t;    //for 64bit

//intptr_t always has the same size with CPU
typedef int                     intptr_t;

#endif

typedef int                     status_t;
typedef unsigned int            bool_t;


#define TRUE                    1
#define FALSE                   0


#define ERROR                   -1
#define OK                      0

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif

