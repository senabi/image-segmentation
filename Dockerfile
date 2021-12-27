FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive
ENV HOME /root
RUN echo 'US/Eastern' > /etc/timezone

RUN apt-get update \
    && apt-get install -y \
    g++ gfortran binutils make cmake ninja-build pkg-config unzip \
    ca-certificates wget iproute2 iputils-ping net-tools \
    ssh sshpass \
    libopenmpi-dev openmpi-bin libomp-dev libopencv-dev \
    libjpeg-dev libpng-dev libtiff-dev ffmpeg \
    vim nodejs npm \
    --no-install-recommends \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get autoclean

RUN wget --no-check-certificate https://www.vpn.net/installers/logmein-hamachi_2.1.0.203-1_amd64.deb -O /tmp/hamachi.deb \
    && dpkg -i /tmp/hamachi.deb

ARG USER_PASS
RUN mkdir /var/run/sshd \
    && echo "root:${USER_PASS}" | chpasswd \
    && echo "export TERM=vt100" >> ${HOME}/.bashrc

RUN echo "    CheckHostIP no\n    StrictHostKeyChecking no" >> /etc/ssh/ssh_config \
    && sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config \
    && sed -i 's/#\?\(PermitRootLogin\s*\).*$/\1 without-password/' /etc/ssh/sshd_config \
    && sed -i 's/#\?\(PermitRootLogin\s*\).*$/\1 yes/' /etc/ssh/sshd_config

ENV CC gcc
ENV CXX g++

COPY init.sh $HOME/init.sh
COPY init-vpn.sh $HOME/init-vpn.sh
COPY add-node.sh $HOME/add-node.sh

COPY server/index.js ${HOME}/server/index.js
COPY server/public ${HOME}/server/public
COPY server/package-lock.json ${HOME}/server/package-lock.json
COPY server/package.json ${HOME}/server/package.json

RUN cd ${HOME}/server \
    && npm i

COPY mpi/src $HOME/mpi/src
COPY mpi/CMakeLists.txt $HOME/mpi/CMakeLists.txt

RUN mkdir -p $HOME/mpi/build \
    && cd $HOME/mpi/build \
    && cmake \
    -D CMAKE_C_COMPILER=$CC \
    -D CMAKE_CXX_COMPILER=$CXX \
    -G Ninja .. \
    && ninja \
    && cp $HOME/mpi/build/mpi-img-seg /usr/local/bin/mpi-img-seg \
    && chmod +x $HOME/*.sh

WORKDIR $HOME
EXPOSE 22