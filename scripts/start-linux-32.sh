#!/bin/bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  linux32 --32bit i386 ./scripts/run-32.sh
fi
