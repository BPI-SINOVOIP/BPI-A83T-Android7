#ifndef _AW_ALG_H_
#define _AW_ALG_H_

#include "AW_Types.h"
#include <string.h>

namespace allwinner {
    namespace Alg {
        template <typename T> inline void Swap(T &a, T &b)
        {  T temp(a); a = b; b = temp; }


        // ***** min/max are not implemented in Visual Studio 6 standard STL
        template <typename T> inline const T Min(const T a, const T b)
        { return (a < b) ? a : b; }
        template <typename T> inline const T Max(const T a, const T b)
        { return (b < a) ? a : b; }
        template <typename T> inline const T Clamp(const T v, const T minVal, const T maxVal)
        { return Max<T>(minVal, Min<T>(v, maxVal)); }

        template <typename T> inline int     Chop(T f)
        { return (int)f; }

        template <typename T> inline T       Lerp(T a, T b, T f)
        { return (b - a) * f + a; }


        // These functions stand to fix a stupid VC++ warning (with /Wp64 on):
        // "warning C4267: 'argument' : conversion from 'size_t' to 'const unsigned', possible loss of data"
        // Use these functions instead of gmin/gmax if the argument has size
        // of the pointer to avoid the warning. Though, functionally they are
        // absolutelly the same as regular gmin/gmax.
        template <typename T>   inline const T PMin(const T a, const T b)
        {
            OVR_COMPILER_ASSERT(sizeof(T) == sizeof(UPInt));
            return (a < b) ? a : b;
        }
        template <typename T>   inline const T PMax(const T a, const T b)
        {
            OVR_COMPILER_ASSERT(sizeof(T) == sizeof(UPInt));
            return (b < a) ? a : b;
        }
        template <typename T>   inline const T Abs(const T v)
        { return (v>=0) ? v : -v; }

         //-----------------------------------------------------------------------------------
         // ***** Median
         // Returns a median value of the input array.
         // Caveats: partially sorts the array, returns a reference to the array element
         // TBD: This needs to be optimized and generalized
         //
         template<class Array>
         typename Array::ValueType& Median(Array& arr)
         {
             UPInt count = arr.GetSize();
             UPInt mid = (count - 1) / 2;
             OVR_ASSERT(count > 0);

             for (UPInt j = 0; j <= mid; j++)
             {
                 UPInt min = j;
                 for (UPInt k = j + 1; k < count; k++)
                     if (arr[k] < arr[min])
                         min = k;
                 Swap(arr[j], arr[min]);
             }
             return arr[mid];
         }

  }//namespace Alg
}//namespace allwinner
#endif
