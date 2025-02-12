#!/bin/bash

CUR_DIR=`pwd`

if [ "$1" == "gk7205v200" ]; then
	export PATH="/home/smb/GK7205V200/Software/GKIPCLinuxV100R001C00SPC020/tools/toolchains/arm-gcc6.3-linux-uclibceabi/bin/:$PATH"
elif [ "$1" == "rv1126" ]; then
	export PATH="/home/smb/RV1126_RV1109_LINUX_SDK_V2.2.5.1_20230530/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/:$PATH"
	HOST=arm-linux-gnueabihf
elif [ "$1" == "x86_64" ]; then
	HOST=
else
	echo "$1 not support, only support gk7205v200, rv1126, x86_64"
	exit 0
fi

CHIP_TYPE=$1
INSTALL_DIR=$CUR_DIR/_install

build_openssl()
{
	SOURCE_DIR=openssl-1.1.1h
	SOURCE_INSTALL_DIR=$INSTALL_DIR/openssl

	cd $CUR_DIR/third_lib
	[ -d $SOURCE_INSTALL_DIR ] && [ "$1" != "rebuild" ] && exit
	[ -d $SOURCE_DIR ] && rm -r $SOURCE_DIR
	[ -d $SOURCE_INSTALL_DIR ] && rm -r $SOURCE_INSTALL_DIR

	tar -xvzf $SOURCE_DIR.tar.gz
	cd $SOURCE_DIR
	./Configure no-asm no-shared no-async linux-generic32 --prefix=$SOURCE_INSTALL_DIR --cross-compile-prefix=$HOST-
	make && make install
	
	cd $CUR_DIR/third_lib
	[ -d $SOURCE_DIR ] && rm -r $SOURCE_DIR
}

build_live555()
{
	SOURCE_DIR=live
	SOURCE_INSTALL_DIR=$INSTALL_DIR/live

	cd $CUR_DIR/third_lib
	[ -d $SOURCE_INSTALL_DIR ] && [ "$1" != "rebuild" ] && exit
	[ -d $SOURCE_DIR ] && rm -r $SOURCE_DIR
	[ -d $SOURCE_INSTALL_DIR ] && rm -r $SOURCE_INSTALL_DIR

	tar -xvzf ${SOURCE_DIR}555-latest.tar.gz
	cd $SOURCE_DIR
	cp $CUR_DIR/live555_config.$CHIP_TYPE ./config.$CHIP_TYPE -raf
	# cp $CUR_DIR/live555_config.$CHIP_TYPE-shared ./config.$CHIP_TYPE -raf
	export PREFIX_DIR="$INSTALL_DIR"
	./genMakefiles $CHIP_TYPE
	make && make install
	
	cd $CUR_DIR/third_lib
	[ -d $SOURCE_DIR ] && rm -r $SOURCE_DIR
}

build_main()
{
	cd $CUR_DIR
	if [ -d $CUR_DIR/build ]; then
		rm -r $CUR_DIR/build
	fi 
	if [ -d $INSTALL_DIR/rtsp ]; then
		rm -r $INSTALL_DIR/rtsp
	fi 

	mkdir build
	cd $CUR_DIR/build
	cmake .. -DCMAKE_C_COMPILER=${HOST}-gcc -DCMAKE_CXX_COMPILER=${HOST}-g++ -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/rtsp -DCHIP_TYPE=$CHIP_TYPE
	make && make install

	cp librtsp.so /home/smb/work/ -raf
	cp rtsp_test /home/smb/work/ -raf
}

[ "$2" == "jthread" ] && build_jthread rebuild && exit
[ "$2" == "jrtplib" ] && build_jrtplib rebuild && exit
[ "$2" == "openssl" ] && build_openssl rebuild && exit
[ "$2" == "live555" ] && build_live555 rebuild && exit
[ "$2" == "main" ] && build_main && exit

if [ "$2" == "all" ]; then
	# build_openssl
	# build_live555
	# build_jthread
    # build_jrtplib
	
	# build_main
fi
