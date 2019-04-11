#ifndef  HAL_PUBLIC_H
#define  HAL_PUBLIC_H

#define astar	1
#define kylin	2
#define octopus 3
#define eagle   4
#define neptune 5
#define uranus  6

#if (TARGET_BOARD_PLATFORM == astar \
     || TARGET_BOARD_PLATFORM == tulip)
#include "hal_public/hal_mali_utgard.h"
#elif (TARGET_BOARD_PLATFORM == neptune \
     || TARGET_BOARD_PLATFORM == uranus)
#include "hal_public/hal_mali_midgard.h"
#elif (TARGET_BOARD_PLATFORM == octopus \
     || TARGET_BOARD_PLATFORM == eagle)
#include "hal_public/hal_img_sgx544.h"
#elif (TARGET_BOARD_PLATFORM == kylin)
#include "hal_public/hal_img_rgx6230.h"
#else
#error "please select a platform\n"
#endif

#endif
