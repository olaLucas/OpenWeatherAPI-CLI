FROM archlinux:base-devel

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm cjson

WORKDIR /app

RUN mkdir src
RUN mkdir libs
RUN mkdir bin

COPY src /app/src
COPY libs /app/libs
COPY Makefile /app

RUN make

CMD ["/bin/bash"]
