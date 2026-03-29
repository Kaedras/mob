#!/bin/sh

set -e

# clean up directories that should be recreated
rm -rf /root/build/build/AppImage
rm -rf /root/build/install

# enable ccache if $USE_CCACHE equals 1
if [ "${USE_CCACHE:-0}" -eq 1 ]; then
  export CMAKE_C_COMPILER_LAUNCHER=ccache
  export CMAKE_CXX_COMPILER_LAUNCHER=ccache
fi

# install vcpkg
if [ ! -d "vcpkg/.git" ]; then
  git clone -q --depth 1 https://github.com/microsoft/vcpkg.git
  cd vcpkg
else
  cd vcpkg
  git pull -q
fi
./bootstrap-vcpkg.sh -disableMetrics
cd ..
export VCPKG_ROOT=/root/vcpkg
export PATH="$VCPKG_ROOT:$PATH"

# install google breakpad tools
vcpkg install breakpad[tools]
export PATH="$VCPKG_ROOT/installed/x64-linux/tools/breakpad:$PATH"

# install mob
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
