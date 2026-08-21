#!/bin/sh

set -e

if ! command -v podman >/dev/null 2>&1
then
  COMMAND=docker
  echo "using docker"
else
  COMMAND=podman
  echo "using podman"
fi

$COMMAND compose build
$COMMAND compose run --rm build
