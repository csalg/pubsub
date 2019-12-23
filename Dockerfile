FROM ubuntu:18.04

ENV TZ=America/New_York
ENV DEBIAN_FRONTEND=noninteractive

RUN mkdir /app
COPY ./src /app
COPY ./sources.list /etc/apt/

# Install Python

RUN apt update --assume-yes
RUN apt install python3 python3-pip --assume-yes

RUN pip3 install -i https://pypi.mirrors.ustc.edu.cn/simple/  jinja2

# Install R

RUN apt install r-base --assume-yes

RUN R -e "install.packages('ggplot2', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('data.table', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('dplyr', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('condMVNorm', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('VGAM', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('configr', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('RJSONIO', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN apt install libcurl4-openssl-dev libssl-dev --assume-yes
RUN R -e "install.packages('optparse', dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
