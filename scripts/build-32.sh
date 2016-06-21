#!/bin/bash

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  echo faked > ./artifacts/examples-static-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/libui-static-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/examples-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/libui-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
else
  export BUILD_ARCH=ia32
  cd build
  ../scripts/clean-build-dir.sh
  ../scripts/configure-static.sh
  make tester examples
  ../scripts/create-artifacts-static.sh
  ../scripts/clean-build-dir.sh
  ../scripts/configure-shared.sh
  make tester examples
  ../scripts/create-artifacts-shared.sh
fi

