/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _TOOLS_ENDIAN_H
#define _TOOLS_ENDIAN_H

#include "fixedint.h"

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;

# define __bswap_constant_64(x) \
     ((((x) & 0xff00000000000000ull) >> 56)                                   \
      | (((x) & 0x00ff000000000000ull) >> 40)                                 \
      | (((x) & 0x0000ff0000000000ull) >> 24)                                 \
      | (((x) & 0x000000ff00000000ull) >> 8)                                  \
      | (((x) & 0x00000000ff000000ull) << 8)                                  \
      | (((x) & 0x0000000000ff0000ull) << 24)                                 \
      | (((x) & 0x000000000000ff00ull) << 40)                                 \
      | (((x) & 0x00000000000000ffull) << 56))

static __inline __uint64_t
__bswap_64 (__uint64_t __bsx)
{
  return __bswap_constant_64 (__bsx);
}

//#define htobe16(x) __bswap_16(x)
//#define be16toh(x) __bswap_16(x)

//#define htobe32(x) __bswap_32(x)
//#define be32toh(x) __bswap_32(x)

#define htobe64(x) __bswap_64(x)
#define be64toh(x) __bswap_64(x)

#define htole16(x) (x)
#define htole32(x) (x)
#define htole64(x) (x)

#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)

#endif /* _TOOLS_ENDIAN_H */
