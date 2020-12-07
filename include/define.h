#ifndef __DEFINE_H__
#define __DEFINE_H__
#include <stdio.h>
#ifndef MIN
#define MIN(a, b)           ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a, b)           ((a)>(b)?(a):(b))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)       (sizeof(a)/sizeof(a[0]))
#endif

//sizeof member in a struct
#ifndef msizeof
#define msizeof(type,member)    sizeof(((type *)0)->member)
#endif

#ifndef htonll
#include <endian.h>
#include <byteswap.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define htonll(x)   (x)
#define ntohll(x)   (x)
#else
#define htonll(x)   __bswap_64(x)
#define ntohll(x)   __bswap_64(x)
#endif
#endif

#define swap_endian_16(x)  ((((uint16_t)(x) & 0xff00) >> 8) | \
                            (((uint16_t)(x) & 0x00ff) << 8))
#define swap_endian_32(x)  ((((uint32_t)(x) & 0xff000000) >> 24) | \
                             (((uint32_t)(x) & 0x00ff0000) >> 8) | \
                             (((uint32_t)(x) & 0x0000ff00) << 8) | \
                             (((uint32_t)(x) & 0x000000ff) << 24))
#define swap_endian_64(x)   (((uint64_t)(swap_endian_32((x)&0xffffffff))<< 32) | swap_endian_32(((x)>>32)&0xffffffff)) 


#define IN_RANGE(c, lo, hi)     (c >= lo && c <= hi)

#define SET_BIT(x,bit)          ((x)|=(1<<bit))
#define CLR_BIT(x,bit)          ((x)&=~(1<<bit))
#define CHK_BIT(x,bit)          ((x)&(1<<bit))

#define VAL(str)                #str
#define MACROTOSTRING(str)      VAL(str)

#define safe_free(p)            do { if (p != NULL) { free(p); p = NULL; } } while(0)
#define safe_fclose(p)          do { if (p != NULL) { fclose(p); p = NULL; } } while(0)
#define xml_safe_free(p)        do { if (p != NULL) { xmlFree(p); p = NULL; } } while(0)
#define xmldoc_safe_free(p)     do { if (p != NULL) { xmlFreeDoc(p); p = NULL; } } while(0)

#define ASCII_LF                0x0A
#define ASCII_CR                0x0D
#define ASCII_ESC               0x1B
#define ASCII_CTRL_C            0x03

#define FILE_NAME_MAX_LENGTH        128
#define PATH_MAX_LENGTH             256

#define safe_sprintf(dst, fmt, ...)  snprintf(dst, sizeof(dst), fmt, ## __VA_ARGS__)

#endif