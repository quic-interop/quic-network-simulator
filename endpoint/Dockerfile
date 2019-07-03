FROM ubuntu:19.04

RUN apt-get update && \
  apt-get install -y net-tools iputils-ping tcpdump ethtool iperf

COPY run.sh .
RUN chmod +x run.sh
ENTRYPOINT ./run.sh
