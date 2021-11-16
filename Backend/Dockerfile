FROM python:3-alpine

RUN \
 apk add --no-cache postgresql-libs && \
 apk add --no-cache --virtual .build-deps gcc musl-dev postgresql-dev && \
 pip install psycopg2 --no-cache-dir && \
 apk --purge del .build-deps

RUN pip install paho_mqtt influxdb flask tailer --no-cache-dir

WORKDIR /afarcloud

#Add the code
COPY lib_*.py Connector_*.py  ./

#Add the public key for TLS with TTN
COPY *.pem ./


ENTRYPOINT ["python","-u"]
