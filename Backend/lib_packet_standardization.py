# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Implements all the logic for parsing packets from TTN (v2 and v3), DIGITA and lorawanserver LoRa networks
# and decoding the message payloads into comprehensible data


import base64
from dateutil.parser import parse
import pytz

hardware2devid = {
    "0004A30B001ECF9C" : "ttgo1",
    "0004A30B001ECF9D" : "ttgo2",
    "0004A30B001ECF9E" : "ttgo3",
    "0004A30B001ECF9F" : "ttgo4",
    "0004A30B001ECF10" : "ttgo5",
}




def standardize_digita_packet(msg_dict):
    payload = msg_dict['DevEUI_uplink']

    common_message = dict()
    common_message["appid"] = "217C9F782EE4F162"

    common_message["devid"] = hardware2devid[payload["DevEUI"]]
    common_message["port"] = payload["FPort"]
    common_message["timestamp"] = payload["Time"]
    common_message["hex_payload"] = payload['payload_hex']

    link_data = dict()
    link_data["frequency"] = 0
    link_data["data_rate"] = 0
    link_data["coding_rate"] = 0
    link_data["airtime"] = 0
    link_data["frame_counter"] = 0

    best_gw = dict()
    best_gw["gateway_id"] = 'digita'
    best_gw["snr"] = payload['LrrSNR']
    best_gw["rssi"] = payload['LrrRSSI']
    # best_gw["arrival_time"] = msg_dict["best_gw"]["time"]

    common_message["link_data"] = link_data
    common_message["best_gateway"] = best_gw

    return common_message

def standardize_lorawanserver_packet(msg_dict):
    common_message = dict()
    common_message["appid"] = msg_dict["appid"]
    common_message["devid"] = hardware2devid[msg_dict["deveui"]]
    common_message["port"] = msg_dict["port"]
    common_message["timestamp"] = parse(msg_dict["datetime"]).isoformat()
    common_message["hex_payload"] = msg_dict["data"]

    link_data = dict()
    link_data["frequency"] = msg_dict["freq"]
    link_data["data_rate"] = msg_dict["datr"]
    link_data["coding_rate"] = msg_dict["codr"]
    link_data["airtime"] = -1
    link_data["frame_counter"] = msg_dict["fcnt"]

    best_gw = dict()
    best_gw["gateway_id"] = msg_dict["best_gw"]["mac"]
    best_gw["snr"] = msg_dict["best_gw"]["lsnr"]
    best_gw["rssi"] = msg_dict["best_gw"]["rssi"]
    # best_gw["arrival_time"] = msg_dict["best_gw"]["time"]

    common_message["link_data"] = link_data
    common_message["best_gateway"] = best_gw

    return common_message


def standardize_ttn_packet(msg_dict):

    common_message = dict()
    common_message["appid"] = msg_dict["app_id"]
    common_message["devid"] = msg_dict["dev_id"]
    common_message["port"] = msg_dict["port"]
    common_message["timestamp"] = parse(msg_dict["metadata"]["time"]).isoformat()
    common_message["hex_payload"] = base64.b64decode(msg_dict["payload_raw"]).hex()

    link_data = dict()
    link_data["frequency"] = msg_dict["metadata"]["frequency"]
    link_data["data_rate"] = msg_dict["metadata"]["data_rate"]
    link_data["coding_rate"] = msg_dict["metadata"]["coding_rate"]
    link_data["airtime"] = msg_dict["metadata"]["airtime"]
    link_data["frame_counter"] = msg_dict["counter"]

    def get_best_gateway(msg_dict,location=False):
        gateways = msg_dict["metadata"]["gateways"]
        
        if location:
            gateways = [x for x in gateways if "latitude" in x.keys()]
        
        RSSIs = [x["rssi"] for x in gateways]
        
        if len(RSSIs)>0:
            idx_best = RSSIs.index(max(RSSIs))
            return gateways[idx_best]
        return None

    best_ttn_gw = get_best_gateway(msg_dict)

    best_gw = dict()
    best_gw["gateway_id"] = best_ttn_gw["gtw_id"]
    best_gw["snr"] = best_ttn_gw["snr"]
    best_gw["rssi"] = best_ttn_gw["rssi"]



    common_message["link_data"] = link_data
    common_message["best_gateway"] = best_gw

    location_gw = get_best_gateway(msg_dict,location=True)
    if location_gw is not None:
        loc_gw = dict()
        loc_gw["gateway_id"] = location_gw["gtw_id"]
        loc_gw["snr"] = location_gw["snr"]
        loc_gw["rssi"] = location_gw["rssi"]
        loc_gw["latitude"] = location_gw["latitude"]
        loc_gw["longitude"] = location_gw["longitude"]
        loc_gw["altitude"] = location_gw["altitude"]
        common_message["localized_gateway"] = loc_gw
        

    return common_message



