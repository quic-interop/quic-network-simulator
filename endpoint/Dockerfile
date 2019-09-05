FROM ubuntu:19.04

RUN apt-get update && \
  apt-get install -y net-tools iputils-ping tcpdump ethtool iperf

COPY setup.sh .
RUN chmod +x setup.sh

COPY run_endpoint.sh .
RUN chmod +x run_endpoint.sh

ENTRYPOINT [ "/run_endpoint.sh" ]
