# Readme

This directory contains files to build ModOrganizer inside an ubuntu:24.04 container.
The main reason for creating this is to build AppImages on an older base system,
see [here](https://docs.appimage.org/reference/best-practices.html#binaries-compiled-on-old-enough-base-system) for more
details.

## Requirements

Podman (including podman-compose) or docker

## Usage

```shell
# may be necessary if podman is installed without a wrapper
alias docker=podman

./build.sh
```

## Configuration

There are some options that can be tweaked by editing `.env`:

- Output directory
- Enable/disable ccache
- Extra qt plugins
- Extra qt platform plugins
- Exclude libraries from appimage
