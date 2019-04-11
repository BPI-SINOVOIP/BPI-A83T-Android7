#!/bin/bash
if [ ! -d ./toolchain/gcc-arm-none-eabi-5_3-2016q1-20160330-linux ]; then
	echo uncompress toolchain package...
	tar  -jxf ./toolchain/gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 -C ./toolchain
	echo uncompress toolchain finish
fi

export PATH=$PATH:$PWD/toolchain/gcc-arm-none-eabi-5_3-2016q1-20160330-linux/gcc-arm-none-eabi-5_3-2016q1/bin:$PWD/../util/nanoapp_postprocess
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/toolchain/gcc-arm-none-eabi-5_3-2016q1-20160330-linux/gcc-arm-none-eabi-5_3-2016q1/lib
export LIBPATH=$PWD/toolchain/gcc-arm-none-eabi-5_3-2016q1-20160330-linux/gcc-arm-none-eabi-5_3-2016q1/lib/gcc/arm-none-eabi/5.3.1/
printf "\n superm $PWD \n"
export CROSS_COMPILE=$PWD/toolchain/gcc-arm-none-eabi-5_3-2016q1-20160330-linux/gcc-arm-none-eabi-5_3-2016q1/bin/arm-none-eabi-
make clean
make
cp full.bin ../../../../../lichee/tools/pack/chips/sun50iw3p1/bin/scp.bin
