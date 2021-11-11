FROM ubuntu:20.04 AS qns-builder

RUN    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        ca-certificates \
        python3 \
        build-essential \
        cmake \
        wget \
    && apt-get clean

ENV VERS 3.32
RUN    wget --progress=dot:giga https://www.nsnam.org/release/ns-allinone-$VERS.tar.bz2 \
    && tar xjf ns-allinone-$VERS.tar.bz2 && rm ns-allinone-$VERS.tar.bz2 \
    && mv /ns-allinone-$VERS/ns-$VERS /ns3

WORKDIR /ns3

RUN    mkdir out/ \
    && ./waf configure \
        --build-profile=release \
        --out=out/ \
    && ./waf build

# make including of the QuicNetworkSimulatorHelper class possible
COPY wscript.patch .
RUN    patch < wscript.patch \
    && rm -r \
        scratch/subdir \
        scratch/scratch-simulator.cc

COPY scenarios scratch/

# compile all the scenarios
RUN    ./waf build \
    && cd out/lib \
    && du -sh . \
    && strip -v ./* \
    && du -sh . \
    && cd ../.. \
    && cd out/scratch \
    && rm -r \
        subdir \
        helper \
        scratch-simulator*

# ---

FROM ubuntu:20.04 AS wait-for-it-builder

RUN    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        wget \
        ca-certificates \
    && apt-get clean

WORKDIR /
RUN    wget --progress=dot:giga https://dl.google.com/go/go1.15.linux-amd64.tar.gz \
    && tar xfz go1.15.linux-amd64.tar.gz \
    && rm go1.15.linux-amd64.tar.gz

ENV PATH="/go/bin:${PATH}"
COPY wait-for-it-quic /wait-for-it-quic
WORKDIR /wait-for-it-quic
RUN go build .

# ---

FROM ubuntu:20.04

RUN    apt-get update \
    && apt-get install -y --no-install-recommends \
        net-tools \
        iptables \
        nftables \
    && apt-get clean

WORKDIR /ns3
COPY --from=qns-builder /ns3/out/src/fd-net-device/* /ns3/src/fd-net-device/
COPY --from=qns-builder /ns3/out/scratch/*/* /ns3/scratch/
COPY --from=qns-builder /ns3/out/lib/ /ns3/lib
COPY --from=wait-for-it-builder /wait-for-it-quic/wait-for-it-quic /usr/bin

# see https://gitlab.com/nsnam/ns-3-dev/issues/97
ENV PATH="/ns3/src/fd-net-device/:${PATH}"
ENV LD_LIBRARY_PATH="/ns3/lib"

COPY run.sh .
RUN    chmod +x run.sh \
    && mkdir /logs

ENTRYPOINT [ "./run.sh" ]
