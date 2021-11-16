# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Connector for pushing code to InfluxDB. This was superseeded by a simpler Postgres database. 


from mqtt_launcher import  launch_mqtt_listener
from influxdb import InfluxDBClient
from lib_sysutils import get_env_variables_if_exists

# Callback functions
def on_connect_mqtt(client, userdata, flags, rc):
    client.subscribe("afarcloud/devices/+/up/decoded")

    print("Connected to MQTT with result code " + str(rc))


def on_message_mqtt(client, userdata, msg):
    import json
    from lib_waypoints import  Waypoint
    #print("********************")
    #print(msg.topic)
    #print(msg.payload.decode('utf8'))
    #print("********************")

    msg.payload.decode('utf8')

    msg_dict = json.loads(msg.payload.decode('utf8'))

    waypoints = msg_dict["Waypoints"]

    waypoints = list( map(lambda x: x.to_influxDB(), map(Waypoint.from_dict, waypoints)))



    global influxDB
    influxDB.write_points(waypoints)
    t = influxDB.query('''SELECT "latitude" FROM "raw_data"."Waypoint" WHERE time > now()-10s''')
    print(waypoints)
    print(t)


global influxDB

influxdb_url = get_env_variables_if_exists("INFLUXDB_URL")
influxdb_username = get_env_variables_if_exists("INFLUXDB_USERNAME")
influxdb_password = get_env_variables_if_exists("INFLUXDB_PASSWORD")
influxdb_port = get_env_variables_if_exists("INFLUXDB_PORT")


influxDB = InfluxDBClient(host=influxdb_url,port=influxdb_port,username=influxdb_username,password=influxdb_password)


#This should be understood as "create if not exists"
influxDB.query(''' CREATE DATABASE "afarcloud"''')

influxDB.query(''' CREATE RETENTION POLICY "raw_data" ON "afarcloud" DURATION INF REPLICATION 5 DEFAULT''')
influxDB.switch_database("afarcloud")


launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)


