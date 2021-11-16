# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Records the decoded waypoints to a postgres database

from lib_mqtt_launcher import launch_mqtt_listener
from lib_postgis import init_DB, connect_DB, create_waypoint_table, insert_waypoint
import json

global WAYPOINT_TABLE_NAME
WAYPOINT_TABLE_NAME ="waypoints"

# Callback functions
def on_connect_mqtt(client, userdata, flags, rc):
    client.subscribe("afarcloud/processed/waypoints/#")

    print("Connected to MQTT with result code " + str(rc))


def on_message_mqtt(client, userdata, msg):
    global WAYPOINT_TABLE_NAME

    waypoint = json.loads(msg.payload)

    print(waypoint)
    #Remap meaningful names
    if waypoint["tracker_ID"] in ["ttgo5"]: 
        print("In it....")
        remap = {"1" : "Cow_resting", "2" : "Cow_walking"  , "3" : "Cow_grazing", "4" : "Cow_other"}
        waypoint["states"] = { remap[k] : v for (k,v) in  waypoint["states"].items() }

    print(waypoint)


    connection = connect_DB()
    insert_waypoint(waypoint,connection,table_name=WAYPOINT_TABLE_NAME)
    connection.close()
    print("Inserted waypoint in DB")

connection = init_DB()
create_waypoint_table(connection,table_name=WAYPOINT_TABLE_NAME)
connection.close()

launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)


