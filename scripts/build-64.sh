BUILD_ARCH=x64
./scripts/install-gtk.sh
./scripts/upgrade-cmake.sh
./scripts/create-dirs.sh
cd build
../scripts/configure-static.sh
make tester examples
../scripts/create-artifacts-static.sh
../scripts/clean-build-dir.sh
../scripts/configure-shared.sh
make tester examples
../scripts/create-artifacts-shared.sh
