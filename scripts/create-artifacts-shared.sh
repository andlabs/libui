#!/bin/bash

LIB_FILE=libui.*;

cd out
mkdir tmp
cp controlgallery cpp-multithread histogram $LIB_FILE tmp
cd tmp
tar -cvf ../../../artifacts/examples-shared-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz *
cd ..

rm -rf tmp
mkdir tmp
cp $LIB_FILE ../../*.h tmp
cd tmp
tar -cvf ../../../artifacts/libui-shared-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz *
