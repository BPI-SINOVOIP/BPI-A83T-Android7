#!/bin/bash
#step1:prepare some tmp files
#if you want to add your platform,just amend here
platform=$1
if [ "$platform" == "sun50iw1p1" ]; then
	ARCH="arm64"
	SDC0_FLAG="sdmmc@01c0f000"
	SDC2_FLAG="sdmmc@01C11000"
elif [ "$platform" == "sun8iw11p1" \
	-o "$platform" == "sun8iw5p1" \
	-o "$platform" == "sun8iw6p1" ];then
	ARCH="arm"
	SDC0_FLAG="sdmmc@01c0f000"
	SDC2_FLAG="sdmmc@01C11000"
else
	echo "unsupported platform!!!!!!!!"
	exit 0
fi
#platform independent code
#decompose the original dtsi to five parts,
#then compose into new dtsi
dtsi_file="./arch/${ARCH}/boot/dts/${platform}.dtsi"
dtsi_bak_file="./arch/${ARCH}/boot/dts/${platform}_bak.dtsi"
dtsi_start_file="./arch/${ARCH}/boot/dts/${platform}_start.dtsi"
dtsi_sdc2_file="./arch/${ARCH}/boot/dts/${platform}_sdc2.dtsi"
dtsi_mid_file="./arch/${ARCH}/boot/dts/${platform}_mid.dtsi"
dtsi_sdc0_file="./arch/${ARCH}/boot/dts/${platform}_sdc0.dtsi"
dtsi_end_file="./arch/${ARCH}/boot/dts/${platform}_end.dtsi"

rm -rf $dtsi_bak_file
rm -rf $dtsi_start_file
rm -rf $dtsi_sdc2_file
rm -rf $dtsi_sdc0_file
rm -rf $dtsi_end_file

cp $dtsi_file	$dtsi_bak_file
#step2:decompose the old_dtsi_file=start:sdc2:mid:sdc0:end
#search the sdc2 head by SDC2_FLAG
sdc2_start=`sed  -n '/'${SDC2_FLAG}'/=' $dtsi_file`
#search the sdc2 tail by the first "};" after SDC2_FLAG
sdc2_end_tmp=`sed  -n ''$sdc2_start',${/'}\;'/=}' $dtsi_file `
sdc2_end=`echo $sdc2_end_tmp | awk -F ' ' '{print $1}'`

#search the sdc0 head by SDC0_FLAG
sdc0_start=`sed  -n '/'${SDC0_FLAG}'/=' $dtsi_file`
#search the sdc0 tail by the first "};" after SDC0_FLAG
sdc0_end_tmp=`sed  -n ''$sdc0_start',${/'}\;'/=}' $dtsi_file `
sdc0_end=`echo $sdc0_end_tmp | awk -F ' ' '{print $1}'`

#sdc2_start < sdc0_start normally in dtsi
if [ "$sdc2_start" -gt "$sdc0_start" ] ;then
	echo "sdc2 is after sdc0,may be something wrong"
	exit 0
fi

part0_start=1
part0_end=$(($sdc2_start - 1))

part1_start=$(($sdc2_start))
part1_end=$(($sdc2_end))

part2_start=$(($sdc2_end + 1))
part2_end=$(($sdc0_start - 1))

part3_start=$(($sdc0_start))
part3_end=$(($sdc0_end))

part4_start=$(($sdc0_end + 1))

sed -n '1,'${part0_end}'p'                $dtsi_file >> $dtsi_start_file
sed -n ''${part1_start}','${part1_end}'p' $dtsi_file >> $dtsi_sdc2_file
sed -n ''${part2_start}','${part2_end}'p' $dtsi_file >> $dtsi_mid_file
sed -n ''${part3_start}','${part3_end}'p' $dtsi_file >> $dtsi_sdc0_file
sed -n ''${part4_start}',$p'              $dtsi_file >> $dtsi_end_file

#step3:compose the new_dtsi_file=start:sdc0:mid:sdc2:end
rm -rf $dtsi_file
cat $dtsi_start_file $dtsi_sdc0_file $dtsi_mid_file $dtsi_sdc2_file $dtsi_end_file >> $dtsi_file

#step4:clean some tmp files
rm -rf $dtsi_start_file
rm -rf $dtsi_sdc2_file
rm -rf $dtsi_mid_file
rm -rf $dtsi_sdc0_file
rm -rf $dtsi_end_file
