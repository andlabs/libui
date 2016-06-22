#!/bin/bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  linux32 --32bit i386 ./scripts/run-32.sh
else
  echo faked > ./artifacts/examples-static-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/libui-static-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/examples-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/libui-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
else
