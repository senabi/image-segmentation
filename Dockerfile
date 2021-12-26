FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive
ENV PASS mpi
ENV HOME /root
RUN echo 'US/Eastern' > /etc/timezone

RUN apt-get update \
    && apt-get install -y \
    build-essential libfmt-dev locales wget \
    openssh-server libopenmpi-dev openmpi-bin openmpi-common openmpi-doc binutils \
    iproute2 iputils-ping net-tools \
    cmake vim tar \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8 \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get autoclean

RUN wget https://www.vpn.net/installers/logmein-hamachi_2.1.0.203-1_amd64.deb -O /tmp/hamachi.deb \
    && dpkg -i /tmp/hamachi.deb

RUN mkdir /var/run/sshd

RUN echo "root:$PASS" | chpasswd \
    && echo "export TERM=vt100" >> ${HOME}/.bashrc

COPY parallel-rank $HOME/parallel-rank
COPY init.sh $HOME/init.sh
COPY init-vpn.sh $HOME/init-vpn.sh
COPY add-node.sh $HOME/add-node.sh

RUN mkdir -p $HOME/parallel-rank/build \
    && cd $HOME/parallel-rank/build \
    && cmake .. \
    && make \
    && cp $HOME/parallel-rank/build/parallel-rank /usr/local/bin/parallel-rank

RUN echo "    CheckHostIP no\n    StrictHostKeyChecking no" >> /etc/ssh/ssh_config
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/#\?\(PermitRootLogin\s*\).*$/\1 without-password/' /etc/ssh/sshd_config
RUN sed -i 's/#\?\(PermitRootLogin\s*\).*$/\1 yes/' /etc/ssh/sshd_config
RUN chmod +x $HOME/*.sh

WORKDIR $HOME
EXPOSE 22