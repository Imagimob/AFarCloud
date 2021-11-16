# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Python implementation of the payload encoder running in the firmware --- for testing purposes


from lib_waypoints import decode_battery, decode_states, Waypoint, states_dictionary
from lib_sysutils import dict_compare
import base64
import json

import time


def encode_battery(batt: float) -> bytes:
    assert((batt <= 5.11) and (batt >= 2.56)) , "Can only encode voltages in [2.56,5.11]"
    return int(batt*100-256).to_bytes(1,"little")

assert(decode_battery(encode_battery(3.7)[0]) == 3.7) , "Battery Encoding/Decoding test failed" 

def encode_state(fraction : float) -> bytes:
    '''Encodes one state'''
    assert((fraction <= 1.0) and (fraction>=0.0)) , "State fraction must be in [0,1]"
    return round(fraction*255).to_bytes(1,"little")

def encode_states(states : list) -> bytes: 
    out = bytes()
    #assert(sum(states)==1) , "State fractions must sum to 1"
    assert(len(states)>1) , "We need at least 2 states"

    for s in states[1:]:
        out+=encode_state(s)
    return out
       

test_v = [0.1, 0.2, 0.3, 0.4]
test_p = 3
assert(dict_compare (decode_states(encode_states(test_v),test_p),
                    {k : v for (k,v) in zip(states_dictionary[test_p],test_v)},eps=1.0/255.0))


def encode_waypoint(fix,previousFix=None):

    lat = round(fix.get_lat() * 1000000)
    lon = round(fix.get_lon() * 1000000)


    # Is gap small enough? If too large gap in time, altitude or distance, we will send it in full instead.
    incremental = False
    if previousFix is not None:
        prevLat = round(previousFix.get_lat() * 1000000)
        prevLon = round(previousFix.get_lon() * 1000000)
        deltat = round(fix.get_timestamp() - previousFix.get_timestamp() )

        incremental = ( (deltat < 255) and (deltat>0) and 
                      (abs(lat - prevLat) < 32767) and 
                      (abs(lon - prevLon) < 32767) and
                      (abs(fix.get_alt() - previousFix.get_alt()) < 127))

    #Create full or incremental waypoint payload
    message = bytes()
    if incremental:
        message += bytearray.fromhex("00")  #not FULLFIX flag
        message += round(fix.get_timestamp()-previousFix.get_timestamp()).to_bytes(1,"little",signed=False)
        message += (lat - prevLat).to_bytes(2,"little",signed=True)
        message += (lon - prevLon).to_bytes(2,"little",signed=True)
        message += round(fix.get_alt()-previousFix.get_alt()).to_bytes(1,"little",signed=True)
        message += round(fix.get_speed()).to_bytes(1,"little",signed=False)
    else:
        message += bytearray.fromhex("01")  #FULLFIX flag
        message += round(fix.get_timestamp()).to_bytes(4,"little",signed=True)
        message += lat.to_bytes(4,"little",signed=True)
        message += lon.to_bytes(4,"little",signed=True)
        message += round(fix.get_alt()).to_bytes(2,"little",signed=True)
        message += round(fix.get_speed()).to_bytes(1,"little",signed=False)

    return message



__test_wp = Waypoint(
                full_waypoint=1,
                timestamp=round(time.time()),
                latitude=47.202776,
                longitude=1.900530,
                altitude=15,
                speed=25
                )
assert(encode_waypoint(__test_wp)[0]==1)              
assert(__test_wp==Waypoint.decode_Full_Waypoint(encode_waypoint(__test_wp)[1:]))


__test_wp2 = Waypoint(
                full_waypoint=0,
                timestamp=round(time.time())+2,
                latitude=47.202776-0.005,
                longitude=1.900530+0.005,
                altitude=15+1,
                speed=25+2
                )

assert(encode_waypoint(__test_wp2,previousFix=__test_wp)[0]==0)
assert(__test_wp2==Waypoint.decode_Diff_Waypoint(
                        encode_waypoint(__test_wp2,previousFix=__test_wp)[1:],
                        ref_full_waypoint=__test_wp
                        ))

