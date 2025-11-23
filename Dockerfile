FROM fedora:43

WORKDIR /opt/tempest

COPY build/apps/db/tempest-db .

RUN dnf install -y libpqxx

RUN useradd tempest
USER tempest

CMD ["/opt/tempest/tempest-db"]