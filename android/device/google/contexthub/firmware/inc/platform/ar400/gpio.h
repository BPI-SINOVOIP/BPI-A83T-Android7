/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _AR400_GPIO_H_
#define _AR400_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define GPIO_HANDLE_OFFSET    1  /* to make sure that 0 stays an invalid number */


#define GPIO_PORTL 1
#define GPIO_PORTM 2

#define GPIO_PORT_SHIFT 5

/*
 * This is a shorthand to specify a GPIO by port and number.
 * Use GPIO_PA(5) for the Nucleo green LED LD2.
 */
#define GPIO_PL(x) ((GPIO_PORTL << GPIO_PORT_SHIFT) + (x))
#define GPIO_PM(x) ((GPIO_PORTM << GPIO_PORT_SHIFT) + (x))


#define GPIO_PIN_MASK 0x1f

enum ArGpioAltFunc
{
    GPIO_AF00 = 0,
    GPIO_AF01,
    GPIO_AF02,
    GPIO_AF03,
    GPIO_AF04,
    GPIO_AF05,
    GPIO_AF06,
    GPIO_AF07,
    GPIO_AF_INPUT = GPIO_AF00,
    GPIO_AF_OUTPUT = GPIO_AF01,
    GPIO_AF_EINT = GPIO_AF06,
    GPIO_AF_I2C0 = GPIO_AF03,
    GPIO_AF_I2C1 = GPIO_AF02,
    GPIO_AF_I2C2 = GPIO_AF02,
    GPIO_AF_SPI0 = GPIO_AF02,
    GPIO_AF_RSB = GPIO_AF02,
    GPIO_AF_UART = GPIO_AF02,
    GPIO_AF_JTAG = GPIO_AF02,
    GPIO_AF_PWM = GPIO_AF02,
    GPIO_AF_CPU_CUR_W = GPIO_AF03,
    GPIO_AF_PLEINT0 = GPIO_AF06,
    GPIO_AF_PLEINT1 = GPIO_AF06,
    GPIO_AF_PLEINT2 = GPIO_AF06,
    GPIO_AF_PLEINT3 = GPIO_AF06,
    GPIO_AF_PLEINT4 = GPIO_AF06,
    GPIO_AF_PLEINT5 = GPIO_AF06,
    GPIO_AF_PLEINT6 = GPIO_AF06,
    GPIO_AF_PLEINT7 = GPIO_AF06,
    GPIO_AF_PLEINT8 = GPIO_AF06,
    GPIO_AF_PLEINT9 = GPIO_AF06,
    GPIO_AF_PLEINT10 = GPIO_AF06,
    GPIO_AF_PLEINT11 = GPIO_AF06,
    GPIO_AF_PLEINT12 = GPIO_AF06,
    GPIO_AF_PMEINT0 = GPIO_AF06,
    GPIO_AF_PMEINT1 = GPIO_AF06,
    GPIO_AF_PMEINT2 = GPIO_AF06,
    GPIO_AF_PMEINT3 = GPIO_AF06,
    GPIO_AF_PMEINT4 = GPIO_AF06,
    GPIO_AF_PMEINT5 = GPIO_AF06,
};

enum ArGpioSpeed       /* CL (pF)     50,  50,   10,  10   */
{                       /* VDD (V) >=   2.7, 1.7,  2.7, 1.7 */
    GPIO_SPEED_LOW = 0, /* Max (MHz)    4,   2,    8,   4   */
    GPIO_SPEED_MEDIUM,  /*             25,  12.5, 50,  20   */
    GPIO_SPEED_FAST,    /*             50,  25,  100,  50   */
    GPIO_SPEED_HIGH,    /*            100,  50,  180, 100   */
};

void gpioBitbangedUartOut(uint32_t chr);

#ifdef __cplusplus
}
#endif

#endif
