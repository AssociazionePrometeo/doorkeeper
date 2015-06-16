FROM mariadb

COPY create_database.sql /tmp/
COPY create_database.sh /tmp/
COPY docker-entrypoint.sh /

RUN  ["chmod","755","/tmp/create_database.sh"]

