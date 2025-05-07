#!/usr/bin/env bash

cd ..
conan install . --output-folder=build --build=missing

# Check if the build folder exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Exiting."
    exit 1
fi

pushd ./build

cmake .. -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug

make -j$(nproc)

popd
