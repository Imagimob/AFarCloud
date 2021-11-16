# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Connector that saves the raw-est incoming data into the DB



from lib_mqtt_launcher import launch_mqtt_listener
from lib_postgis import init_DB, connect_DB, create_rawdata_table, insert_message_into_rawdata
import json

global RAWDATA_TABLE_NAME
RAWDATA_TABLE_NAME = "raw_messsages"

# Callback functions
def on_connect_mqtt(client, userdata, flags, rc):
    client.subscribe("afarcloud/devices/+/up")
    client.subscribe("afarcloud/lorawan_server/uplinks/")


    print("Connected to MQTT with result code " + str(rc))

def on_message_mqtt(client, userdata, msg):
    global RAWDATA_TABLE_NAME

    msg_dict = json.loads(msg.payload.decode('utf8'))

    connection = connect_DB()
    insert_message_into_rawdata(msg_dict,msg.topic,connection,table_name=RAWDATA_TABLE_NAME)
    connection.close()

connection = init_DB()
create_rawdata_table(connection,table_name=RAWDATA_TABLE_NAME)
connection.close()

launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)

