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
if [ "$RUN_HL" = "y" ]; then
  if [ -z "$STEAM_DIR" ]; then
    echo "Steam executable directory not specified"
    exit 1
  fi
  echo "Running half life"
  $STEAM_DIR -applaunch 70 -game survex +sv_cheats 1 +developer 2 +map c1a0
fi
