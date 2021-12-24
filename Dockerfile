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

RUN echo "root:${PASS}" | chpasswd \
    && echo "export TERM=vt100" >> ${HOME}/.bashrc

COPY parallel-rank ${HOME}/parallel-rank-with-mpi-x

RUN mkdir -p ${HOME}/parallel-rank-with-mpi-x/build \
    && cd ${HOME}/parallel-rank-with-mpi-x/build \
    && cmake .. \
    && make \
    && cp ${HOME}/parallel-rank-with-mpi-x/build/parallel-rank ${HOME}

RUN echo "    CheckHostIP no\n    StrictHostKeyChecking no" >> /etc/ssh/ssh_config
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config
RUN sed -i 's/#\?\(PermitRootLogin\s*\).*$/\1 without-password/' /etc/ssh/sshd_config
RUN sed -i 's/#\?\(PermitRootLogin\s*\).*$/\1 yes/' /etc/ssh/sshd_config

COPY services.sh ${HOME}
RUN chmod +x ${HOME}/*.sh
WORKDIR ${HOME}
EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]