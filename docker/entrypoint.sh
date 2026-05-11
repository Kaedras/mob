#!/bin/sh

set -e

if [ "$DO_BUILD" -eq 0 ] && [ "$DO_RELEASE" -eq 0 ]; then
  echo "Build and release are both disabled, exiting"
  return 0
fi

if [ "$DO_BUILD" -eq 1 ]; then
  # clean up directories that should be recreated
  rm -rf /root/build/build/AppImage
  rm -rf /root/build/install
fi

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

if [ "$DO_RELEASE" -eq 1 ]; then
  # install google breakpad tools
  vcpkg install breakpad[tools]
  export PATH="$VCPKG_ROOT/installed/x64-linux/tools/breakpad:$PATH"
fi

# install mob
if [ ! -d "mob/.git" ]; then
  echo cloning mob...
  git clone -q --depth 1 https://github.com/Kaedras/mob.git
  cd mob
  echo building mob...
  ./bootstrap.sh
else
  cd mob
  if [ "$UPDATE_MOB" -eq 1 ]; then
    git pull -q
    echo building mob...
    ./bootstrap.sh
  fi
fi

if [ "$DO_BUILD" -eq 1 ]; then
  echo building modorganizer...
  ./mob -d /root/build --log-file mob.build.log build
fi

if [ "$DO_RELEASE" -eq 1 ]; then
  echo creating release...
  ./mob -d /root/build release devbuild --appimage --log-file mob.release.log
fi

if [ "${USE_CCACHE:-0}" -eq 1 ]; then
  echo ccache stats:
  ccache --show-stats
fi
