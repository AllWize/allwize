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
# Copyright (C) 2018-2021 by AllWize <github@allwize.io>
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
import datetime

# configuration
SERIAL_PORT = "/dev/ttyACM0"
SERIAL_BAUD = 115200

# parse message
def parse(data):

    parts = data.decode().split(",")
    if len(parts) != 4:
        print("[PARSER] Wrong number of fields")
        return
    channel = parts[0]
    datarate = int(parts[1])
    rssi = int(parts[2])
    payload = parts[3]
    ts = datetime.datetime.utcnow().strftime('%Y-%m-%dT%H:%M:%SZ')

    print("%s,%s,%d,%d,%s" % (ts, channel, datarate, rssi, payload), flush=True)

# header
print("time,channel,datarate,rssi,payload", flush=True)

# connect to device
ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=0.5)
while True:
    line = ser.readline()
    if len(line) and line[0] != "#":
        parse(line.rstrip())
