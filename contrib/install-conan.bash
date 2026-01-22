#!/bin/bash

cd /tmp
rm -f conan-ubuntu-64.deb
curl -LO https://github.com/conan-io/conan/releases/download/1.66.0/conan-ubuntu-64.deb
if which sudo &>/dev/null && [ -x `which sudo` ]; then
    sudo apt install -y ./conan-ubuntu-64.deb
else
    apt install -y ./conan-ubuntu-64.deb
fi
