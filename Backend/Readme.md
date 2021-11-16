 
# TTN to VerneMQ Bridge 

This program connects our pipeline to The Things Network by transfering everything that is published on TTN data API into our VerneMQ MQTT broker.

The code is configured through environmental variables to make it easier to integrate in the docker-compose framework.
You'll need to define the following ones (the names are self-explanatory): 

```
MQTT_USERNAME
MQTT_PASSWORD
MQTT_SERVER_URL
MQTT_SERVER_PORT
TTN_APP_ID
TTN_APP_KEY
TTN_SERVER_URL
 ```
 
Currentlyj, it'll connect over SSL to TTN, but unencrypted to VerneMQ (it assumes this runs on the same server - secure - as VerneMQ)
 

TBA: 
- Filter and transform the input before re-publishing
- Allow for re-publishing downlinks from VernMQ to TTN 
- ADD SSL on the VerneMQ side
- Play with QOS and 'unclean sessions' 
