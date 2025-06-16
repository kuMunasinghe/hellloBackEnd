FROM ubuntu:24.04

RUN apt update && apt install -y \
    g++ cmake libboost-all-dev libssl-dev wget

# Add source
WORKDIR /app
COPY . /app

# Build
RUN cmake . && make

# Expose port
EXPOSE 9798

# Run
CMD ["./CppServer"]
