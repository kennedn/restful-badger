FROM debian:stable-slim

RUN apt-get update && \
    apt-get install -y chrony libfaketime faketime && \
    rm -rf /var/lib/apt/lists/*

COPY chrony.conf /etc/chrony/chrony.conf

EXPOSE 123/udp

ENV FAKETIME="@2025-10-01 12:34:56"

CMD ["bash", "-c", "FAKETIME=\"$FAKETIME\" faketime -f \"$FAKETIME\" chronyd -d -x"]

