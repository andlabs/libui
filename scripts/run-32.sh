#!/bin/bash

cd /libui-32;
apt-get update;
apt-get install libc6-dev-i386;
apt-get install libgtk-3-dev -y || apt-cache search libgtk3;
./scripts/build-32.sh
