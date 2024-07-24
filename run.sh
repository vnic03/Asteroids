#!/bin/bash

BUILD="build"

if [ ! -d "$BUILD" ]; then
  mkdir $BUILD
fi

cd $BUILD

cmake ..
make

if [ $? -eq 0 ]; then
  echo "Done! Have fun :)"
  ./Asteroids
fi
