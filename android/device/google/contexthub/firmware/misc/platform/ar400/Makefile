#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

M3DEBUG ?= m3debug

ifneq ($(CPU),cortexm4f)
        $(error "ar400 platform only supports Cortex-M4F CPUs")
endif

DELIVERABLES = os.checked.bin bl.unchecked.bin full.bin showsizes
BL_FILE = bl.unchecked.bin
OS_FILE = os.checked.bin

FLAGS += -I. -fno-unwind-tables -fstack-reuse=all -ffunction-sections -fdata-sections
FLAGS += -Wl,--gc-sections
FLAGS += -Iinc/platform/$(PLATFORM)/cmsis

APPFLAGS += -msingle-pic-base -mpic-register=r9 -nostartfiles -fpic -shared

#platform bootloader
SRCS_bl += src/platform/$(PLATFORM)/bl.c

#platform runtime
SRCS_os +=  src/platform/$(PLATFORM)/crt_$(PLATFORM).c

#platform drivers
SRCS_os += src/platform/$(PLATFORM)/platform.c \
	src/platform/$(PLATFORM)/gpio.c \
	src/platform/$(PLATFORM)/mpu.c \
	src/platform/$(PLATFORM)/pwr.c \
	src/platform/$(PLATFORM)/i2c.c \
	src/platform/$(PLATFORM)/spi.c \
	src/platform/$(PLATFORM)/wdt.c \
	src/platform/$(PLATFORM)/eeData.c \
	src/platform/$(PLATFORM)/intc/*.c \
	src/platform/$(PLATFORM)/cpucfg/*.c \
	src/platform/$(PLATFORM)/pin/*.c \
	src/platform/$(PLATFORM)/uart/*.c \
	src/platform/$(PLATFORM)/timer/*.c \
	src/platform/$(PLATFORM)/prcm/*.c \
	src/platform/$(PLATFORM)/hwmsgbox/*.c \
	src/platform/$(PLATFORM)/hwspinlock/*.c \
	src/platform/$(PLATFORM)/library/*.c \
	src/platform/$(PLATFORM)/pmu/*.c \
	src/platform/$(PLATFORM)/rsb/*.c \
	src/platform/$(PLATFORM)/watchdog/*.c \
	src/platform/$(PLATFORM)/apInt.c \
	src/platform/$(PLATFORM)/hostIntf.c \
	src/platform/$(PLATFORM)/sharemem.c \
	src/platform/$(PLATFORM)/crc.c \
	src/platform/$(PLATFORM)/rtc.c \
	src/platform/$(PLATFORM)/exti.c \
	src/platform/$(PLATFORM)/system/notifier/*.c \
	src/platform/$(PLATFORM)/system/message_manager/*.c \
	src/platform/$(PLATFORM)/system/cpu/time_ticks.c \
	src/platform/$(PLATFORM)/service/dvfs/*.c \
	src/platform/$(PLATFORM)/service/standby/*.c \
	src/platform/$(PLATFORM)/service/standby/suspend_resume/*.c \
	src/platform/$(PLATFORM)/system/daemon/*.c \
	src/platform/$(PLATFORM)/dram/dram-sun50iw3/*.c

#platform drivers for bootloader
SRCS_bl += src/platform/$(PLATFORM)/pwr.c src/platform/$(PLATFORM)/gpio.c

#extra deps
DEPS += $(wildcard inc/platform/$(PLATFORM)/*.h)
DEPS += $(wildcard inc/platform/$(PLATFORM)/cmsis/*.h)

#platform flags
PLATFORM_HAS_HARDWARE_CRC = true
FLAGS += -DPLATFORM_HW_VER=0

#platform-specific rules
OBJCOPY_PARAMS = -I elf32-littlearm -O binary

bl.%.bin : bl.%.elf
#	$(OBJCOPY) -j .bl -j .blrodata $(OBJCOPY_PARAMS) $< $@
	$(OBJCOPY) -j .bl -j .blrodata -j .eedata $(OBJCOPY_PARAMS) $< $@

os.%.bin : os.%.elf
	$(OBJCOPY) -j .data -j .text $(OBJCOPY_PARAMS) $< $@

showsizes: os.unchecked.elf
	misc/platform/$(PLATFORM)/showsizes.sh $<

