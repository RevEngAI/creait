#!/usr/bin/env sh

cmake -G Ninja -B Build -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=/usr/local
ninja -C Build
