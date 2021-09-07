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
mkdir -p packageroot/usr/lib/systemd/system/
mkdir -p packageroot/var/log/volumehidtoosc/

cp bin/volumehidtoosc packageroot/usr/bin/
cp ../volumehidtoosc.conf packageroot/etc/volumehidtoosc/
cp ../volumehidtoosc.service packageroot/usr/lib/systemd/system/
touch packageroot/var/log/volumehidtoosc/volumehidtoosc.log

dpkg-deb -b packageroot volumehidtoosc_1.0.0_amd64.deb



