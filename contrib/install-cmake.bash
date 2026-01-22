#!/bin/bash

cd /tmp
if ! [ -f cmake-3.31.10-linux-$HOSTTYPE.sh ]; then
    curl -LO https://github.com/Kitware/CMake/releases/download/v3.31.10/cmake-3.31.10-linux-$HOSTTYPE.sh
fi
bash cmake-3.31.10-linux-$HOSTTYPE.sh --skip-license --prefix=/usr/local/ --exclude-subdir
/usr/local/bin/cmake --version
