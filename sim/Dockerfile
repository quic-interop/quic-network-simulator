FROM ubuntu:19.04

RUN apt-get update && \
  apt-get install -y git mercurial python unzip unrar build-essential cmake wget \
  net-tools tcpdump iptables iproute2 iputils-ping vim

RUN wget https://www.nsnam.org/release/ns-allinone-3.29.tar.bz2 && \
  tar xjf ns-allinone-3.29.tar.bz2

WORKDIR /ns-allinone-3.29/ns-3.29

RUN ./waf configure --build-profile=optimized
RUN ./waf build

RUN mkdir ./scratch/sim
COPY quic-network-simulator-helper.h quic-network-simulator-helper.cc ./scratch/sim/  
COPY sim.cc ./scratch/sim

COPY run.sh .
RUN chmod +x run.sh

ENTRYPOINT [ "./run.sh" ]
