#!/bin/sh
BASE_DIR=$(pwd)
# pushd $BASE_DIR/../syscalls
# make
# cp -r to_fsdir/* ../fsdir/
# popd
../createfs -i ../fsdir -o filesys_img
make
./debug.sh
