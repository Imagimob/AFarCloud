# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

# Logic for ungrouping waypoints and interpreting the states




import time 


# A dictionary of states givent as { LoRa_port_number : ["cow sleeping", "cow dancing tango",...]}
states_dictionary = {
    3 : ["1" , "2"     , "3"     , "4"],          #["Cow_resting" , "Cow_walking"     , "Cow_grazing"     , "Cow_other"]
    4 : ["1"  , "2" , "3" , "4"],                 #["Tractor_off"  , "Tractor_idling" , "Tractor_on_road" , "Tractor_in_field"],
    5 : ["1","2","3","4","5","6","7","8"],
}


class Waypoint:
    full_Length = 16
    diff_Length = 8

    def __init__(self, full_waypoint = None, timestamp = None,  latitude: float = None, longitude: float = None, altitude: float = None, speed: float = None,battery: float = None, states= None):

        self.data=dict()
        self.data["full_waypoint"] = full_waypoint if full_waypoint is not None else 0
        self.data["timestamp"] = timestamp if timestamp is not None else int(time.time())
        self.data["latitude"] = float(latitude) if latitude is not None else 0.0
        self.data["longitude"] = float(longitude) if longitude is not None else 0.0
        self.data["altitude"] = float(altitude) if altitude is not None else 0.0
        self.data["speed"] = float(speed) if speed is not None else 0.0
        if battery is not None:
            self.data["battery"]=battery
        if states is not None:
            self.data["states"]=states

        if self.data["timestamp"] == 0: self.data["timestamp"] = int(time.time())
        print("################# New waypoint")
        print(self.data["timestamp"])

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return str(self.data)

    def get_lat(self):
        return self.data["latitude"]

    def get_lon(self):
        return self.data["longitude"]

    def get_alt(self):
        return self.data["altitude"]

    def get_timestamp(self):
        return self.data["timestamp"]
    
    def get_speed(self):
        return self.data["speed"]

    def  __eq__(self,wp):
        try:
            for k in self.data.keys():
                if k=="states":
                    continue

                if not (self.data[k]==wp.data[k]):
                    return False

            return True
        except:
            return False
        return False

    def __ne__(self,wp):
        return not self.__eq__(wp)


    def decode_Full_Waypoint(array: bytes,batt=None):
        return Waypoint(
            full_waypoint = 1,
            timestamp = int.from_bytes(array[0:4],byteorder='little',signed=True),
            latitude = int.from_bytes(array[4:8],byteorder='little',signed=True)/ 1000000.0,
            longitude = int.from_bytes(array[8:12],byteorder='little',signed=True)/ 1000000.0,
            altitude = int.from_bytes(array[12:14],byteorder='little',signed=True),
            speed = int.from_bytes(array[14:15],byteorder='little',signed=False),
            battery = batt
        )

    def decode_Diff_Waypoint(array: bytes,ref_full_waypoint,batt=None):
        print("Diff waypoint")
        return Waypoint(
            full_waypoint = 0,
            timestamp = int.from_bytes(array[0:1],byteorder='little',signed=False)+ref_full_waypoint.get_timestamp(),
            latitude =  int.from_bytes(array[1:3],byteorder='little',signed=True)/ 1000000.0 +ref_full_waypoint.get_lat(),
            longitude =  int.from_bytes(array[3:5],byteorder='little',signed=True)/ 1000000.0 +ref_full_waypoint.get_lon(),
            altitude = int.from_bytes(array[5:6],byteorder='little',signed=True)+ref_full_waypoint.get_alt(),
            speed = int.from_bytes(array[6:7],byteorder='little',signed=False),
            battery = batt
        )

    def decode_noGPS_Waypoint(batt=None):
        print("No GPS waypoint")
        return Waypoint(
            battery = batt
        )


    def calculate_distance_GPS(self,lat: float ,lon: float):
       import math.radians as deg2rad
       from math import sin, cos, atan2, sqrt

       R = 6371 # Radius of the earth in km

       lat1 = self.get_lat()
       lon1 = self.get_lon()

       dLat = deg2rad(lat2 - lat1)
       dLon = deg2rad(lon2 - lon1)

       a = sin(dLat / 2) * sin(dLat / 2) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * sin(dLon / 2) * sin(dLon / 2)
       c = 2 * atan2(sqrt(a), sqrt(1 - a))
       d = R * c # Distance in km
       return d * 1000

    def to_dict(self):
        return self.data

    def from_dict(d):

        assert((d["full_waypoint"] in [0,1]))
        assert(type(d["timestamp"]) == int)

        return Waypoint(
                        full_waypoint=d["full_waypoint"],
                        timestamp=d["timestamp"],
                        latitude = d["latitude"],
                        longitude = d["longitude"],
                        altitude=d["altitude"],
                        speed=d["speed"],
                        battery=d["battery"] if "battery" in d.keys() else None,
                        states=d["states"] if "states" in d.keys() else None)

    def to_influxDB(self,measurement_name="Waypoint"):
        return {
            "measurement": measurement_name,
            "tags": { k : self.data[k] for k in ["full_waypoint"]},
            "timestamp": self.data["timestamp"]*int(10e9),  #InfluxDB takes timestamps in nanoseconds
            "fields" :  { k : self.data[k] for k in ["latitude","longitude","altitude","speed","full_waypoint"] if self.data[k] is not None}
        }

    def add_battery(self,battery):
        self.data["battery"] = battery

    def add_states(self,states):
        self.data["states"] = states



