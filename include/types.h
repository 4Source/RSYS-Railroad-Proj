#ifndef TYPES_H
#define TYPES_H

#ifdef __KERNEL__
#include <linux/types.h>
typedef u16 uint16_t;
typedef u64 uint64_t;
#else
#include <stdint.h>
#endif

#endif