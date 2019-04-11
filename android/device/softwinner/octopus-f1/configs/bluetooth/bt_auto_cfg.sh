#!/bin/bash

BT_CFG_DIR=`echo $(cd "$(dirname "$0")"; pwd)`

TARGET=$1
MODULE=$2
UART=$3
NAME=$4

VND_CFG_FILE=bt_vendor.conf
DEV_CFG_FILE=vnd_$TARGET.txt
STK_CFG_FILE=bdroid_buildcfg.h

TMP_VND_CFG_FILE=tmp_$VND_CFG_FILE
TMP_DEV_CFG_FILE=tmp_$DEV_CFG_FILE
TMP_STK_CFG_FILE=tmp_$STK_CFG_FILE

cp $BT_CFG_DIR/default_bt_bdroid.h     $BT_CFG_DIR/$TMP_STK_CFG_FILE
cp $BT_CFG_DIR/default_bt_vendor.conf  $BT_CFG_DIR/$TMP_VND_CFG_FILE
cp $BT_CFG_DIR/default_bt_device.txt   $BT_CFG_DIR/$TMP_DEV_CFG_FILE

PARA_BAUD_NAME=`echo $TARGET | awk -F '-' '{print  "'$MODULE'_baud_" $1 }'`
PARA_BTFW_NAME=`echo $MODULE"_btfw_name"`

PARA_BAUD_VAL=`cat  $BT_CFG_DIR/default_cfg.txt | sed -e 's/^[ \t]*//g' -e '/^#/d' | grep $PARA_BAUD_NAME`
PARA_BAUD_VAL=`echo $PARA_BAUD_VAL | awk '{print $2}'`

PARA_BTFW_VAL=`cat  $BT_CFG_DIR/default_cfg.txt | sed -e 's/^[ \t]*//g' -e '/^#/d' | grep $PARA_BTFW_NAME`
PARA_BTFW_VAL=`echo $PARA_BTFW_VAL | awk '{print $2}'`

# baudrate in tmp_vnd_<TARGET>.txt
sed -i "/Set baudrate to/s/to.*$/to $PARA_BAUD_VAL/" $BT_CFG_DIR/$TMP_DEV_CFG_FILE
sed -i "/UART_TARGET_BAUD_RATE/s/RATE.*$/RATE = $PARA_BAUD_VAL/" $BT_CFG_DIR/$TMP_DEV_CFG_FILE

# UART in tmp_vnd_<TARGET>.txt
sed -i "/BLUETOOTH_UART_DEVICE_PORT/s/PORT.*$/PORT = \"\/dev\/$UART\"/" $BT_CFG_DIR/$TMP_DEV_CFG_FILE

# UART in tmp_bt_vendor.conf
sed -i "/UartPort/s/Port.*$/Port = \/dev\/$UART/" $BT_CFG_DIR/$TMP_VND_CFG_FILE

# fw name in tmp_bt_vendor.conf
sed -i "/FwPatchFileName/s/Name.*$/Name = $PARA_BTFW_VAL/" $BT_CFG_DIR/$TMP_VND_CFG_FILE

# bt name in tmp_bdroid_buildcfg.h
sed -i "/BTM_DEF_LOCAL_NAME/s/NAME.*$/NAME \"$NAME\"/" $BT_CFG_DIR/$TMP_STK_CFG_FILE

if [ ! -f $BT_CFG_DIR/$DEV_CFG_FILE ]; then
	cp $BT_CFG_DIR/$TMP_DEV_CFG_FILE $BT_CFG_DIR/$DEV_CFG_FILE
else
	md5sum1=`md5sum $BT_CFG_DIR/$DEV_CFG_FILE | awk '{print $1}'`
	md5sum2=`md5sum $BT_CFG_DIR/$TMP_DEV_CFG_FILE | awk '{print $1}'`
	if [ $md5sum1 != $md5sum2 ]; then
		cp $BT_CFG_DIR/$TMP_DEV_CFG_FILE $BT_CFG_DIR/$DEV_CFG_FILE
	fi
fi

if [ ! -f $BT_CFG_DIR/$VND_CFG_FILE ]; then
	cp $BT_CFG_DIR/$TMP_VND_CFG_FILE $BT_CFG_DIR/$VND_CFG_FILE
else
	md5sum1=`md5sum $BT_CFG_DIR/$VND_CFG_FILE | awk '{print $1}'`
	md5sum2=`md5sum $BT_CFG_DIR/$TMP_VND_CFG_FILE | awk '{print $1}'`
	if [ $md5sum1 != $md5sum2 ]; then
		cp $BT_CFG_DIR/$TMP_VND_CFG_FILE $BT_CFG_DIR/$VND_CFG_FILE
	fi
fi

if [ ! -f $BT_CFG_DIR/$STK_CFG_FILE ]; then
	cp $BT_CFG_DIR/$TMP_STK_CFG_FILE $BT_CFG_DIR/$STK_CFG_FILE
else
	md5sum1=`md5sum $BT_CFG_DIR/$STK_CFG_FILE | awk '{print $1}'`
	md5sum2=`md5sum $BT_CFG_DIR/$TMP_STK_CFG_FILE | awk '{print $1}'`
	if [ $md5sum1 != $md5sum2 ]; then
		cp $BT_CFG_DIR/$TMP_STK_CFG_FILE $BT_CFG_DIR/$STK_CFG_FILE
	fi
fi

rm -rf $BT_CFG_DIR/$TMP_DEV_CFG_FILE
rm -rf $BT_CFG_DIR/$TMP_VND_CFG_FILE
rm -rf $BT_CFG_DIR/$TMP_STK_CFG_FILE
