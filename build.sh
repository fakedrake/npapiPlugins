#!/bin/bash
set -e

build_dir=build-linux

git submodule init
git submodule update
firebreath-dev/prepmake.sh Codebendercc $build_dir -D CMAKE_BUILD_TYPE="Debug"
make -C $build_dir

ffdir=$HOME/.mozilla/firefox/*.default/extensions/codebender@codebender.cc/plugins
so=$(pwd)/$build_dir/bin/Codebendercc/npCodebendercc.so
install_rdf=$(pwd)/Codebendercc/fake_install.rdf

echo "###########################################################################"
echo "Your plugin .so is in $(pwd)/build-linux/bin/Codebendercc/npCodebendercc.so"
echo "firefox:"
echo "    mkdir -p $ffdir"
echo "    ln -s $so $ffdir"
echo "###########################################################################"

rm -f $build_dir/codebendercc.xpi
rm -rf $build_dir/firefox_xpi

mkdir -p $build_dir/firefox_xpi/plugins
cp $so $build_dir/firefox_xpi/plugins
cp $install_rdf $build_dir/firefox_xpi/install.rdf
cd $build_dir/firefox_xpi && zip ../codebendercc.xpi plugins/npCodebendercc.so install.rdf
