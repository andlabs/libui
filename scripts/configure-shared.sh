if [ "$BUILD_ARCH" == "ia32" ]; then
  cmake .. -G "Unix Makefiles" -DBUILD_32=ON
fi
if [ "$BUILD_ARCH" == "x64" ]; then
  cmake .. -G "Unix Makefiles"
fi
