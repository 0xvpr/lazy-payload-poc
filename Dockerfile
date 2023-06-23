# Created by:       VPR
# Created:          June 22nd, 2023

# Updated by:       VPR
# Updated:          June 22nd, 2023

FROM ubuntu:22.04

# Set env to avoid user input interruption during installation
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install normal goodies
RUN apt-get update && apt upgrade -y
RUN apt-get install -y --no-install-recommends \
    zsh \
    zsh-autosuggestions \
    vim \
    make \
    build-essential \
    mingw-w64-common \
    mingw-w64-tools \
    mingw-w64-x86-64-dev \
    mingw-w64

# Change login shell to zsh
RUN chsh -s /bin/zsh $(whoami)

# Add non-root user & create working environment
RUN groupadd -g 1000 dev
RUN useradd -u 1000 -g dev lazy-payload-dev
USER lazy-payload-dev
ENV HOME=/home/lazy-payload-dev
WORKDIR /var/lazy-payload-dev/lazy-payload
