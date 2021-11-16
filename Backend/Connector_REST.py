# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Example code for transfering data from an MQTT topic to a REST endpoint. 



from lib_mqtt_launcher import launch_mqtt_listener
import json
import requests

global REST_ENDPOINT
REST_ENDPOINT = 'https://<your url>/api/sink/'


# Callback functions
def on_connect_mqtt(client, userdata, flags, rc):
    client.subscribe("/afarcloud/processed/#")

    print("Connected to MQTT with result code " + str(rc))


def on_message_mqtt(client, userdata, msg):
    global REST_ENDPOINT

    waypoint = json.loads(msg.payload)


    res = requests.post(REST_ENDPOINT, json=json.dumps(waypoint))
    if res.ok:
        pass
    else:
        print("REST request failed!")

launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)
