if [ "$TRAVIS_OS_NAME" == "linux" ]; then
  rm out/libui.so;
  LIB_FILE=libui.so.0;
fi

cd out
tar -cvf ../../artifacts/examples-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz controlgallery cpp-multithread histogram $LIB_FILE
tar -cvf ../../artifacts/libui-shared-$TRAVIS_OS_NAME-ia32-$TRAVIS_TAG.tar.gz $LIB_FILE
