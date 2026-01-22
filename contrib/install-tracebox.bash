#!/bin/bash

cd /tmp
rm -f tracebox
curl -LO https://get.perfetto.dev/tracebox
chmod +x tracebox
if which sudo &>/dev/null && [ -x `which sudo` ]; then
    sudo mv tracebox /usr/local/bin/
else
    mv tracebox /usr/local/bin/
fi
