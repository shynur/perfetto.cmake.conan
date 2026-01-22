#!/bin/bash

cd /tmp
rm -f tracebox
curl -LO https://get.perfetto.dev/tracebox
chmod +x tracebox
mv tracebox /usr/local/bin/
