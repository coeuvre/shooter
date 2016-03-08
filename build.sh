#!/bin/sh

cc=clang
src="`pwd`/src/shooter.c `pwd`/src/hammer/hammer.c"

clear

[ ! -d "build" ] && mkdir build

pushd build

$cc -W -Wall -g -std=c11 -DHM_GAME_CODE_STATIC $src -lSDL2 -o shooter

success=$?

popd

exit $success

