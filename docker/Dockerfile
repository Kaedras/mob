FROM ubuntu:24.04

WORKDIR /root

# upgrade packages
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get full-upgrade -y

# install gcc 16
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y --no-install-recommends software-properties-common gpg-agent \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get update \
    && apt-get install -y --no-install-recommends gcc-16 g++-16

# set gcc 16 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-16 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-16 100 && \
    update-alternatives --install /usr/bin/x86_64-linux-gnu-gcc x86_64-linux-gnu-gcc /usr/bin/gcc-16 100 && \
    update-alternatives --install /usr/bin/x86_64-linux-gnu-g++ x86_64-linux-gnu-g++ /usr/bin/g++-16 100

# install python 3.13
RUN add-apt-repository -y ppa:deadsnakes/ppa && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      python3.13-full \
      python3.13-venv \
      libpython3.13-dev && \
    python3.13 -m ensurepip --upgrade

# set python 3.13 as default
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.13 100

# add kitware apt repository for newer cmake versions. see https://apt.kitware.com/ for more details
# while the flag for c++26 has been added in cmake 3.25, it is only functional in cmake 4.2 and later
RUN apt-get install -y --no-install-recommends ca-certificates gpg wget && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor - > /usr/share/keyrings/kitware-archive-keyring.gpg && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' > /etc/apt/sources.list.d/kitware.list && \
    apt-get update && \
    rm /usr/share/keyrings/kitware-archive-keyring.gpg && \
    apt-get install -y kitware-archive-keyring

# install dependencies
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update && apt-get install -y --no-install-recommends \
    7zip \
    appstream \
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
    unzip \
    wget \
    zip \
    zstd

# install qt
ARG QT_VERSION="6.11.1"
ARG QT_MODULES="qtnetworkauth qtpositioning qtserialport qttasktree qtwaylandcompositor qtwebchannel qtwebengine qtwebsockets"
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
RUN apt-get install -y --no-install-recommends rustup && rustup default stable

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
