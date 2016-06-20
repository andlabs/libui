if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  sudo apt-get update;
  sudo apt-get install libgtk-3-dev -y || sudo apt-cache search libgtk3;
fi
