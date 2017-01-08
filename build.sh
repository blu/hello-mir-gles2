#!/bin/bash

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
	-marm
)
DEPEND=(
	/usr/lib/arm-linux-gnueabihf/libhybris-egl/libEGL.so.1
	/usr/lib/arm-linux-gnueabihf/libhybris-egl/libGLESv2.so.2
	/usr/lib/arm-linux-gnueabihf/libmirclient.so.9
)

g++ ${SOURCE[@]} ${CFLAGS[@]} ${DEPEND[@]} && click build ${CLICK} && pkcon install-local --allow-untrusted ${CLICK_PACKAGE}

if [ -f $TARGET ]; then
	rm $TARGET
fi

if [ -f $CLICK_PACKAGE ]; then
	rm $CLICK_PACKAGE
fi

if [ -f $APP_LOG ]; then
	rm $APP_LOG
fi
