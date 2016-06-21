if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  rm out/libui.so;
  LIB_FILE=libui.so.0;
fi

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  LIB_FILE=libui.A.dylib;
fi

cd out
tar -cvf ../../artifacts/examples-shared-$TRAVIS_OS_NAME-x64-$TRAVIS_TAG.tar.gz controlgallery cpp-multithread histogram $LIB_FILE
tar -cvf ../../artifacts/libui-shared-$TRAVIS_OS_NAME-x64-$TRAVIS_TAG.tar.gz $LIB_FILE
if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  echo faked > ../../artifacts/examples-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz;
  echo faked > ../../artifacts/libui-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz;
fi
