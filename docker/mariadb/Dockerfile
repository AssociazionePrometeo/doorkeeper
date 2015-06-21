FROM mariadb

COPY create_database.sql /tmp/
COPY create_database.sh /tmp/
COPY docker-entrypoint.sh /

CMD ["mysql","--skip-name-resolve=ON"]
