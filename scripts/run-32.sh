#!/bin/bash

cd /libui-32;
sudo apt-get update;
sudo apt-get install libgtk-3-dev -y || sudo apt-cache search libgtk3;
./scripts/build-32.sh
