FROM ubuntu:19.10 AS builder

RUN apt-get update && \
  apt-get install -y python3 build-essential cmake wget

RUN wget https://www.nsnam.org/release/ns-allinone-3.30.1.tar.bz2 && \
  tar xjf ns-allinone-3.30.1.tar.bz2 && \
  rm ns-allinone-3.30.1.tar.bz2

WORKDIR /ns-allinone-3.30.1/ns-3.30.1

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


FROM ubuntu:19.10

RUN apt-get update && \
  apt-get install -y net-tools iptables && \
  apt-get clean

WORKDIR /ns3
COPY --from=builder /ns-allinone-3.30.1/ns-3.30.1/out/src/fd-net-device/*optimized /ns-allinone-3.30.1/ns-3.30.1/out/src/fd-net-device/*debug /ns3/src/fd-net-device/
COPY --from=builder /ns-allinone-3.30.1/ns-3.30.1/out/scratch/*/* /ns3/scratch/
COPY --from=builder /ns-allinone-3.30.1/ns-3.30.1/out/lib/ /ns3/lib

# see https://gitlab.com/nsnam/ns-3-dev/issues/97
ENV PATH="/ns3/src/fd-net-device/:${PATH}"
ENV LD_LIBRARY_PATH="/ns3/lib"

COPY run.sh .
RUN chmod +x run.sh
RUN mkdir /logs

ENTRYPOINT [ "./run.sh" ]
