# --- Build Stage ---
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV APP_HOME=/app
WORKDIR $APP_HOME

# --- Toolchain and system dependencies ---
# Use a closer Ubuntu mirror; the default archive.ubuntu.com is often slow/flaky.
RUN sed -i 's|archive.ubuntu.com|in.archive.ubuntu.com|g' /etc/apt/sources.list.d/ubuntu.sources && \
    apt-get update && apt-get install -y \
    clang build-essential git cmake \
    uuid-dev libssl-dev libpq-dev libjsoncpp-dev zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

ENV CC=clang
ENV CXX=clang++

# --- Build third-party libs from source ---
RUN git clone --no-checkout https://github.com/jbeder/yaml-cpp.git \
    && cd yaml-cpp \
    && git fetch --depth 1 origin 89ff142b991af432b5d7a7cee55282f082a7e629 \
    && git checkout 89ff142b991af432b5d7a7cee55282f082a7e629 \
    && cmake -S . -B build \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. \
    && rm -rf yaml-cpp

RUN git clone --branch v0.22.0 --depth 1 https://github.com/getml/reflect-cpp.git && \
    cd reflect-cpp && \
    cmake -S . -B build \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc) && \
    cmake --install build && \
    cd .. && rm -rf reflect-cpp

RUN git clone --branch v1.9.11 --depth 1 https://github.com/drogonframework/drogon.git && \
    cd drogon && \
    git submodule update --init --recursive && \
    cmake -S . -B build \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_CXX_STANDARD=20 \
        -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc) && \
    cmake --install build && \
    cd .. && rm -rf drogon

# --- Build the application ---
COPY . .

RUN cmake -S . -B build \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)

# --- Collect runtime libs ---
RUN mkdir -p /runtime_libs && \
    ldd /app/build/ApiServer | awk '{print $3}' | grep -E '^/' | xargs -I '{}' cp -n '{}' /runtime_libs/ || true && \
    cp -rv /usr/local/lib/*.so* /runtime_libs/ || true

# --- Runtime Stage ---
FROM ubuntu:24.04

WORKDIR /app

RUN sed -i 's|archive.ubuntu.com|in.archive.ubuntu.com|g' /etc/apt/sources.list.d/ubuntu.sources && \
    apt-get update && apt-get install -y \
    libssl3 libpq5 libjsoncpp25 libuuid1 zlib1g && \
    rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/ApiServer /app/ApiServer
COPY --from=builder /runtime_libs /usr/local/lib/

RUN ldconfig

EXPOSE 5555
ENTRYPOINT ["/app/ApiServer"]
