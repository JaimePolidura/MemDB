FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

USER root

RUN apt-get update && TZ=Europe && DEBIAN_FRONTEND=noninteractive && apt-get install -y \
    build-essential \
    zip \
    git \
    make \
    cmake \
    unzip \
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
    && ./vcpkg install nlohmann-json:x64-linux boost:x64-linux \
    && cd ..

# Set up etcd-cpp-apiv3
RUN git clone https://github.com/microsoft/cpprestsdk.git \
    && cd cpprestsdk \
    && mkdir build && cd build \
    && cmake .. -DCPPREST_EXCLUDE_WEBSOCKETS=ON \
    && make -j$(nproc) && make install \
    && cd .. \

RUN git clone https://github.com/etcd-cpp-apiv3/etcd-cpp-apiv3 \
    && cd etcd-cpp-apiv3 \
    && mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc) && make install \
    && cd ..

# Copy file
COPY src/build/src/memdb_run .

ENTRYPOINT ["./memdb_run"]
