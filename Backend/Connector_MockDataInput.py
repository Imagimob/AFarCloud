# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# This connector just sends a pre-defined mqtt message into the pipeline every second to test the pipeline for testing - nothing less, nothing more!

from lib_mqtt_launcher import prep_mqtt_listener
from time import sleep
from lib_dummyencoder import genDummyMessage_ttn


# Callback functions for the MQTT side of the bridge (we do subscribe and do nothing - only publish)
def on_connect_mqtt(client, userdata, flags, rc):
    print("Connected to MQTT with result code " + str(rc))

def on_message_mqtt(client, userdata, msg):
    pass


client_mqtt = prep_mqtt_listener(on_connect_mqtt, on_message_mqtt)

client_mqtt.loop_start()

while True:
    sleep(15)

    (topic_ttn,msg_ttn) = genDummyMessage_ttn()


    # Send ttn message
    client_mqtt.publish(topic_ttn, payload=msg_ttn)
    # Send lorawan_server message
    #client_mqtt.publish(topic_lorawan, payload=msg_lorawan)
