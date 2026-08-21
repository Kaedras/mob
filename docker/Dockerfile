ARG BASE_IMAGE=ubuntu:24.04
FROM ${BASE_IMAGE}

ARG BASE_IMAGE

WORKDIR /root

# upgrade packages
RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get full-upgrade -y

# run container-specific setup
COPY ./setup.sh /tmp/setup.sh
RUN chmod +x /tmp/setup.sh && /tmp/setup.sh "$BASE_IMAGE"

# set gcc 16 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-16 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-16 100 && \
    update-alternatives --install /usr/bin/x86_64-linux-gnu-gcc x86_64-linux-gnu-gcc /usr/bin/gcc-16 100 && \
    update-alternatives --install /usr/bin/x86_64-linux-gnu-g++ x86_64-linux-gnu-g++ /usr/bin/g++-16 100

# install python
ARG PYTHON_VERSION="3.14"
RUN add-apt-repository -y ppa:deadsnakes/ppa && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
      python${PYTHON_VERSION}-full \
      python${PYTHON_VERSION}-venv \
      libpython${PYTHON_VERSION}-dev

# set new python as default
RUN update-alternatives --install /usr/bin/python python /usr/bin/python${PYTHON_VERSION} 100

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
VOLUME /data

WORKDIR /data

# set entrypoint
COPY ./entrypoint.sh /root/entrypoint.sh
RUN chmod +x /root/entrypoint.sh
ENTRYPOINT [ "bash", "/root/entrypoint.sh" ]
