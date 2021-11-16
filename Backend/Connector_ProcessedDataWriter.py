# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# This connector transforms incoming packets into individual waypoints.


from lib_mqtt_launcher import launch_mqtt_listener
import pytz
from datetime import datetime

global PROC_DATA_PUBLISH_TOPIC
PROC_DATA_PUBLISH_TOPIC = "afarcloud/processed/"


global SEQUENCE_NUMBER
SEQUENCE_NUMBER=5000


def get_devID_from_topic(topic: str):
    try:
        fields=topic.split("/")
        assert(len(fields)==5),"Unexpected topic"
        assert(fields[0]=="afarcloud"),"Unexpected topic"
        assert(fields[1]=="devices"),"Unexpected topic"
        assert(fields[3]=="up"),"Unexpected topic"
        assert(fields[4]=="decoded"),"Unexpected topic"
        return fields[2]
    except:
        return None


def get_radio_metadata(msg_dict):
    d = msg_dict["link_data"]

    best_gw = msg_dict["best_gateway"]

    d['best_gateway_id'] = best_gw["gateway_id"]
    d['rssi'] = best_gw["rssi"]
    d['snr'] = best_gw["snr"]

    return d

# Callback functions
def on_connect_mqtt(client, userdata, flags, rc):
    client.subscribe("afarcloud/decoded_uplinks/+/")
    print("Connected to MQTT with result code " + str(rc))


def on_message_mqtt(client, userdata, msg):
    import json

    msg_dict = json.loads(msg.payload.decode('utf8'))
    dev_id = msg_dict["devid"]

    waypoints = msg_dict["Waypoints"]
    radio_metadata = get_radio_metadata(msg_dict)

    global PROC_DATA_PUBLISH_TOPIC
    msg_topic=PROC_DATA_PUBLISH_TOPIC+"waypoints/"+str(dev_id)

    for w in waypoints:

        w["tracker_ID"] = dev_id

        w["radio_metadata"] = radio_metadata
        w["timestamp"] = datetime.fromtimestamp(w['timestamp']).replace(tzinfo=pytz.UTC).isoformat()

        global SEQUENCE_NUMBER
        w["seq_number"] = SEQUENCE_NUMBER
        SEQUENCE_NUMBER+=1

        send_payload = json.dumps(w)
        print("Publishing:")
        print(msg_topic)
        print(send_payload)
        client.publish(msg_topic,payload=send_payload)


launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)
