# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Code for creating an endpoint that can receive packets from the DIGITA LoRa network in Finland. 
# Used during year 1 demonstrator in Finland only. Not maintained after Year 1


from flask import (
    Flask,
    render_template
)

from flask import request, Response
import tailer
from lib_sysutils import get_env_variables_if_exists
from lib_mqtt_launcher import prep_mqtt_listener

print("Hello World")

def on_connect_mqtt(client, userdata, flags, rc):
    print("Connected to MQTT with result code " + str(rc))

def on_message_mqtt(client, userdata, msg):
    pass

client_mqtt = prep_mqtt_listener(on_connect_mqtt,on_message_mqtt)
client_mqtt.loop_start()

# Create the application instance
app = Flask(__name__, template_folder="templates")


# Create a URL route in our application for "/"
@app.route('/')
def home():
    """
    This function just responds to the browser ULR
    localhost:5000/

    :return:        the last 20 queries '
    """
    last_lines = tailer.tail(open('/data/queries.txt'), 20)
    last_lines = ''.join([x + "\n" for x in last_lines])

    return Response(last_lines, mimetype='text/plain')

@app.route('/digita', methods=['GET', 'POST'])
def digita():
    import datetime, json
    query = dict()

    content = request.get_json(silent=True)
    query["Timestamp"] = str(datetime.datetime.now())
    query["Type"] = request.method
    query["IP"] = request.remote_addr
    query["Payload"] = content

    print(query)  # Do your processing
    with open("/data/queries.txt", "a") as myfile:
        myfile.write("Digita::::" + str(query) + "\n")

    topic="afarcloud/digita/uplinks/"
    client_mqtt.publish(topic,json.dumps(content))

    return "ok"


if __name__ == '__main__':
    app.debug = True
    app.run(host='0.0.0.0', port=5000)

