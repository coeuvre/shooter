#!/bin/sh

cc=clang
src=`pwd`/src/shooter.c

clear

[ ! -d "build" ] && mkdir build

pushd build

$cc -o shooter -std=c11 -W -Wall -g $src -lSDL2

success=$?

popd

exit $success

