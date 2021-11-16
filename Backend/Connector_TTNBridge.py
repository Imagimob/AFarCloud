# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# This program connects to the TTN MQTT, reads all the data coming in and re-publishes it on our own MQTT broker
# (i.e. it is essentially an MQTT bridge with extra options for easily filtering or modifying the incoming data on the fly)
#

from lib_sysutils import get_env_variables_if_exists
from lib_mqtt_launcher import prep_mqtt_listener

# Callback functions for the TTN side of the bridge
def on_connect_ttn(client, userdata, flags, rc):
    print("Connected to TTN with result code " + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.  <=== Interesting what this actually means... figure out later!
    client.subscribe("#")    #<== subscribe to all topics that do not start with $


def on_message_ttn(client, userdata, msg):
    # Just republish the same on the VerneMQ side (do better later)
    print("*****  FROM TTN *******")
    print(msg.topic)
    print(msg.payload)
    print("**************")
    client_mqtt.publish(msg.topic, payload=msg.payload)
    print("Published!")


# Callback functions for the MQTT side of the bridge (we do subscribe and do nothing - only publish)
def on_connect_mqtt(client, userdata, flags, rc):
    print("Connected to MQTT with result code " + str(rc))

def on_message_mqtt(client, userdata, msg):
    #print("****** TO MQTT  ********")
    #print(msg.topic)
    #print(msg.payload)
    #print("**************")
    pass



ttn_mqtt_username = get_env_variables_if_exists("TTN_MQTT_USERNAME")
ttn_mqtt_password = get_env_variables_if_exists("TTN_MQTT_PASSWORD")
ttn_server_url = get_env_variables_if_exists("TTN_MQTT_SERVER_URL")
ttn_server_port = get_env_variables_if_exists("TTN_MQTT_SERVER_PORT",type=int,default=8883)


client_mqtt = prep_mqtt_listener(on_connect_mqtt,on_message_mqtt)

client_ttn  = prep_mqtt_listener(on_connect_ttn ,on_message_ttn , mqtt_username=ttn_mqtt_username,
                                 mqtt_password=ttn_mqtt_password, mqtt_server_url=ttn_server_url,mqtt_server_port= ttn_server_port,
                                 SSL="secure",SSL_pub_key_path="./TTN_SSL_Pubkey.pem")


client_mqtt.loop_start()
client_ttn.loop_forever()

