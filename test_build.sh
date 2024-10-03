rm -rf build
mkdir build
cd build
cmake -DBUILD_TESTS_ONLY=ON ..
make