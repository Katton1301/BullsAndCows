FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    wget \
    libssl-dev \
    libsasl2-dev \
    zlib1g-dev \
    librdkafka-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /usr/include/rapidjson && \
    wget -qO- https://github.com/Tencent/rapidjson/archive/master.tar.gz | \
    tar xz -C /tmp && \
    cp -r /tmp/rapidjson-master/include/rapidjson/* /usr/include/rapidjson/ && \
    rm -rf /tmp/rapidjson-master

WORKDIR /app
COPY . .

RUN cd projects && mkdir -p build && cd build && \
    cmake \
    -DBUILD_SERVER=ON \
    -DBUILD_SIMULATION=OFF \
    -DBUILD_TESTS=OFF \
    -DBUILD_GAMEVIEW=OFF \
    -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    librdkafka++1 \
    libssl3 \
    libsasl2-2 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/projects/build/ .
COPY .env .

CMD ["./BullsAndCows.Server"]