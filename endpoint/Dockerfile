FROM ubuntu:20.04

RUN apt-get update && \
  apt-get install -y wget net-tools iputils-ping tcpdump ethtool iperf iproute2

COPY setup.sh .
RUN chmod +x setup.sh

COPY run_endpoint.sh .
RUN chmod +x run_endpoint.sh

RUN wget https://raw.githubusercontent.com/vishnubob/wait-for-it/master/wait-for-it.sh && chmod +x wait-for-it.sh

ENTRYPOINT [ "/run_endpoint.sh" ]
