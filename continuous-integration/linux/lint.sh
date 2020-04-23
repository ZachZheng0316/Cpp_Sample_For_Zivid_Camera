#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$(realpath "$SCRIPT_DIR/../..")
SOURCE_DIR="$ROOT_DIR/source"

cppFiles=$(find "$SOURCE_DIR" -name '*.cpp')
hFiles=$(find "$SOURCE_DIR" -name '*.h' -not -path "$SOURCE_DIR/3rd-party/*")

if [ -z "$cppFiles" ]; then
    echo Error: Cannot find C++ source files
    exit 1
fi

echo "Checking formatting"
errorsFound=0
for fileName in $cppFiles $hFiles; do
    echo $fileName
    diff $fileName <(clang-format-8 $fileName)
    if [ $? -ne 0 ]; then
        let "errorsFound=errorsFound+1"
    else
        echo "ok"
    fi
done
echo
if [ $errorsFound -ne 0 ]; then
    echo "ERROR: $errorsFound formatting error(s) found! See the diffs for each file printed above."
    exit 1
fi

BUILD_DIR="$ROOT_DIR/build/ci/tidy"
mkdir --parents "$BUILD_DIR" || exit $?
cd "$BUILD_DIR" || exit $?
cmake -GNinja \
    -DLINT_ONLY=ON \
    -DUSE_EIGEN3=ON \
    -DUSE_OPENCV=OFF \
    -DUSE_PCL=OFF \
    -DEIGEN3_INCLUDE_DIR="/etc/eigen/" \
    -DCMAKE_CXX_CLANG_TIDY="/usr/bin/clang-tidy-8" \
    -DWARNINGS=ON \
    -DWARNINGS_AS_ERRORS=ON \
    "$SOURCE_DIR" || exit $?
cmake --build . || exit $?

cmake -ULINT_ONLY "$SOURCE_DIR" || exit $?

echo "All files are properly formatted!" ["$0"]
