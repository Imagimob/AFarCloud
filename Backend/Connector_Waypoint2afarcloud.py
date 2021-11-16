# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Forwards the waypoints to afarcloud telemetry service

from lib_mqtt_launcher import launch_mqtt_listener
from lib_packet_standardization import waypoint2afarcloud

afarcloud_rest_url03 ='https://torcos.etsist.upm.es:9203/telemetry'
afarcloud_rest_url09 ='https://torcos.etsist.upm.es:9209/telemetry'
afarcloud_rest_url11 ='https://torcos.etsist.upm.es:9211/telemetry'

def on_connect_mqtt(client, userdata, flags, rc):
    client.subscribe("afarcloud/processed/waypoints/#")

def on_message_mqtt(client, userdata, msg):
    print("Got message")

    import json
    print("de-jsonifying....")

    msg_dict=json.loads(msg.payload)
    print(msg_dict)
    
    try: 
        std_msg_dict03=waypoint2afarcloud(msg_dict,param="3")
        std_msg_dict09=waypoint2afarcloud(msg_dict,param="9")
        std_msg_dict11=waypoint2afarcloud(msg_dict,param="11")

    except Exception as error: 
        print(error.args)
        raise error

    print("Importing...")

    import requests
    headers = {'Content-type': 'application/json', 'Accept' : '*/*'}
    
    print("Trying to post to afarcloud")
    print(afarcloud_rest_url03)
    print(afarcloud_rest_url09)
    print(afarcloud_rest_url11)

    print(json.dumps(std_msg_dict03))
    print(json.dumps(std_msg_dict09))
    print(json.dumps(std_msg_dict11))

    try:
        res = requests.post(afarcloud_rest_url03, data=json.dumps(std_msg_dict03), verify=False, headers=headers)
    except Exception as error:
        print("Something fishy with the request to AS03")
        print(error.args)


    if res.ok :
        print("Posted to afarcloud "+afarcloud_rest_url03)
        print(json.dumps(std_msg_dict03))
    else:
        print("ERROR: Failed to post to Afarcloud AS03. Status code: "+ str(res.status_code)+ str(res.text))    

    try:
        res = requests.post(afarcloud_rest_url09, data=json.dumps(std_msg_dict09), verify=False, headers=headers)
    except Exception as error:
        print("Something fishy with the request to AS09")
        print(error.args)


    if res.ok :
        print("Posted to afarcloud "+afarcloud_rest_url09)
        print(json.dumps(std_msg_dict09))
    else:
        print("ERROR: Failed to post to Afarcloud AS09. Status code: "+ str(res.status_code) + str(res.text))    
    
    

    try:
        res = requests.post(afarcloud_rest_url11, data=json.dumps(std_msg_dict11), verify=False, headers=headers)
    except Exception as error:
        print("Something fishy with the request to AS11")
        print(error.args)


    if res.ok :
        print("Posted to afarcloud "+afarcloud_rest_url11)
        print(json.dumps(std_msg_dict11))
    else:
        print("ERROR: Failed to post to Afarcloud AS11. Status code: "+ str(res.status_code)+ str(res.text))    
    
    

    return

launch_mqtt_listener(on_connect_mqtt,on_message_mqtt)