def standardize_ttnv3_packet(msg_dict):

    common_message = dict()
    common_message["appid"] = msg_dict["end_device_ids"]["application_ids"]["application_id"]
    common_message["devid"] = msg_dict["end_device_ids"]["device_id"]
    common_message["port"] = msg_dict["uplink_message"]["f_port"]
    common_message["timestamp"] = parse(msg_dict["uplink_message"]["received_at"]).isoformat()
    common_message["hex_payload"] = base64.b64decode(msg_dict["uplink_message"]["frm_payload"]).hex()

    link_data = dict()
    link_data["frequency"] = msg_dict["uplink_message"]["settings"]["frequency"]
    link_data["data_rate"] = msg_dict["uplink_message"]["settings"].get("data_rate_index","")
    link_data["coding_rate"] = msg_dict["uplink_message"]["settings"].get("coding_rate","")
    link_data["airtime"] = msg_dict["uplink_message"].get("consumed_airtime","")
    link_data["frame_counter"] = msg_dict["uplink_message"].get("f_cnt","")

    def get_best_gateway(msg_dict,location=False):
        gateways = msg_dict["uplink_message"]["rx_metadata"]
        
        if location:
            gateways = [x for x in gateways if "latitude" in x.keys()]
        
        RSSIs = [x["rssi"] for x in gateways]
        
        if len(RSSIs)>0:
            idx_best = RSSIs.index(max(RSSIs))
            return gateways[idx_best]
        return None

    best_ttn_gw = get_best_gateway(msg_dict)

    best_gw = dict()
    best_gw["gateway_id"] = best_ttn_gw["gateway_ids"]["gateway_id"]
    best_gw["snr"] = best_ttn_gw["snr"]
    best_gw["rssi"] = best_ttn_gw["rssi"]



    common_message["link_data"] = link_data
    common_message["best_gateway"] = best_gw

    #location_gw = get_best_gateway(msg_dict,location=True)
    #if location_gw is not None:
    #    loc_gw = dict()
    #    loc_gw["gateway_id"] = location_gw["gateway_ids"]["gateway_id"]
    #    loc_gw["snr"] = location_gw["snr"]
    #    loc_gw["rssi"] = location_gw["rssi"]
    #    loc_gw["latitude"] = location_gw["latitude"]
    #    loc_gw["longitude"] = location_gw["longitude"]
    #    loc_gw["altitude"] = location_gw["altitude"]
    #    common_message["localized_gateway"] = loc_gw
        

    return common_message



def identify_packet(msg_dict):
    keys_ttn = {'app_id',
                'counter',
                'dev_id',
                'hardware_serial',
                'metadata',
                'payload_raw',
                'port'}

    keys_lorawanserver = {'all_gw',
                          'app',
                          'appid',
                          'battery',
                          'best_gw',
                          'codr',
                          'data',
                          'datetime',
                          'datr',
                          'devaddr',
                          'deveui',
                          'fcnt',
                          'freq',
                          'lsnr',
                          'mac',
                          'netid',
                          'port',
                          'rssi'}

    keys_digita = {'DevEUI_uplink'}

    keys_ttnv3 = {
        "end_device_ids",
        "correlation_ids",
        "received_at",
        "uplink_message",
    }

    if set(msg_dict.keys()) >= keys_ttn:
        return "ttn"
    elif set(msg_dict.keys()) >= keys_lorawanserver:
        return "lorawanserver"
    elif set(msg_dict.keys()) >= keys_digita:
        return "digita"
    elif set(msg_dict.keys()) >= keys_ttnv3:
        return "ttnv3"
    return "unknown"

def standardize_packet(msg_dict):
    source = identify_packet(msg_dict)

    if source == "ttn":
        return standardize_ttn_packet(msg_dict)
    elif source == "lorawanserver":
        return standardize_lorawanserver_packet(msg_dict)
    elif source == "digita":
        return standardize_digita_packet(msg_dict)
    elif source == "ttnv3":
        return standardize_ttnv3_packet(msg_dict)
    else:
        print("Error: packet source not identified")
        return dict



