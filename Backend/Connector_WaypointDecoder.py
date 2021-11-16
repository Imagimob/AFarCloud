# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Decodes the message from TTN, DIGITA or a lorawanserver instance into a standard format,
# and interprets the bytes from the sensor into more high level data structure for further processing

from lib_mqtt_launcher import launch_mqtt_listener
from lib_packet_standardization import standardize_packet
import base64, json
from lib_waypoints import decode_raw_payload, decode_raw_payload2, parse_standardized_message, states_dictionary

global SEQUENCE_NUMBER
SEQUENCE_NUMBER=5000
print("Hello")
print("Sequence number : " + str(SEQUENCE_NUMBER))


# Callback functions
def on_connect_mqtt(client, userdata, flags, rc):
    client.subscribe("afarcloud/devices/+/up")
    client.subscribe("afarcloud/lorawan_server/uplinks/")
    client.subscribe("afarcloud/digita/uplinks/")
    client.subscribe("v3/afarcloud@ttn/devices/+/up")

    print("Connected to MQTT with result code " + str(rc))


def on_message_mqtt(client, userdata, msg):

    print("Received :")
    print(msg.topic)
    print(msg.payload)
    msg_dict = json.loads(msg.payload.decode('utf8'))

    print("Will be standardizing...")
    msg_dict = standardize_packet(msg_dict)
    print("Standardized")
    print(msg_dict)

    if ((msg_dict["port"]==1) or (msg_dict["port"]==2)) :

        payload = bytes.fromhex(msg_dict["hex_payload"])

        if (msg_dict["port"]==1):
            waypoints = decode_raw_payload(payload)
        elif (msg_dict["port"] == 2):
           waypoints = decode_raw_payload2(payload)


    elif (msg_dict["port"] in states_dictionary.keys()):
        waypoints = parse_standardized_message(msg_dict)

    else:
        print("Message received on unknown port - ignoring.")
        return


    print("Decoded")
    print(waypoints)
    msg_dict["Waypoints"] = [ x.to_dict() for x in waypoints]

    global SEQUENCE_NUMBER
    msg_dict["Seq_number"] = SEQUENCE_NUMBER
    SEQUENCE_NUMBER+=1
    print("Sequence number : " + str(SEQUENCE_NUMBER))


    send_payload = json.dumps(msg_dict)

    msg_topic = "afarcloud/decoded_uplinks/{deveui}/".format(deveui=msg_dict["devid"])

    print("Publishing: ")
    print(msg_topic)
    print(send_payload)
    client.publish(msg_topic,payload=send_payload)



launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)
