FROM ubuntu:20.04 AS builder

ARG TARGETPLATFORM
RUN echo "TARGETPLATFORM : $TARGETPLATFORM"

RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y python3 build-essential cmake wget

ENV VERS 3.40
RUN wget https://www.nsnam.org/release/ns-allinone-$VERS.tar.bz2
RUN tar xjf ns-allinone-$VERS.tar.bz2 && rm ns-allinone-$VERS.tar.bz2
RUN mv /ns-allinone-$VERS/ns-$VERS /ns3

WORKDIR /ns3

RUN rm -r scratch/nested-subdir/ scratch/subdir/ scratch/scratch-simulator.cc
COPY scenarios/ scratch/
# apply some build patches to allow for generated executable filenames
# be created without ns3 version numbers
COPY macros-and-definitions.cmake.patch ./build-support/
COPY fd-net-device-CMakeLists.txt.patch ./src/fd-net-device/
RUN cd ./build-support && patch < macros-and-definitions.cmake.patch
RUN cd ./src/fd-net-device/ && patch < fd-net-device-CMakeLists.txt.patch

RUN mkdir out/
RUN ./ns3 configure --build-profile=release --disable-werror --out=out/
RUN ./ns3 build
RUN cd out/lib && strip -v *

RUN if [ "$TARGETPLATFORM" = "linux/amd64" ]; then cd / && \
      wget https://dl.google.com/go/go1.21.1.linux-amd64.tar.gz && \
      tar xfz go1.21.1.linux-amd64.tar.gz && \
      rm go1.21.1.linux-amd64.tar.gz; \
    fi

RUN if [ "$TARGETPLATFORM" = "linux/arm64" ]; then cd / && \
      wget https://dl.google.com/go/go1.21.1.linux-arm64.tar.gz && \
      tar xfz go1.21.1.linux-arm64.tar.gz && \
      rm go1.21.1.linux-arm64.tar.gz; \
    fi

ENV PATH="/go/bin:${PATH}"
COPY wait-for-it-quic /wait-for-it-quic
RUN cd /wait-for-it-quic && go build .

FROM ubuntu:20.04

RUN apt-get update && \
  apt-get install -y net-tools iptables && \
  apt-get clean

WORKDIR /ns3
COPY --from=builder /ns3/out/src/fd-net-device/* /ns3/src/fd-net-device/
COPY --from=builder /ns3/out/scratch/*/* /ns3/scratch/
COPY --from=builder /ns3/out/lib/ /ns3/lib
COPY --from=builder /wait-for-it-quic/wait-for-it-quic /usr/bin

# see https://gitlab.com/nsnam/ns-3-dev/issues/97
ENV PATH="/ns3/src/fd-net-device/:${PATH}"
ENV LD_LIBRARY_PATH="/ns3/lib"

COPY run.sh .
RUN chmod +x run.sh
RUN mkdir /logs

ENTRYPOINT [ "./run.sh" ]
