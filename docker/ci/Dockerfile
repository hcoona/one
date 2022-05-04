FROM ubuntu:20.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qq -y \
    apt-transport-https ca-certificates wget lsb-release software-properties-common build-essential python3-distutils \
    && wget https://apt.llvm.org/llvm.sh -O llvm.sh && chmod +x llvm.sh && env DEBIAN_FRONTEND=noninteractive ./llvm.sh 14 && rm -f llvm.sh \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qq -y \
    lld-14 libfuzzer-14-dev libc++-14-dev libc++abi-14-dev libunwind-14-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*