__test_wp2 = Waypoint(
                full_waypoint=1,
                timestamp=round(time.time())+1574,
                latitude=14,
                longitude=-15,
                altitude=1824,
                speed=0
                )

assert(encode_waypoint(__test_wp2,previousFix=__test_wp)[0]==1)
assert(__test_wp2==Waypoint.decode_Full_Waypoint(
                        encode_waypoint(__test_wp2,previousFix=__test_wp)[1:],
                        ))


def encode_waypoints(waypoints : list, previousWp = None):
    if len(waypoints) >0:
        currentWP = waypoints[0]
        return encode_waypoint(fix=currentWP,previousFix=previousWp) + encode_waypoints(waypoints[1:],previousWp=currentWP)
    return bytes()

def encode_message(
            battery : int,
            states : list,
            waypoints : list = [],
            port = int,
        ) -> bytes :
    assert(port in states_dictionary.keys()) , "Undefined port number"
    
    payload = bytes()

    payload += encode_battery(battery)
    payload += encode_states(states)

    payload += encode_waypoints(waypoints)


    return payload


def static_vars(**kwargs):
    def decorate(func):
        for k in kwargs:
            setattr(func, k, kwargs[k])
        return func
    return decorate


@static_vars(counter=0)
def genDummyMessage_ttn():
    import random 
    import pytz
    import datetime


    devidx = random.randint(1, 2)
    if devidx==2 : devidx=1
    else: devidx=4
    waypoints = (devidx>3)

    dev_id = "ttgo"+str(devidx)
    
    app_id = "afarcloud"

    topic = app_id+"/devices/"+dev_id+"/up"

    genDummyMessage_ttn.counter += 1

    #Generate garbage data to create a payload
    port = 5

    states = [random.random() for x in states_dictionary[port]]
    states = [x/sum(states) for x in states]
    states[0] = 1.0-sum(states[1:])

    if waypoints:
        wp = Waypoint(
            full_waypoint=1,
            timestamp=round(time.time()),
            latitude=59.3293+random.random()-0.5,
            longitude=18.0686+random.random()-0.5,
            altitude=round(500*random.random()),
            speed=round(10*random.random())
        )

        wp2 = Waypoint(
            full_waypoint=1,
            timestamp=round(time.time()),
            latitude=59.3293+random.random()-0.5,
            longitude=18.0686+random.random()-0.5,
            altitude=round(500*random.random()),
            speed=round(10*random.random())
        )

        payload = encode_message(
                        battery=3.75,
                        states=states,
                        waypoints = [wp, wp2],
                        port = port
                    )
    else:
        payload = encode_message(
                battery=3.75,
                states=states,
                waypoints = [],
                port = port
                    )

    payload = base64.b64encode(payload).decode("ascii")


    # An example  message from TTN
    message = {
            "app_id": app_id,
            "dev_id": dev_id,
            "hardware_serial": "00000000000000"+dev_id[-2:],
            "port": port,
            "counter": genDummyMessage_ttn.counter,
            "payload_raw": payload,

            "metadata": {"time": datetime.datetime.now(tz=pytz.utc).isoformat().replace('+00:00', 'Z'),
                         "frequency": 867.7,
                         "modulation": "LORA",
                         "data_rate": "SF7BW125",
                         "airtime": 66816000,
                         "coding_rate": "4/5",
                         "gateways": [{
                                "gtw_id": "akersberga_1",
                                "gtw_trusted": True,
                                "timestamp": 504697796,
                                "time": datetime.datetime.now(tz=pytz.utc).isoformat().replace('+00:00', 'Z'),
                                "channel": 6,
                                "rssi": -119,
                                "snr": -0.25,
                                "rf_chain": 0,
                                #Malma Gård
                                "latitude": 59.277480 ,
                                "longitude": 18.612695,
                                "altitude": 35,
                                "location_source": "registry"
                            },
                                {"gtw_id": "eui-008000000000b775",
                                "timestamp": round(time.time()),
                                "time": "",
                                "channel": 6,
                                "rssi": -125,
                                "snr": 5.2,
                                "rf_chain": 0}]}}

    msg_str = json.dumps(message)
    return (topic,msg_str)