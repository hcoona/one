FROM ubuntu:22.04

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -qq -y \
    clang-14 lld-14 libfuzzer-14-dev libc++-14-dev libc++abi-14-dev libunwind-14-dev \
    && apt-get autoremove -y && apt-get clean && rm -rf /var/lib/apt/lists/*