def waypoint2afarcloud(msg,param="3"):
    from dateutil.parser import isoparse
    param = str(param)
    assert(param in ["3","9","11"]) , "Wrong param"

    # afarcloud/scenario/service/type_of_device/deviceID/measureList
    padded_param = param if len(param)==2 else ("0"+param)

    scenario = "AS"+str(padded_param)
    provider = "IMAG"

    name_mapping = {"ttgo1" : ("cows1","livestockManagement","algorithm"),
                    "ttgo2" : ("tractor2","cropsManagement","tractor"),
                    "ttgo3" : ("cows1","livestockManagement","algorithm"),
                    "ttgo4" : ("tractor1","cropsManagement","tractor"),
                    "ttgo5" : ("cows1","livestockManagement","algorithm"),
                    "ttgo6" : ("tractor3","cropsManagement","tractor"),            
                    }

    deviceID,service,type_of_device = name_mapping[msg["tracker_ID"]]


    ressourceID = "urn:afc:" + scenario +":" +service +":" + provider + ":" + type_of_device + ":" + deviceID +"_"+param

    unix_timestamp = int(isoparse(msg["timestamp"]).timestamp())
    
    observations = []
    
    print("Hello")

    if type_of_device == "tractor":
        if "speed" in msg.keys() and msg["speed"] is not None:
            obs = {
                            "observedProperty": "vehicle_speed",
                            "resultTime": unix_timestamp,
                            "result": {
                                "value": msg["speed"],
                                "uom": "http://qudt.org/vocab/unit/KiloM-PER-HR"
                        }
            }
        
            observations.append(obs)
        state_prefix = "tractor_"
    else:
        state_prefix = "cow_"
        
    print("Hi!")
   
    if "battery" in msg.keys() and msg["battery"] is not None:
        obs = {
                          "observedProperty": "battery",
                          "resultTime": unix_timestamp,
                          "result": {
                            "value": msg["battery"],
                            "uom": "http://qudt.org/vocab/unit/V"
                     }
                    }
        
        observations.append(obs)
    
    if "radio_metadata" in msg.keys() and "rssi" in msg["radio_metadata"].keys() and msg["radio_metadata"]["rssi"] is not None:
        obs = {
                          "observedProperty": "signal_strength",
                          "resultTime": unix_timestamp,
                          "result": {
                            "value": msg["radio_metadata"]["rssi"],
                            "uom": "http://qudt.org/vocab/unit/DeciB"
                     }
                    }
        
        observations.append(obs)
    
    print("Here I am!")

    if "states" in msg:
        for (s,v) in msg["states"].items():
            obs = {
                          "observedProperty": state_prefix+"state"+s+"_fraction",
                          "resultTime": unix_timestamp,
                          "result": {
                            "value": v,
                            "uom": "http://www.qudt.org/vocab/unit/UNITLESS"
                     }
                    }
            observations.append(obs)

    print("Almost...")

    out_msg = {
        "resourceId": ressourceID,
        "sequenceNumber" : msg["seq_number"],
        "location" : {
                "latitude" : msg["latitude"],
                "longitude" : msg["longitude"],
                "altitude" : msg["altitude"]
            },
        "observations": observations,        
    }

    print("Bye...")


    return out_msg


# sample message:
# {"full_waypoint": 1,
# "timestamp": "2019-05-02T08:48:37+00:00",
# "latitude": 59.482015,
# "longitude": 18.287408,
# "altitude": 24.0,
# "speed": 0.0,
# "tracker_ID": "0004A30B001ECF9B",
# "radio_metadata": {"frequency": 867.3, "data_rate": "SF12BW125", "coding_rate": "4/5", "airtime": -1, "frame_counter": 4550, "best_gateway_id": "008000000000B775", "rssi": -97, "snr": 9.2},
#  "seq_number": 55}

### Template:
#{
#  "collar": {
#    "resourceId": "urn:afc:scenario:service:provider:type:entityName",
#    "location": {
#      "latitude": 45.45123,
#      "longitude": 25.25456,
#      "altitude": 2.10789
#    },
#    "resultTime": 1558086914,
#    "resourceAlarm": false,
#    "anomalies": {
#      "locationAnomaly": false,
#      "temperatureAnomaly": false,
#      "distanceAnomaly": false,
#      "activityAnomaly": true,
#      "positionAnomaly": false
#    },
#    "acceleration": {
#      "accX": 0.2331,
#      "accY": 0.898,
#      "accZ": 0.998
#    },
#    "temperature": 36.5
#  },
#  "sequenceNumber": 123
#}




#Sample afarcloud measurement

#{
#  "sequenceNumber": 123,
#  "location": {
##    "latitude": 45.45123,
# #   "longitude": 25.25456,
#    "altitude": 2.10789
#  },
#  "observation": {
#    "resourceId": "urn:afc:AS04:environmentalObservations:TST:airTemperatureSensor0012",
#    "observedProperty": "http://environment.data.gov.au/def/property/air_temperature",
#    "resultTime": 1558086914,
#    "result": {
#      "value": 3.2,
#      "uom": "http://qudt.org/vocab/unit#DegreeCelsius",
#      "variance": 0
#    }
#  }
#}


