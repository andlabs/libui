cd /libui-32
./scripts/install-gtk.sh
./scripts/install-cmake.sh
./scripts/create-dirs.sh
cd build
../scripts/configure-static.sh
make tester examples
../scripts/create-artifacts-static-32.sh
../scripts/clean-build-dir.sh
../scripts/configure-shared.sh
make tester examples
../scripts/create-artifacts-shared-32.sh
