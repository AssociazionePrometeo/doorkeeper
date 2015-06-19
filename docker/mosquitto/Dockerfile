FROM ansi/mosquitto

COPY auth-plug.so /usr/lib64/auth-plug.so
COPY mosquitto.conf /etc/mosquitto/mosquitto.conf
RUN apt-get update && apt-get install -y libmysqlclient-dev
CMD /usr/local/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf -v 
EXPOSE 8883
