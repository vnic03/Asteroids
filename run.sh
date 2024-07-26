#!/bin/bash

BUILD="build"

if [ ! -d "$BUILD" ]; then
  mkdir $BUILD
fi

cd $BUILD

if cmake .. && make; then
  if ./Asteroids; then
    echo "Done! Have fun :)"
  fi
else
  echo "Mmh.. something went wrong :("
fi
