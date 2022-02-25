# From MARTe2 Training Docker Image
# https://vcis-gitlab.f4e.europa.eu/aneto/MARTe2-demos-padova
#docker build -t buster_fsignal .
# https://ownyourbits.com/2019/05/13/building-docker-containers-in-2019/
# docker run -it -e DISPLAY=$DISPLAY -w /root/Projects -v $(pwd)/Projects:/root/Projects:Z -v /tmp/.X11-unix:/tmp/.X11-unix DOCKER_IMAGE_ID
# docker run -it -w /root/Projects -v $(pwd)/Projects:/root/Projects:Z
# docker exec -it xxx bash
# To restart stopped container
#  docker start  `docker ps -q -l`
#  docker attach `docker ps -q -l` # reattach the terminal & stdin

#FROM debian:buster
#FROM debian:buster-slim
FROM debian:bullseye-slim

LABEL maintainer="Bernardo Carvalho <bernardo.carvalho@tecnico.ulisboa.pt>"

#Setting path for workdir
ARG WORK_PATH=/root/Projects
#Setting workdir
WORKDIR $WORK_PATH
#Installing all the dependencies needed
RUN apt-get update
#RUN apt-get install -y build-essential git vim-nox
RUN apt-get install -y git make gcc libc-dev vim-nox g++
RUN apt-get install -y libomniorb4-dev libxerces-c-dev liblog4cxx-dev
RUN apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
#RUN adduser --disabled-password --gecos "bernardo"
#RUN su bernardo
RUN git clone https://github.com/bernardocarvalho/fsignal-node-pcie-timing

#COPY dist.tgz
#RUN tar xf dist.tgz
RUN cd fsignal-node-pcie-timing/trunk/cpp/fsignal
# RUN make

