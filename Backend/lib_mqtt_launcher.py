# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Helper function for starting the services

from lib_sysutils import get_env_variables_if_exists
from time import sleep


def prep_mqtt_listener(on_connect,on_message,on_log=None,mqtt_username=None,mqtt_password=None,mqtt_server_url=None,mqtt_server_port=None,SSL='off',SSL_pub_key_path=None):

    import paho.mqtt.client as mqtt

    if mqtt_username is None:
        mqtt_username = get_env_variables_if_exists("MQTT_USERNAME")

    if mqtt_password is None:
        mqtt_password = get_env_variables_if_exists("MQTT_PASSWORD")

    if mqtt_server_url is None:
        mqtt_server_url = get_env_variables_if_exists("MQTT_SERVER_URL")

    if mqtt_server_port is None:
        mqtt_server_port = get_env_variables_if_exists("MQTT_SERVER_PORT",type=int)


    client_mqtt = mqtt.Client()

    client_mqtt.username_pw_set(username=mqtt_username,password=mqtt_password)

    client_mqtt.on_connect = on_connect
    client_mqtt.on_message = on_message

    if on_log is not None:
        client_mqtt.on_log = on_log


    if SSL=='off':
        pass
    elif SSL=='insecure':
        client_mqtt.tls_set()
        client_mqtt.tls_insecure_set(True)
    elif SSL=="secure":
        if SSL_pub_key_path is not None:
            client_mqtt.tls_set(SSL_pub_key_path)
        else:
            #Here, we need to hope that the SSL pubkey is in the system root certificates - Not recommended.
            client_mqtt.tls_set()
        client_mqtt.tls_insecure_set(True)
    else:
        raise ValueError("SSL parameter must be off, secure or insecure")


    # Connection sometimes fails due to server taking a few sec to get online - thus we retry several times
    max_retries = 60

    retry = 1
    while retry:
        try:
            print("Trying to connect to server " + mqtt_server_url)

            client_mqtt.connect(mqtt_server_url, mqtt_server_port, 60)
            retry=0
            print("Success")
        except:
            retry+=1
            if retry > max_retries:
               raise SystemError("Server "+mqtt_server_url+" not available")
            sleep(1)

    return client_mqtt


def launch_mqtt_listener(on_connect,on_message,mqtt_username=None,mqtt_password=None,mqtt_server_url=None,mqtt_server_port=None,SSL='off',SSL_pub_key_path=None):
    client_mqtt = prep_mqtt_listener(on_connect, on_message, mqtt_username=mqtt_username, mqtt_password=mqtt_password, mqtt_server_url=mqtt_server_url,
                       mqtt_server_port=mqtt_server_port,SSL=SSL,SSL_pub_key_path=SSL_pub_key_path)
    client_mqtt.loop_forever()
