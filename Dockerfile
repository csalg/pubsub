FROM ubuntu:18.04

RUN mkdir /app
COPY ./src /app
COPY ./sources.list /etc/apt/

# Install Python

RUN apt update
RUN apt install python3

RUN pip3 install json
RUN pip3 install jinja2

# Install R

RUN apt install r-base

RUN R -e "install.packages('ggplot2'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('data.table'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('dplyr'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('condMVNorm'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('VGAM'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('configr'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('RJSONIO'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"
RUN R -e "install.packages('optparse'), dependencies=TRUE, repos='https://mirrors.ustc.edu.cn/CRAN/')"

