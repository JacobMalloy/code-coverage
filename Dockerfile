FROM ubuntu:20.04

# Use non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    clang \
    lldb \
    gdb \
    git \
    wget \
    curl \
    unzip \
    python3 \
    python3-pip \
    libxml2-dev \
    zlib1g-dev \
    pkg-config \
    libedit-dev \
    libncurses5-dev \
    libtool \
    binutils-dev \
    lsb-release \
    software-properties-common \
    libzstd-dev \
    ubuntu-dbgsym-keyring \
    vim \

# Install LLVM 12
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 18 && \
    rm llvm.sh

RUN rm -rf /var/lib/apt/lists/*

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

ENV PATH="/root/.cargo/bin:${PATH}"

# Set environment variables for LLVM 12
ENV CC=clang-18
ENV CXX=clang++-18
ENV LLVM_DIR=/usr/lib/llvm-18
ENV PATH="${LLVM_DIR}/bin:$PATH"

# Create a working directory
WORKDIR /workspace

# Set default command to bash
CMD ["/bin/bash"]

