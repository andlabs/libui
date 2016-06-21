if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  rm out/libui.so;
  LIB_FILE=libui.so.0;
fi

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  LIB_FILE=libui.A.dylib;
fi

cd out
tar -cvf ../../artifacts/examples-shared-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz controlgallery cpp-multithread histogram $LIB_FILE
tar -cvf ../../artifacts/libui-shared-$TRAVIS_OS_NAME-$BUILD_ARCH-$TRAVIS_TAG.tar.gz $LIB_FILE
