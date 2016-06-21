if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  docker run -e "TRAVIS_OS_NAME=$TRAVIS_OS_NAME" -e "TRAVIS_TAG=$TRAVIS_TAG" -i -v "${PWD}:/libui-32" leandrosouza/cmake-gcc /bin/bash -c "linux32 --32bit i386 /libui-32/scripts/build-32.sh"
fi
