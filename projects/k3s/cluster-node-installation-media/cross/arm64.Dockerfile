# Cross build environment for arm64 (aarch64) packages of the install medium on
# an amd64 machine: Ubuntu 22.04 with the aarch64 cross compilers, the arm64
# libraries the programs link against and run with, rpmbuild for RPM packages.
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN sed -i 's#^deb http#deb [arch=amd64] http#' /etc/apt/sources.list && \
    printf 'deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports jammy main universe\ndeb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports jammy-updates main universe\ndeb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports jammy-security main universe\n' \
        > /etc/apt/sources.list.d/arm64.list && \
    dpkg --add-architecture arm64 && apt-get update && \
    apt-get install -y --no-install-recommends crossbuild-essential-arm64 cmake ninja-build python3 rpm file \
        libx11-dev:arm64 libssl3:arm64 libstdc++6:arm64 libgcc-s1:arm64 && \
    rm -rf /var/lib/apt/lists/*
