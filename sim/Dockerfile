FROM ubuntu:19.04

RUN apt-get update && \
  apt-get install -y git mercurial python unzip unrar build-essential cmake wget \
  net-tools tcpdump iptables iproute2 iputils-ping vim

RUN wget https://www.nsnam.org/release/ns-allinone-3.29.tar.bz2 && \
  tar xjf ns-allinone-3.29.tar.bz2

WORKDIR /ns-allinone-3.29/ns-3.29

RUN ./waf configure --build-profile=optimized
RUN ./waf build

COPY wscript.patch .
RUN patch < wscript.patch

RUN rm -r scratch/subdir scratch/scratch-simulator.cc
COPY helper scratch/helper
COPY sim.cc scratch/

COPY run.sh .
RUN chmod +x run.sh

ENTRYPOINT [ "./run.sh" ]
