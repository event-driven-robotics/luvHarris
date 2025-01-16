FROM ubuntu22.04
ENV DEBIAN_FRONTEND noninteractive

ARG CODE_DIR=/usr/local/src

RUN apt update

#basic environment
RUN apt install -y \
    ca-certificates \
    build-essential \
    git \
    cmake \
    cmake-curses-gui \
    libace-dev \
    libassimp-dev \
    libglew-dev \
    libglfw3-dev \
    libglm-dev \
    libeigen3-dev \
    vim

RUN apt install -y \
    libcanberra-gtk-module \
    ffmpeg \
    libopencv-dev

# YCM
ARG YCM_VERSION=v0.15.2
RUN cd $CODE_DIR && \
    git clone --depth 1 --branch $YCM_VERSION https://github.com/robotology/ycm.git && \
    cd ycm && \
    mkdir build && cd build && \
    cmake .. && \
    make -j `nproc` install

# YARP
ARG YARP_VERSION=v3.8.0
RUN cd $CODE_DIR && \
    git clone --depth 1 --branch $YARP_VERSION https://github.com/robotology/yarp.git &&\
    cd yarp &&\
    mkdir build && cd build &&\
    cmake .. &&\
    make -j `nproc` install

# event-driven
ARG ED_VERSION=main
RUN cd $CODE_DIR &&\
    git clone --depth 1 --branch $ED_VERSION https://github.com/robotology/event-driven.git &&\
    cd event-driven &&\
    mkdir build && cd build &&\
    cmake .. &&\
    make -j `nproc` install

# Build luvHarris
ARG LUVHARRIS_VERSION=main
RUN cd $CODE_DIR &&\
    git clone --branch $LUVHARRIS_VERSION https://github.com/event-driven-robotics/luvHarris.git &&\
    cd luvHarris &&\
    mkdir build && cd build &&\
    cmake .. &&\
    make -j `nproc` install