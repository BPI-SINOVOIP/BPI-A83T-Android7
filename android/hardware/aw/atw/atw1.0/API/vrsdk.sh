#!/bin/bash
PROJ_TYPE=neptune-y2
ANDROID_OUT=../../../../../out/target/product/${PROJ_TYPE}
ATW_DIR=../
DST_DIR=./prebuild

cp ${ANDROID_OUT}/system/lib64/libatw_api.so     ${DST_DIR}/lib64/
cp ${ANDROID_OUT}/system/lib/libatw_api.so     ${DST_DIR}/lib/

cp ${ANDROID_OUT}/system/lib64/libc++.so     ${DST_DIR}/lib64/
cp ${ANDROID_OUT}/system/lib/libc++.so     ${DST_DIR}/lib/

cp ${ATW_DIR}/common/AW_CAPI.h                 ./inc/
cp ${ATW_DIR}/common/AtwTypesPublic.h          ./inc/
