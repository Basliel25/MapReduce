# Build
FROM debian:bookworm-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY Makefile ./
COPY src/ ./src/

RUN make

# Run
FROM debian:bookworm-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
        valgrind \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /src/MapReduce /app/MapReduce

ENTRYPOINT ["/app/MapReduce"]
CMD []
