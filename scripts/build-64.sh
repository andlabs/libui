#!/bin/bash

export BUILD_ARCH=x64
cd build
../scripts/configure-static.sh
make tester examples
../scripts/create-artifacts-static.sh
../scripts/clean-build-dir.sh
../scripts/configure-shared.sh
make tester examples
../scripts/create-artifacts-shared.sh
