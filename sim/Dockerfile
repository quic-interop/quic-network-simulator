FROM ubuntu:24.04 AS builder

ARG TARGETARCH
RUN echo "TARGETARCH : $TARGETARCH"

RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y \
  python3 build-essential cmake ninja-build libboost-dev libgsl-dev libxml2-dev \
  libsqlite3-dev

ENV NS_VERS 3.41
ADD --checksum=sha256:ce6a2c6a91d6979999da0a889cf0aa5db8798f387edf20672af9bcd966277aa9 \
  https://www.nsnam.org/release/ns-allinone-$NS_VERS.tar.bz2 ns3.tar.bz2
RUN tar xjf ns3.tar.bz2 && rm ns3.tar.bz2
RUN mv /ns-allinone-$NS_VERS/ns-$NS_VERS /ns3

ENV GO_VERS 1.22.3
ADD --checksum=sha256:6c33e52a5b26e7aa021b94475587fce80043a727a54ceb0eee2f9fc160646434 \
  https://dl.google.com/go/go$GO_VERS.linux-$TARGETARCH.tar.gz go.tar.gz
RUN tar xfz go.tar.gz && rm go.tar.gz

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

ENV PATH="/go/bin:${PATH}"
COPY wait-for-it-quic /wait-for-it-quic
RUN cd /wait-for-it-quic && go build .

FROM ubuntu:24.04

RUN apt-get update && \
  apt-get install -y net-tools iptables && \
  apt-get clean

WORKDIR /ns3
COPY --from=builder /ns3/out/src/fd-net-device/* /ns3/out/src/fd-net-device/
COPY --from=builder /ns3/out/scratch/*/* /ns3/scratch/
COPY --from=builder /ns3/out/lib/ /ns3/out/lib
COPY --from=builder /wait-for-it-quic/wait-for-it-quic /usr/bin

COPY run.sh .
RUN chmod +x run.sh
RUN mkdir /logs

ENTRYPOINT [ "./run.sh" ]
