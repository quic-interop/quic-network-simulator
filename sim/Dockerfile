FROM ubuntu:24.04 AS builder

ARG TARGETARCH
RUN echo "TARGETARCH : $TARGETARCH"

RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y \
  python3 build-essential cmake wget ninja-build libboost-dev libgsl-dev libxml2-dev \
  libsqlite3-dev

ENV NS_VERS 3.41
RUN wget -O ns3.tar.bz2 https://www.nsnam.org/release/ns-allinone-$NS_VERS.tar.bz2
RUN tar xjf ns3.tar.bz2 && rm ns3.tar.bz2
RUN mv /ns-allinone-$NS_VERS/ns-$NS_VERS /ns3

ENV GO_VERS 1.22.3
RUN wget -O go.tar.gz https://dl.google.com/go/go$GO_VERS.linux-$TARGETARCH.tar.gz
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
