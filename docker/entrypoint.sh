#!/bin/sh

set -e

if [ ! -z "$DEBUG" ] ; then
  env
  gcc -v
  python -V
fi

if [ "$DO_BUILD" -eq 0 ] && [ "$DO_RELEASE" -eq 0 ]; then
  echo "Build and release are both disabled, exiting"
  return 0
fi

if [ "$DO_BUILD" -eq 1 ]; then
  # clean up directories that should be recreated
  rm -rf /data/build/AppImage
  rm -rf /data/install
fi

# enable ccache if $USE_CCACHE equals 1
if [ "${USE_CCACHE:-0}" -eq 1 ]; then
  export CMAKE_C_COMPILER_LAUNCHER=ccache
  export CMAKE_CXX_COMPILER_LAUNCHER=ccache
fi

# install vcpkg
if [ ! -d "vcpkg/.git" ]; then
  git clone -q https://github.com/microsoft/vcpkg.git
  cd vcpkg
else
  cd vcpkg
  git pull -q
fi
./bootstrap-vcpkg.sh -disableMetrics
cd - 1>/dev/null
export VCPKG_ROOT=/data/vcpkg
export PATH="$VCPKG_ROOT:$PATH"

if [ "$DO_RELEASE" -eq 1 ]; then
  # install google breakpad tools
  vcpkg install breakpad[tools]
  export PATH="$VCPKG_ROOT/installed/x64-linux/tools/breakpad:$PATH"
fi

# install mob
## clone or pull
if [ ! -d "mob/.git" ]; then
  echo cloning mob...
  git clone -q --depth 1 https://github.com/Kaedras/mob.git
else
  if [ "$UPDATE_MOB" -eq 1 ]; then
    (cd mob && git pull -q)
  fi
fi

## build mob
if [ "$UPDATE_MOB" -eq 1 ] || [ ! -f "mob/mob" ]; then
  cd mob
  echo building mob...
  ./bootstrap.sh
fi

if [ "$DO_BUILD" -eq 1 ]; then
  echo building modorganizer...
  ./mob -d /data --log-file mob.build.log build
fi

if [ "$DO_RELEASE" -eq 1 ]; then
  echo creating release...
  ./mob -d /data release devbuild --appimage --log-file mob.release.log
fi

if [ "${USE_CCACHE:-0}" -eq 1 ]; then
  echo ccache stats:
  ccache --show-stats
fi
