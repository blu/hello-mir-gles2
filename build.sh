#!/bin/bash

CLICK=click_resource
CLICK_PACKAGE=hello-egl_0.1_armhf.click
RUN_LOG=~/.cache/upstart/application-click-hello-egl_hello-egl_0.1.log

TARGET=${CLICK}/hello-egl
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
	-marm
)
DEPEND=(
	/usr/lib/arm-linux-gnueabihf/libhybris-egl/libEGL.so.1
	/usr/lib/arm-linux-gnueabihf/libhybris-egl/libGLESv2.so.2
	/usr/lib/arm-linux-gnueabihf/libmirclient.so.9
)

g++-4.9 ${SOURCE[@]} ${CFLAGS[@]} ${DEPEND[@]} && click build ${CLICK} && pkcon install-local --allow-untrusted ${CLICK_PACKAGE}

if [ -f $TARGET ]; then
	rm $TARGET
fi

if [ -f $CLICK_PACKAGE ]; then
	rm $CLICK_PACKAGE
fi

if [ -f $RUN_LOG ]; then
	rm $RUN_LOG
fi
