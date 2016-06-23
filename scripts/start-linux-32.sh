#!/bin/bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  docker run -e "TRAVIS_OS_NAME=$TRAVIS_OS_NAME" -e "TRAVIS_TAG=$TRAVIS_TAG" -i -v "${PWD}:/libui-32" 32bit/ubuntu /bin/bash -c "linux32 --32bit i386 /libui-32/scripts/run-32.sh"
  
  ls -l ./artifacts	
else
  echo faked > ./artifacts/examples-static-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/libui-static-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/examples-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
  echo faked > ./artifacts/libui-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz
fi
