#!/bin/bash

if [ -z "$BUILD_DIR" ]; then
  echo "Enter where you want survex to be built"
  read -p "This is usually somewhere like steamapps/common/Half-Life/survex : " BUILD_DIR
fi
echo "Copying assets"
cp -r assets/* "$BUILD_DIR"
echo "Building"
working_dir=$PWD
cd "$BUILD_DIR"
mkdir cl_dlls dlls
cd "$working_dir/linux"
make
cd release
cp hl.so "$BUILD_DIR/dlls"
cp client.so "$BUILD_DIR/cl_dlls"
echo "We are finised"
