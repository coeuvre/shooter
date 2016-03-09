#!/bin/sh

cc=clang
src=`pwd`/src/shooter.c

clear

[ ! -d "build" ] && mkdir build

pushd build

$cc -W -Wall -g -std=c11 $src -c -o shooter.o
$cc -lSDL2 -lhammer shooter.o -o shooter

success=$?

popd

exit $success

