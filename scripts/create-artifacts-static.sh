#!/bin/bash
cd out
tar -cvf ../../artifacts/examples-static-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz controlgallery cpp-multithread histogram
tar -cvf ../../artifacts/libui-static-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz libui.a

