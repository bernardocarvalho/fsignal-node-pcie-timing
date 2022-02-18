# From MARTe2 Training Docker Image
# https://vcis-gitlab.f4e.europa.eu/aneto/MARTe2-demos-padova
#docker build -t buster_fsignal .
# https://ownyourbits.com/2019/05/13/building-docker-containers-in-2019/
# docker run -it -e DISPLAY=$DISPLAY -w /root/Projects -v ~/Projects:/root/Projects:Z -v /tmp/.X11-unix:/tmp/.X11-unix DOCKER_IMAGE_ID
# docker exec -it xxx bash

FROM debian:buster

LABEL maintainer="Bernardo Brotas <bernardo.brotas@gmail.com>"

#Setting path for workdir
ARG WORK_PATH=~/Projects
#Setting workdir
WORKDIR $WORK_PATH
#Installing all the dependencies needed
RUN apt-get update
#RUN apt-get install -y build-essential git vim-nox
RUN apt-get install -y git make gcc libc-dev vim-nox
#RUN apt-get clean \
#    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
#RUN adduser --disabled-password --gecos "bernardo"
#RUN su bernardo
RUN git clone https://github.com/bernardocarvalho/fsignal-node-pcie-timing

