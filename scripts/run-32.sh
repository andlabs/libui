#!/bin/bash

sudo apt-get update;
sudo apt-get install libc6-dev-i386 -y;
sudo apt-get install libgtk-3-dev:i386 -y;
./scripts/build-32.sh
