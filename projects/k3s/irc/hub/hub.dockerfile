# gucefIrcHub: the staged build of build-image.sh on the Ubuntu release it was built on
ARG UBUNTU_VERSION=22.04
FROM ubuntu:${UBUNTU_VERSION}
# openssl brings the libssl the TLS provider of GUCEF loads; ca-certificates the authorities for the vendor APIs;
# gucefCORE links libX11 (its desktop console windows), which is not used here
RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates openssl libx11-6 \
    && rm -rf /var/lib/apt/lists/*
COPY stage/ /opt/gucef-irc-hub/
ENV GUCEF_CONSOLE_LOGGER=stderr
USER 65534:65534
ENTRYPOINT ["/opt/gucef-irc-hub/bin/gucefIrcHub"]
