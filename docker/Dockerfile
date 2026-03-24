FROM ubuntu:24.04

WORKDIR /root

# upgrade packages
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get full-upgrade -y

# install gcc 15
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends software-properties-common gpg-agent \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get install -y --no-install-recommends gcc-15 g++-15

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-15 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-15 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-15 100

# install dependencies
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    autoconf automake autoconf-archive ccache cmake ninja-build curl zip unzip libssl-dev libcurl4-openssl-dev git 7zip libcups2-dev libfontconfig1 libfuse3-dev libgl1-mesa-dev libsecret-1-dev libxkbcommon-dev python-is-python3 python3 python3-pip python3-venv libpython3-dev wget zstd \
    libnspr4 libnss3 libxrandr-dev libxi-dev libxtst-dev libxfixes-dev libxdamage-dev libasound-dev libxkbfile-dev libxcomposite-dev libxkbcommon-x11-dev libxcb-cursor-dev libxcb-icccm4 libxcb-keysyms1 libxcb-shape0-dev libfbclient2 libwayland-cursor0 libwayland-egl1 \
    fuse3 file

# install qt
ARG QT_VERSION="6.7.3"
ARG QT_MODULES="qtpositioning qtserialport qtwebchannel qtwebengine qtwebsockets qtnetworkauth qtwaylandcompositor"
RUN python -m venv venv \
    && . venv/bin/activate \
    && pip install aqtinstall \
    && aqt install-qt --outputdir /opt/qt linux desktop ${QT_VERSION} linux_gcc_64 -m ${QT_MODULES}

ENV QT_ROOT_DIR="/opt/qt/${QT_VERSION}/gcc_64"
ENV CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:+${CMAKE_PREFIX_PATH}:}${QT_ROOT_DIR}"
ENV QT_PLUGIN_PATH="${QT_ROOT_DIR}/plugins"
ENV QML2_IMPORT_PATH="${QT_ROOT_DIR}/qml"
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+${LD_LIBRARY_PATH}:}${QT_ROOT_DIR}/lib"
ENV PKG_CONFIG_PATH="${PKG_CONFIG_PATH:+${PKG_CONFIG_PATH}:}${QT_ROOT_DIR}/lib/pkgconfig"
ENV PATH="${QT_ROOT_DIR}/bin:$PATH"

# install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git \
    && cd vcpkg && ./bootstrap-vcpkg.sh -disableMetrics
ENV VCPKG_ROOT=/root/vcpkg
ENV PATH="$VCPKG_ROOT:$PATH"

# install rust (required for loot)
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:$PATH"

# install google breakpad tools
RUN vcpkg install breakpad[tools]
env PATH="$VCPKG_ROOT/installed/x64-linux/tools/breakpad:$PATH"

# define volumes
VOLUME /dev/fuse
VOLUME /root/build
VOLUME /root/ccache
VOLUME /root/mob
VOLUME /root/vcpkg/buildtrees
VOLUME /root/vcpkg/downloads
VOLUME /root/vcpkg/installed
VOLUME /root/vcpkg/packages

ENV CCACHE_DIR=/root/ccache

# set entrypoint
COPY ./entrypoint.sh /root/entrypoint.sh
RUN chmod +x /root/entrypoint.sh
ENTRYPOINT [ "bash", "./entrypoint.sh" ]
