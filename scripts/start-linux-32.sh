if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  docker run -i -v "${PWD}:/libui-32" andystanton/gcc-cmake-gl /bin/bash -c "linux32 --32bit i386 /libui-32/scripts/build-32.sh"
fi
