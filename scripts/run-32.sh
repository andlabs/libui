#!/bin/bash

cd /libui-32;
apt-get update;
apt-get install libc6-dev-i386 -y;
apt-get install libgtk-3-dev:i386 -y;
./scripts/build-32.sh
