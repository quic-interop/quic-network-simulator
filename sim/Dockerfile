FROM ubuntu:24.04 AS builder

ARG TARGETARCH
RUN echo "TARGETARCH : $TARGETARCH"

ARG VERSION=unknown
RUN echo "VERSION : $VERSION"

RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y \
  python3 build-essential cmake ninja-build libboost-dev libgsl-dev libxml2-dev \
  libsqlite3-dev golang-go

ENV NS_VERS=3.46.1
ADD --checksum=sha256:fd3255df035a52008638fb5d81f77685260fd607be107eaf8e5116bcf1fd2c03 \
  https://www.nsnam.org/release/ns-$NS_VERS.tar.bz2 ns3.tar.bz2
RUN tar xjf ns3.tar.bz2 && rm ns3.tar.bz2
RUN mv /ns-$NS_VERS /ns3

WORKDIR /ns3

RUN mkdir out/
RUN ./ns3 configure --build-profile=release --out=out/

# make including of the QuicNetworkSimulatorHelper class possible
COPY CMakeLists.patch .
RUN patch -d scratch < CMakeLists.patch
COPY scenarios scratch/

# compile all the scenarios
RUN ./ns3 build

# strip ns3 version prefix from scratches
RUN find out/scratch -name "ns${NS_VERS}-*" | \
  sed -e 'p' -E -e "s|ns${NS_VERS}-*||g" | \
  xargs -n2 mv

COPY wait-for-it-quic /wait-for-it-quic
RUN cd /wait-for-it-quic && go build .

FROM ubuntu:24.04

ARG VERSION=unknown

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  net-tools iptables libgsl-dev libxml2 libsqlite3-0 wireshark-common && \
  apt-get autoremove -y && apt-get clean -y && \
  rm -rf /var/lib/apt/lists/*

WORKDIR /ns3
COPY --from=builder /ns3/out/src/fd-net-device/* /ns3/out/src/fd-net-device/
COPY --from=builder /ns3/out/scratch/*/* /ns3/scratch/
COPY --from=builder /ns3/out/lib/ /ns3/out/lib
COPY --from=builder /wait-for-it-quic/wait-for-it-quic /usr/bin

COPY run.sh .
RUN chmod +x run.sh
RUN mkdir /logs

RUN echo "$VERSION" > /version.txt

ENTRYPOINT [ "./run.sh" ]
