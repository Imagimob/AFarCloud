#
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#


# This connect just prints everything that goes through the MQTT broker on the console - useful for debugging

from lib_mqtt_launcher import prep_mqtt_listener
from datetime import datetime


# Callback functions for the MQTT side of the bridge (we do subscribe and do nothing - only publish)
def on_connect_mqtt(client, userdata, flags, rc):
    print("Connected to MQTT with result code " + str(rc))
    client.subscribe("#")


def on_message_mqtt(client, userdata, msg):
    print("*********************** "+str(datetime.now()) +
          " ****************************************")
    print(str(msg.topic))
    print(" ")
    print(str(msg.payload))
    print("*********************************************************************************************************")


client_mqtt = prep_mqtt_listener(on_connect_mqtt, on_message_mqtt)

client_mqtt.loop_forever()
