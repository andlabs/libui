#!/bin/bash

sudo apt-get update;
sudo apt-get install libc6-dev-i386 -y;
sudo apt-get install libgtk-3-dev -y;
# sudo apt-get install gtk+-3.0 -y;
cd /libui-32;
./scripts/build-32.sh;
