/*
    Portable header to provide the 32 and 64 bits type.

    Not a compatible replacement for <stdint.h>, do not blindly use it as such.
*/
#ifndef FIXEDINT_H_INCLUDED
    #define FIXEDINT_H_INCLUDED
    
    #include <linux/types.h>

    /* (u)int32_t */
    #ifndef uint32_t
        typedef u_int32_t uint32_t;
    #endif

    /* (u)int64_t */
    typedef int64_t int64_t;
    typedef u_int64_t uint64_t;

    #define UINT64_C(v) v ##ULL
    #define INT64_C(v) v ##LL

#endif
