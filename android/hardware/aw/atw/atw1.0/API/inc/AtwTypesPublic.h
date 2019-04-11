#pragma once

#include <stdint.h>
#include "AW_CAPI.h"

typedef enum
{
   left_eye = 0,
   right_eye
}eye_t;

typedef int (*PTRFUN_HeadTracker)(uint64_t timestamp, avrQuatf &pose);