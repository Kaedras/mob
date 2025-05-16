#!/bin/sh

set -e

VERBOSE=0
CONFIG="Release"

usage() {
  echo ""
  echo "Usage: $0 [-v] [-c <config>]"
  echo "Options:"
  echo "  -v            Enable verbose output"
  echo "  -c <config>   Set the configuration (Debug, RelWithDebInfo, Release)"
  exit 1
}

while getopts "vc:" opt; do
  case "$opt" in
    v)
      VERBOSE=1
      ;;
    c)
      case "$OPTARG" in
        Debug|RelWithDebInfo|Release)
          CONFIG="$OPTARG"
          ;;
        *)
          echo "Invalid configuration: $OPTARG"
          usage
          ;;
      esac
      ;;
    *)
      usage
      ;;
  esac
done

ROOT="$(dirname "$0")"
if [ -z "$ROOT" ]; then
  ROOT="."
fi

if [ "$VERBOSE" -eq 1 ]; then
  cmake -B "$ROOT/build" "$ROOT" -DCMAKE_BUILD_TYPE="$CONFIG"
  cmake --build "$ROOT/build" -j "$(nproc)"
else
  cmake -B "$ROOT/build" "$ROOT" --log-level=ERROR -DCMAKE_CXX_FLAGS="-w" -DCMAKE_BUILD_TYPE="$CONFIG" -DCMAKE_RULE_MESSAGES=OFF 1> /dev/null
  cmake --build "$ROOT/build" -j "$(nproc)" -- --quiet
fi

EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ]; then
  echo "Build failed" >&2
  exit $EXIT_CODE
fi

cp "$ROOT/build/src/mob" "$ROOT/mob"
echo "run ``./mob -d prefix/path build`` to start building"
