#/bin/bash

set -x
set -e

source ~/.bashrc
qmake
make clean
make
upx connect_ui

DATE=$(date +%Y%m%d-%H:%M:%S)
mv connect_ui ../connect-ui-bin/connect_ui-$DATE
