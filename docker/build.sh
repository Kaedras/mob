#!/bin/sh

set -e

docker compose build
docker compose run build
