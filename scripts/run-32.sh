#!/bin/bash
sudo apt-get dist-upgrade -y;
sudo apt-get update -y;

sudo apt-get install software-properties-common
sudo add-apt-repository ppa:george-edison55/cmake-3.x -y;
sudo apt-get update -y;

sudo apt-get install build-essential -y;
sudo apt-get install cmake -y;
sudo apt-get install pkg-config -y;
sudo apt-get install libgtk-3-dev -y;

# sudo apt-get install gtk+-3.0 -y;
cd /libui-32;
./scripts/build-32.sh;
