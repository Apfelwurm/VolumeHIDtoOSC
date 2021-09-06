#!/bin/bash
# rm -rf /etc/volumehidtoosc/volumehidtoosc.conf
# rm -rf /usr/lib/systemd/system/volumehidtoosc.service
# rm -rf /usr/lib/systemd/user/volumehidtoosc.service
# rm -rf /usr/bin/volumehidtoosc
rm -rf build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ../ 
make

./bin/volumehidtoosc

