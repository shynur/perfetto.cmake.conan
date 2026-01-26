#!/bin/bash

NEED_INSTALL_CMAKE=
if ! which cmake; then
    NEED_INSTALL_CMAKE=1
elif ! cmake --version >/dev/null; then
    NEED_INSTALL_CMAKE=1
elif [ v3.23 != v`{ echo 3.23; cmake --version | egrep '^cmake version [[:digit:]]+[.][[:digit:]]+' | awk '{printf $3}'; } | sort -V | head -1` ]; then
    NEED_INSTALL_CMAKE=1
fi

if ! [ $NEED_INSTALL_CMAKE ]; then
    exit
fi

cd /tmp
if ! [ -f cmake-3.31.10-linux-$HOSTTYPE.sh ]; then
    curl `#--remove-on-error` -LO https://github.com/Kitware/CMake/releases/download/v3.31.10/cmake-3.31.10-linux-$HOSTTYPE.sh
fi
bash cmake-3.31.10-linux-$HOSTTYPE.sh --skip-license --prefix=/usr/local/ --exclude-subdir

/usr/local/bin/cmake --version
