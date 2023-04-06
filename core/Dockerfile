FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

USER root

RUN apt-get update && TZ=Europe && DEBIAN_FRONTEND=noninteractive && apt-get install -y \
    build-essential \
    zip \
    git \
    tar \
    make \
    unzip \
    wget \
    curl \
    libgrpc-dev \
    libboost-all-dev \
    libssl-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    pkg-config \
    protobuf-compiler-grpc

# Set up vcpkg and basic dependecies
RUN git clone https://github.com/Microsoft/vcpkg.git \
    && cd /vcpkg \
    && ./bootstrap-vcpkg.sh \
    && ./vcpkg install nlohmann-json:x64-linux boost:x64-linux

RUN cd usr && cd local && cd include \
    && mkdir etcd && mkdir pplx && mkdir cpprest

#Install cmake 3.26
RUN wget https://github.com/Kitware/CMake/releases/download/v3.26.0/cmake-3.26.0.tar.gz
RUN tar xvfz cmake-3.26.0.tar.gz
RUN cd cmake-3.26.0 && ./bootstrap
RUN cd cmake-3.26.0 && make && make install
RUN cd /usr/bin && ln -s cmake-3.26.0/bin/cmake cmake && ln -s vcpkg/vcpkg vcpkg

# Set up etcd-cpp-apiv3
RUN git clone https://github.com/microsoft/cpprestsdk.git \
    && cd cpprestsdk \
    && mkdir build && cd build \
    && cmake .. -DCPPREST_EXCLUDE_WEBSOCKETS=ON \
    && make -j$(nproc) && make install

RUN git clone https://github.com/etcd-cpp-apiv3/etcd-cpp-apiv3 \
    && cd etcd-cpp-apiv3 \
    && mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) && make install

# Copy project and compile
WORKDIR /memdb
COPY . /memdb
RUN chmod +x /memdb/linux-build.sh && /memdb/linux-build.sh

# Update dynamic library cache
RUN ldconfig

ENTRYPOINT ["/memdb/src/build/src/memdb_run"]