#!/bin/bash

LIB_FILE=libui.*;

cd out
tar -cvf ../../artifacts/examples-shared-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz controlgallery cpp-multithread histogram $LIB_FILE
tar -cvf ../../artifacts/libui-shared-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz $LIB_FILE ../../*.h
