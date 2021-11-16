# 
# MIT License
# Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
#

from lib_waypoints import Waypoint, decode_raw_payload

import base64
coded_string = "AbDVkFz0nosD9woXAUAAAf4IEAA3AP8A/ggIAJv//gH+BBkAz/8AAv4FAAA/AAAA"
payload = base64.b64decode(coded_string)


wps = decode_raw_payload(payload)
print(wps)