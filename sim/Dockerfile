FROM ubuntu:20.04 AS builder

RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install -y python3 build-essential cmake wget

ENV VERS 3.31
RUN wget https://www.nsnam.org/release/ns-allinone-$VERS.tar.bz2
RUN tar xjf ns-allinone-$VERS.tar.bz2 && rm ns-allinone-$VERS.tar.bz2
RUN mv /ns-allinone-$VERS/ns-$VERS /ns3

WORKDIR /ns3

RUN mkdir out/
RUN ./waf configure --build-profile=optimized --out=out/
RUN ./waf build

# make including of the QuicNetworkSimulatorHelper class possible
COPY wscript.patch .
RUN patch < wscript.patch

RUN rm -r scratch/subdir scratch/scratch-simulator.cc
COPY scenarios scratch/

# compile all the scenarios
RUN ./waf build  && \
  cd out/lib && du -sh . && strip -v * && du -sh . && cd ../.. && \
  cd out/scratch && rm -r subdir helper scratch-simulator*

FROM ubuntu:20.04

RUN apt-get update && \
  apt-get install -y net-tools iptables && \
  apt-get clean

WORKDIR /ns3
COPY --from=builder /ns3/out/src/fd-net-device/*optimized /ns3/out/src/fd-net-device/*debug /ns3/src/fd-net-device/
COPY --from=builder /ns3/out/scratch/*/* /ns3/scratch/
COPY --from=builder /ns3/out/lib/ /ns3/lib

# see https://gitlab.com/nsnam/ns-3-dev/issues/97
ENV PATH="/ns3/src/fd-net-device/:${PATH}"
ENV LD_LIBRARY_PATH="/ns3/lib"

COPY run.sh .
RUN chmod +x run.sh
RUN mkdir /logs

ENTRYPOINT [ "./run.sh" ]