def decode_raw_payload(payload,batt=None):
    Full_Fix_mask = 1
    waypoints = []
    idx = 0

    print("In decoder")
    print(payload)


    while (idx<len(payload)):
        print("Loop")
        if (payload[idx] & Full_Fix_mask):
            print("Full")
            l = Waypoint.full_Length
            wp = Waypoint.decode_Full_Waypoint(payload[idx+1:idx+l],batt)
            waypoints.append(wp)
        else:
            print("Diff")
            l = Waypoint.diff_Length
            try: 
                wp = Waypoint.decode_Diff_Waypoint(payload[idx+1:idx+l],wp,batt)
            except: 
                wp = Waypoint.decode_noGPS_Waypoint(batt)
        waypoints.append(wp)
        print(wp)
        idx+=l

    return waypoints

def decode_battery(batt: int):
    return (batt+256)/100

 

def decode_states(states : bytes,port : int) -> dict:
    assert(len(states) == len(states_dictionary[port])-1 ) , "Wrong combination of array len / port number."

    out = { k : float(v)/255.0 for (k,v) in zip(states_dictionary[port][1:],states)}
    
    out[states_dictionary[port][0]] = 1.0 - sum(out.values())
    return out


def decode_raw_payload2(payload):
    if len(payload)>1:
        last_byte = payload[-1]
        payload = payload[0:-1]

        battery = decode_battery(last_byte)

        return decode_raw_payload(payload,batt=battery)
    else:
        battery = decode_battery(payload[0])

        import  time

        return [Waypoint(
            full_waypoint = None,
            timestamp = int(time.time()),
            latitude =  None,
            longitude =  None,
            altitude = None,
            speed = None,
            battery = battery)]


def decode_raw_payload3(payload,port):

    battery = decode_battery(payload[0])

    NbStates = len(states_dictionary[port])

    states = decode_states(
            states= payload[1:NbStates],
            port=port
            )
    payload = payload[NbStates:]   

    waypoints = decode_raw_payload(payload)

    return (battery,states,waypoints)

def parse_standardized_message(msg_dict):
    payload = bytes.fromhex(msg_dict["hex_payload"])
    port = msg_dict["port"]

    (battery,states,waypoints) = decode_raw_payload3(payload,port)
    print("port is " + str(port))
    if waypoints==[]:
        print("empty waupoint")
        from dateutil.parser import isoparse

        #If we have no GPS, we try to get from gateway
        if "localized_gateway" in msg_dict:
            print("localized_gw")
            waypoints.append(
                Waypoint(
                    timestamp = round(isoparse(msg_dict["timestamp"]).timestamp()),
                    latitude = msg_dict["localized_gateway"]["latitude"],
                    longitude = msg_dict["localized_gateway"]["longitude"],
                    altitude = msg_dict["localized_gateway"]["altitude"],
                )
            )
        else:
            print("Last resort")
            waypoints.append(
                Waypoint(
                    timestamp = round(isoparse(msg_dict["timestamp"]).timestamp()),
                )
            )

    #We will add the extra data to the first waypoint only. We'll need to make sure that this is the latest point collected in the firmware
    waypoints[0].add_battery(battery)
    waypoints[0].add_states(states)

    return waypoints

