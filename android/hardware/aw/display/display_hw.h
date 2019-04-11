#ifndef _DISPLAY_HW_H_
#define _DISPLAY_HW_H_

#define astar   (1)
#define kylin   (2)
#define octopus (3)
#define eagle   (4)
#define neptune (5)
#define uranus  (6)

#define DISPLAY_FUNC_OFF_MASK (0x00)
#define DISPLAY_FUNC_ON_MASK (0x01)
#define DISPLAY_FUNC_DEMO_ON_MASK (0x03)

/*Enhance*/
#if  (TARGET_BOARD_PLATFORM == octopus \
    || TARGET_BOARD_PLATFORM == eagle)
#define SUPPORT_DE_NUM (1)
#define DISPLAY_ENHANCE_OFF ((char*)"0")
#define DISPLAY_ENHANCE_ON ((char*)"1")
#define DISPLAY_ENHANCE_DEMO_ON ((char*)"3")

#elif (TARGET_BOARD_PLATFORM == neptune \
     || TARGET_BOARD_PLATFORM == uranus)
#define SUPPORT_DE_NUM (2)
#define DISPLAY_ENHANCE_OFF ((char*)"0")
#define DISPLAY_ENHANCE_ON ((char*)"1")
#define DISPLAY_ENHANCE_DEMO_ON ((char*)"2")
#endif

/*Reading Mode & Color Temperature*/
#if (TARGET_BOARD_PLATFORM == neptune \
     || TARGET_BOARD_PLATFORM == uranus)
#define READING_MODE_STRENGTH_MIN (10)
#define RM_STRENGTH_OFFSET (50)
#endif


#endif
