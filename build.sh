#!/bin/bash

CC=g++
CLICK=click_resource
CLICK_PACKAGE=hello-gles_0.1_armhf.click
APP_LOG=~/.cache/upstart/application-click-hello-gles_hello-gles_0.1.log

TARGET=${CLICK}/hello-gles
SOURCE=(
	eglapp.cpp
	hello.cpp
)
CFLAGS=(
	-o ${TARGET}
	-I/usr/include/mircommon
	-I/usr/include/mirclient
	-I./include
	-DANDROID
)
DEPEND=(
	`ldconfig -p | grep -m 1 ^[[:space:]]libEGL.so | sed "s/^.\+ //"`
	`ldconfig -p | grep -m 1 ^[[:space:]]libGLESv2.so | sed "s/^.\+ //"`
	`ldconfig -p | grep -m 1 ^[[:space:]]libmirclient.so | sed "s/^.\+ //"`
)

if [[ $HOSTTYPE == "arm" ]]; then

	# Canonical insist on targeting Thumb2 on ARM - go proper ARM
	CFLAGS+=(
		-marm
		-march=armv8-a
	)

	# clang can fail auto-detecting the host armv8 cpu on some setups; collect all part numbers
	UARCH=`cat /proc/cpuinfo | grep "^CPU part" | sed s/^[^[:digit:]]*//`

	# in order of preference, in case of big.LITTLE
	if   [ `echo $UARCH | grep -c 0xd09` -ne 0 ]; then
		CFLAGS+=(
			-mtune=cortex-a73
		)
	elif [ `echo $UARCH | grep -c 0xd08` -ne 0 ]; then
		CFLAGS+=(
			-mtune=cortex-a72
		)
	elif [ `echo $UARCH | grep -c 0xd07` -ne 0 ]; then
		CFLAGS+=(
			-mtune=cortex-a57
		)
	elif [ `echo $UARCH | grep -c 0xd03` -ne 0 ]; then
		CFLAGS+=(
			-mtune=cortex-a53
		)
	fi

elif [[ $HOSTTYPE == "x86_64" || $HOSTTYPE == "i686" ]]; then

	CFLAGS+=(
		-march=native
		-mtune=native
	)

fi

BUILD_CMD=${SOURCE[@]}" "${CFLAGS[@]}" "${DEPEND[@]}
echo $CC $BUILD_CMD

"$CC" $BUILD_CMD && click build ${CLICK} && pkcon install-local --allow-untrusted ${CLICK_PACKAGE}

if [ -f $TARGET ]; then
	rm $TARGET
fi

if [ -f $CLICK_PACKAGE ]; then
	rm $CLICK_PACKAGE
fi

if [ -f $APP_LOG ]; then
	rm $APP_LOG
fi
