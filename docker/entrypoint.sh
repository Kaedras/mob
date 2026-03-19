#!/bin/sh

set -e

# clean up directories that should be recreated
rm -rf /root/build/build/AppImage
rm -rf /root/build/install

if [ ! -d "mob/.git" ]; then
    echo cloning mob...
    git clone -q --depth 1 https://github.com/Kaedras/mob.git
fi

cd mob
echo building mob...
./bootstrap.sh

echo building modorganizer...
./mob -d /root/build --log-file mob.build.log build

echo creating release...
./mob -d /root/build release devbuild --appimage --log-file mob.release.log

if [ "${USE_CCACHE:-0}" -eq 1 ]; then
  echo ccache stats:
  ccache --show-stats
fi
