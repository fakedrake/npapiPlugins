#!/bin/bash
set -e

build_dir=build-linux

git submodule init
git submodule update
firebreath-dev/prepmake.sh Codebendercc $build_dir -D CMAKE_BUILD_TYPE="Debug"
make -C $build_dir

ffdir=$HOME/.mozilla/firefox/*.default/extensions/codebender@codebender.cc/plugins
so=$(pwd)/$build_dir/bin/Codebendercc/npCodebendercc.so

echo "###########################################################################"
echo "Your plugin .so is in $(pwd)/build-linux/bin/Codebendercc/npCodebendercc.so"
echo "firefox:"
echo "    mkdir -p $ffdir"
echo "    ln -s $so $ffdir"
echo "###########################################################################"
