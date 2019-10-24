FROM ubuntu:19.10

RUN apt-get update && \
  apt-get install -y git mercurial python3 unzip unrar build-essential cmake wget \
  gdb net-tools tcpdump iptables iproute2 iputils-ping vim

RUN wget https://www.nsnam.org/release/ns-allinone-3.30.1.tar.bz2 && \
  tar xjf ns-allinone-3.30.1.tar.bz2 && \
  rm ns-allinone-3.30.1.tar.bz2

WORKDIR /ns-allinone-3.30.1/ns-3.30.1

RUN ./waf configure --build-profile=optimized
RUN ./waf build

# make including of the QuicNetworkSimulatorHelper class possible
COPY wscript.patch .
RUN patch < wscript.patch

RUN rm -r scratch/subdir scratch/scratch-simulator.cc
COPY scenarios scratch/

# compile all the scenarios
RUN ./waf build

COPY run.sh .
RUN chmod +x run.sh

ENTRYPOINT [ "./run.sh" ]
