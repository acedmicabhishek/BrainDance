#ifndef TYPES_H
#define TYPES_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#if !defined(__cplusplus) &&                                                   \
    (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L)
typedef unsigned char bool;
#define true 1
#define false 0
#endif

#endif