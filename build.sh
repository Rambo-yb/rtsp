#!/bin/bash

CUR_DIR=`pwd`

if [ "$1" == "gk7205v200" ]; then
	export PATH="/home/smb/GK7205V200/Software/GKIPCLinuxV100R001C00SPC020/tools/toolchains/arm-gcc6.3-linux-uclibceabi/bin/:$PATH"
	HOST="arm-gcc6.3-linux-uclibceabi"
elif [ "$1" == "rv1126" ]; then
	export PATH="/home/smb/RV1126_RV1109_LINUX_SDK_V2.2.5.1_20230530/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/:$PATH"
	HOST=arm-linux-gnueabihf
elif [ "$1" == "x86_64" ]; then
	HOST=
elif [ "$1" == "rk3588" ]; then
	export PATH="/home/smb/compiler/rk3588_host/bin/:$PATH"
	export PERL5LIB="/home/smb/compiler/rk3588_host/lib/perl5/5.32.1:$PERL5LIB"
	HOST="aarch64-buildroot-linux-gnu"
else
	echo "$1 not support, only support gk7205v200, rv1126, x86_64, rk3588"
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
	./Configure no-asm no-async linux-generic32 --prefix=$SOURCE_INSTALL_DIR --cross-compile-prefix=$HOST-
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
}

build_pack()
{
    cd $CUR_DIR

    PACK_DIR=rtsp_$1
    [ -d $PACK_DIR ] && rm -r $PACK_DIR
    [ -f $PACK_DIR.tar.gz ] && rm -r $PACK_DIR.tar.gz
    
    mkdir $PACK_DIR
    cd $PACK_DIR
    cp $INSTALL_DIR/rtsp/* ./ -raf
    cp $INSTALL_DIR/openssl/lib/lib*.so* ./lib -raf

    cd ..
    tar -cvzf $PACK_DIR.tar.gz $PACK_DIR
    rm $PACK_DIR -r
}

[ "$2" == "openssl" ] && build_openssl rebuild && exit
[ "$2" == "live555" ] && build_live555 rebuild && exit
[ "$2" == "main" ] && build_main && exit
[ "$2" == "pack" ] && build_pack $1 && exit

if [ "$2" == "all" ]; then
	build_openssl
	# build_live555
	
	build_main
	build_pack $1
fi
