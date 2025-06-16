FROM ubuntu:24.04

RUN apt update && apt install -y \
    g++ cmake libboost-all-dev libssl-dev wget curl

# Add source
WORKDIR /app
COPY . /app

# Download json.hpp manually
RUN mkdir -p include && \
    curl -L https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp -o include/json.hpp

# Build
RUN cmake . && make
# Expose port
EXPOSE 9798

# Run
CMD ["./CppServer"]
