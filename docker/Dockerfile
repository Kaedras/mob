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

# set gcc 15 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-15 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-15 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-15 100

# install dependencies
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends \
    7zip \
    autoconf \
    autoconf-archive \
    automake \
    ccache \
    cmake \
    curl \
    file \
    fuse3 \
    git \
    libasound-dev \
    libcups2-dev \
    libcurl4-openssl-dev \
    libfbclient2 \
    libfontconfig1 \
    libfuse3-dev \
    libgl1-mesa-dev \
    libnspr4 \
    libnss3 \
    libpython3-dev \
    libsecret-1-dev \
    libssl-dev \
    libwayland-cursor0 \
    libwayland-egl1 \
    libxcb-cursor-dev \
    libxcb-icccm4 \
    libxcb-keysyms1 \
    libxcb-shape0-dev \
    libxcomposite-dev \
    libxdamage-dev \
    libxfixes-dev \
    libxi-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libxkbfile-dev \
    libxrandr-dev \
    libxtst-dev \
    ninja-build \
    python-is-python3 \
    python3 \
    python3-pip \
    python3-venv \
    unzip \
    wget \
    zip \
    zstd

# install qt
ARG QT_VERSION="6.7.3"
ARG QT_MODULES="qtpositioning qtserialport qtwebchannel qtwebengine qtwebsockets qtnetworkauth qtwaylandcompositor"
RUN python -m venv venv \
    && . venv/bin/activate \
    && pip install aqtinstall \
    && aqt install-qt --outputdir /opt/qt linux desktop ${QT_VERSION} linux_gcc_64 -m ${QT_MODULES}

# set environment variables
ENV QT_ROOT_DIR="/opt/qt/${QT_VERSION}/gcc_64"
ENV CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:+${CMAKE_PREFIX_PATH}:}${QT_ROOT_DIR}"
ENV QT_PLUGIN_PATH="${QT_ROOT_DIR}/plugins"
ENV QML2_IMPORT_PATH="${QT_ROOT_DIR}/qml"
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+${LD_LIBRARY_PATH}:}${QT_ROOT_DIR}/lib"
ENV PKG_CONFIG_PATH="${PKG_CONFIG_PATH:+${PKG_CONFIG_PATH}:}${QT_ROOT_DIR}/lib/pkgconfig"
ENV PATH="${QT_ROOT_DIR}/bin:$PATH"
ENV CCACHE_DIR=/root/ccache

# install rust (required for loot)
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:$PATH"

# define volumes
VOLUME /dev/fuse
VOLUME /root/build
VOLUME /root/ccache
VOLUME /root/mob
VOLUME /root/vcpkg

# set entrypoint
COPY ./entrypoint.sh /root/entrypoint.sh
RUN chmod +x /root/entrypoint.sh
ENTRYPOINT [ "bash", "./entrypoint.sh" ]
