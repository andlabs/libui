#!/bin/bash

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
chmod o+rw ../artifacts/*
