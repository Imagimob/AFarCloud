# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Helper functions for writing to the postgress database


import psycopg2
import json
from lib_sysutils import get_env_variables_if_exists
from time import sleep


def create_DB_if_not_Exists(connection,db_name):
    assert (db_name.islower()), "DB name must be lowercase"

    cursor = connection.cursor()
    cursor.execute("SELECT 1 FROM pg_catalog.pg_database WHERE datname = '{}'".format(db_name))
    exists = cursor.fetchone()
    if not exists:
        print("Creating DB {}".format(db_name))
        cursor.execute('CREATE DATABASE {}'.format(db_name))
    else:
        print("DB {} already exists. Skipping...".format(db_name))


def init_DB(override_db_name=None):
    global db_user
    global db_pass
    global db_url
    global db_name

    db_name = get_env_variables_if_exists("POSTGRES_DB")
    db_user = get_env_variables_if_exists("POSTGRES_USER")
    db_pass = get_env_variables_if_exists("POSTGRES_PASSWORD")
    db_url = get_env_variables_if_exists("POSTGRES_URL")

    # Connection sometimes fails due to server taking a few sec to get online - thus we retry several times
    max_retries = 60
    retry = 1
    while retry:
        try:
            print("Trying to connect to DB " + db_url)

            connection = psycopg2.connect(user=db_user,
                                          password=db_pass,
                                          host=db_url,
                                          port="5432")
            retry = 0
            print("Success")
        except:
            retry += 1
            if retry > max_retries:
                raise SystemError("Server " + mqtt_server_url + " not available")
            sleep(1)


    connection.autocommit = True

    create_DB_if_not_Exists(connection, db_name=db_name)

    return connect_DB()

def connect_DB():
    global db_user
    global db_pass
    global db_url
    global db_name

    connection = psycopg2.connect(user=db_user,
                                  password=db_pass,
                                  host=db_url,
                                  port="5432",
                                  database=db_name)
    connection.autocommit = True
    return connection


def create_rawdata_table(connection,table_name="raw_messsages"):
    cursor = connection.cursor()
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS {table_name} (
     ID serial NOT NULL PRIMARY KEY,
     topic text NOT NULL,
     message json NOT NULL
    );  '''.format(table_name=table_name)
                   )


def create_waypoint_table(connection,table_name="waypoints"):
    cursor = connection.cursor()
    cursor.execute('''CREATE TABLE IF NOT EXISTS {table_name}  (
                    ID serial NOT NULL PRIMARY KEY,
                    TRACKER_ID text,
                    TIMESTAMP timestamptz,
                    LATITUDE float NOT NULL,
                    LONGITUDE float NOT NULL,
                    ALTITUDE float,
                    SPEED float,
                    METADATA json
                );'''.format(table_name=table_name))
   # cursor.execute('''CREATE EXTENSION IF NOT EXISTS postgis;''')



def create_environment_table(connection,table_name="environment"):
    cursor = connection.cursor()
    cursor.execute('''CREATE TABLE IF NOT EXISTS {table_name}  (
                    ID serial NOT NULL PRIMARY KEY,
                    TRACKER_ID text,
                    TIMESTAMP timestamptz,
                    TEMPERATURE float,
                    HUMIDITY float,
                    BAR_PRESSURE float,
                    LIGHT_LEVEL float,
                    METADATA json
                );'''.format(table_name=table_name))

def insert_message_into_rawdata(d,topic, connection,table_name="raw_messsages"):
    assert (type(d) == dict)
    assert (type(topic) == str)

    cursor = connection.cursor()
    cursor.execute('''INSERT INTO {table_name} (topic,message)
        VALUES
         (
         '{topic}',
         '{values}'
         );'''.format(values=json.dumps(d),table_name=table_name,topic=topic)
                   )


def insert_waypoint(d, connection, table_name="waypoints"):
    fields = []
    values = []

    metadata_fields = list(d.keys())

    if 'tracker_ID' in d.keys():
        fields.append("TRACKER_ID")
        values.append(str(d["tracker_ID"]))
        metadata_fields.remove("tracker_ID")

    assert ('timestamp' in d.keys())
    fields.append("TIMESTAMP")
    values.append(str(d["timestamp"]))
    metadata_fields.remove("timestamp")

    assert ('latitude' in d.keys())
    fields.append("LATITUDE")
    values.append(float(d["latitude"]))
    metadata_fields.remove("latitude")

    assert ('longitude' in d.keys())
    fields.append("LONGITUDE")
    values.append(float(d["longitude"]))
    metadata_fields.remove("longitude")

    if 'altitude' in d.keys():
        fields.append("ALTITUDE")
        values.append(float(d["altitude"]))
        metadata_fields.remove("altitude")

    if 'speed' in d.keys():
        fields.append("SPEED")
        values.append(float(d["speed"]))
        metadata_fields.remove("speed")

    fields.append("METADATA")
    values.append(json.dumps({k: d[k] for k in metadata_fields}))

    cursor = connection.cursor()
    print("?")
    cursor.execute(
        '''INSERT INTO {table_name} {fields} VALUES {values};'''.format(fields=str(tuple(fields)).replace("'", ""),
                                                                        values=tuple(values), table_name=table_name))

def insert_environment_point(d, connection, table_name="waypoints"):
    fields = []
    values = []

    metadata_fields = list(d.keys())

    assert ('timestamp' in d.keys())
    fields.append("TIMESTAMP")
    values.append(str(d["timestamp"]))
    metadata_fields.remove("timestamp")

    if 'tracker_ID' in d.keys():
        fields.append("TRACKER_ID")
        values.append(str(d["tracker_ID"]))
        metadata_fields.remove("tracker_ID")

    if ('temperature' in d.keys()):
        fields.append("TEMPERATURE")
        values.append(str(d["temperature"]))
        metadata_fields.remove("temperature")

    if ('light' in d.keys()):
        fields.append("LIGHT_LEVEL")
        values.append(str(d["light"]))
        metadata_fields.remove("light")


    #TBA: humidity, etc...

    fields.append("METADATA")
    values.append(json.dumps({k: d[k] for k in metadata_fields}))

    cursor = connection.cursor()
    cursor.execute(
        '''INSERT INTO {table_name} {fields} VALUES {values};'''.format(fields=str(tuple(fields)).replace("'", ""),
                                                                        values=tuple(values), table_name=table_name))
