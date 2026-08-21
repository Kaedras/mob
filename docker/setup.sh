#!/bin/sh

set -e

BASE_IMAGE="$1"

export DEBIAN_FRONTEND=noninteractive
apt-get install -y --no-install-recommends software-properties-common

case "$BASE_IMAGE" in
  ubuntu:24.04)
    # add ppa for gcc 16
    apt-get install -y --no-install-recommends gpg-agent
    add-apt-repository -y ppa:ubuntu-toolchain-r/test
    apt-get update

    # add kitware apt repository for newer cmake versions. see https://apt.kitware.com/ for more details.
    # while the flag for c++26 has been added in cmake 3.25, it is only functional in cmake 4.2 and later
    apt-get install -y --no-install-recommends ca-certificates gpg wget
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor - > /usr/share/keyrings/kitware-archive-keyring.gpg
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' > /etc/apt/sources.list.d/kitware.list
    apt-get update
    rm /usr/share/keyrings/kitware-archive-keyring.gpg
    apt-get install -y kitware-archive-keyring

    ;;
  ubuntu:26.04)

    ;;
  *)
    echo "unknown base image \"$BASE_IMAGE\""
    exit 1
    ;;
esac

apt-get install -y --no-install-recommends gcc-16 g++-16
