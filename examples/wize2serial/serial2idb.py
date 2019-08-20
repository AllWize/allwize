#
#
# AllWize - WIZE 2 InfluxDB Bridge with CayenneLPP
#
# Listens to messages in the serial port and inserts them to an InfluxDB instance.
# Requires pyserial, requests and python-cayennelpp packages: 
# 
# `pip install pyserial requests python-cayennelpp`.
#
# You might need to give permissions to the dialout group to the current user
# if on Linux /Raspberry Pi): `sudo adduser $USER dialout`
#
# Copyright (C) 2018-2019 by AllWize <github@allwize.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import serial
import requests
from python_cayennelpp.decoder import decode

# configuration
SERIAL_PORT = "/dev/ttyACM0"
SERIAL_BAUD = 115200

IDB_DATABASE = "data"
IDB_ENDPOINT = "http://localhost:8086/write?db=bridge"

def send(data):
    print("[IDB] Inserting %s" % data)
    #requests.post(url = IDB_ENDPOINT, data = data)

# parse message
def parse(data):

    parts = data.rstrip().split(",")
    if len(parts) != 4:
        print("[PARSER] Wrong number of fields")
        return
    device = parts[0]
    counter = parts[1]
    rssi = parts[2]
    payload = parts[3]
    fields = decode(payload)

    send("rssi,uid=%s value=%s" % (device, rssi))
    for f in fields:
        name = f["name"].replace(' ', '_').lower()
        if isinstance(f["value"], dict):
            data = "%s,uid=%s" % (name, device)
            sep = " "
            for k, v in f["value"].items():
                name = k.replace(' ', '_').lower()
                data = "%s%s%s=%s" % (data, sep, name, v)
                sep = ","
            send(data)
                
        else:
            send("%s,uid=%s value=%s" % (name, device, f["value"]))

# connect to device
ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=0.5)
while True:
    line = ser.readline()
    if len(line) and line[0] != "#":
        print("[SERIAL] Received %s" % line.rstrip())
        parse(line)
