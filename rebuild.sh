#!/bin/bash
rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ../ 
make
mkdir packageroot
cp -rf ../DEBIAN packageroot/
mkdir -p packageroot/usr/bin/
mkdir -p packageroot/etc/volumehidtoosc/
cp bin/volumehidtoosc packageroot/usr/bin/
cp ../volumehidtoosc.conf packageroot/etc/volumehidtoosc/

dpkg-deb -b packageroot volumehidtoosc_1.0.0_amd64.deb



