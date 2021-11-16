# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Sample of skelleton code for standardized "Connector"


from lib_mqtt_launcher import launch_mqtt_listener


# Callback functions
def on_connect_mqtt(client, userdata, flags, rc):
    pass


def on_message_mqtt(client, userdata, msg):
    pass



launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)
