#ifndef _AW_TYPES_H_
#define _AW_TYPES_H_

#include    <stddef.h>
#include    <limits.h>
#include    <float.h>

#include    <sys/types.h>

namespace allwinner{
typedef char            Char;

// Pointer-sized integer
typedef size_t          UPInt;
typedef ptrdiff_t       SPInt;

typedef int8_t          SByte;
typedef uint8_t         UByte;
typedef int16_t         SInt16;
typedef uint16_t        UInt16;
typedef int32_t         SInt32;
typedef uint32_t        UInt32;
typedef int64_t         SInt64;
typedef uint64_t        UInt64;

namespace BaseTypes
{
    using allwinner::UPInt;
    using allwinner::SPInt;
    using allwinner::UByte;
    using allwinner::SByte;
    using allwinner::UInt16;
    using allwinner::SInt16;
    using allwinner::UInt32;
    using allwinner::SInt32;
    using allwinner::UInt64;
    using allwinner::SInt64;
}

#define OVR_DEBUG_BREAK  ((void)0)
#define OVR_ASSERT(p)    ((void)0)
#define OVR_COMPILER_ASSERT(x)  { int zero = 0; switch(zero) {case 0: case x:;} }

} //namespace allwinner


#endif //_AW_TYPES_H_
