#!/bin/sh

set -e

VERBOSE=0
CONFIG="Release"

if [ -z $VCPKG_ROOT ]; then
  printf "\033[0;31m\$VCPKG_ROOT is not set\n"
  printf "Please install vcpkg and set \$VCPKG_ROOT to its installation directory\033[m\n"
  exit 1
fi

usage() {
  echo ""
  echo "Usage: $(basename $0) [-Verbose] [-Config <config>]"
  echo "Options:"
  echo "  -Verbose           Enable verbose output"
  echo "  -Config <config>   Set the configuration (Debug, RelWithDebInfo, Release)"
  exit 1
}

# Parse arguments
while [ $# -gt 0 ]; do
  case "$1" in
    -Verbose)
      VERBOSE=1
      shift 1
      ;;
    -Config)
      case "$2" in
        Debug|RelWithDebInfo|Release)
          CONFIG="$2"
          ;;
        *)
          echo "Invalid configuration: $2"
          usage
          ;;
      esac
      shift 2
      ;;
    *)
      echo "Unknown option: $1"
      usage
      ;;
  esac
done

ROOT="$(dirname "$0")"
if [ -z "$ROOT" ]; then
  ROOT="."
fi

if [ "$VERBOSE" -eq 1 ]; then
  LOG_LEVEL="STATUS"
  VERBOSE_ARG="--verbose"
else
  LOG_LEVEL="ERROR"
  VERBOSE_ARG=""
fi

cmake --preset vcpkg-linux --log-level=$LOG_LEVEL
cmake --build --preset "${CONFIG}-linux" -j "$(nproc)" $VERBOSE_ARG

EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ]; then
  echo "Build failed" >&2
  exit $EXIT_CODE
fi

cp "$ROOT/build/src/$CONFIG/mob" "$ROOT/mob"
echo "run \`./mob -d prefix/path build\` to start building"
