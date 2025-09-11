#!/bin/sh

set -e

VERBOSE=0
CONFIG="Release"

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
