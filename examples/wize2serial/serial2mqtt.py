#
#
# AllWize - WIZE 2 Serial Bridge with CayenneLPP
#
# Listens to messages in the serial port, decodes using CayenneLPP and forwards them to an MQTT broker.
# Requires pyserial, paho-mqtt and python-cayennelpp packages: 
# 
# `pip install pyserial paho-mqtt python-cayennelpp`.
#
# You might also need to give permissions to the dialout group to the current user
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
import paho.mqtt.client as mqtt
from python_cayennelpp.decoder import decode

# configuration
SERIAL_PORT = "/dev/ttyACM0"
SERIAL_BAUD = 115200

MQTT_TOPIC = "device/%s/payload"
MQTT_SERVER = "localhost"
MQTT_PORT = 1883
#MQTT_USER =
#MQTT_PASS =
MQTT_QOS = 2
MQTT_RETAIN = 0

def send(topic, payload):
    print("[MQTT] Sending %s => %s" % (topic, payload))
    client.publish(topic, payload, MQTT_QOS, MQTT_RETAIN)

# parse message
def parse(data):

    parts = data.rstrip().split(",")
    if len(parts) != 4:
        print("[PARSER] Wrong number of fields")
        return
    channel = parts[0]
    datarate = int(parts[1])
    rssi = int(parts[2])
    raw = parts[3]
    device = raw[4:20]
    payload = raw[32:-6]
    fields = decode(payload)

    fields = decode(payload)
    if len(fields) == 0:
        send("device/%s/raw" % (device), payload)
    else:
        for f in fields:
            name = f["name"].replace(' ', '_').lower()
            if isinstance(f["value"], dict):
                for k, v in f["value"].items():
                    name = k.replace(' ', '_').lower()
                    topic = "device/%s/%s" % (device, name)
                    send(topic, v)
                    
            else:
                topic = "device/%s/%s" % (device, name)
                send(topic, f["value"])

# callback functions
def on_connect(client, userdata, flags, rc):
    print("[MQTT] Connected")

client = mqtt.Client()
client.on_connect = on_connect
print("[MQTT] Connecting...")
client.connect(MQTT_SERVER, MQTT_PORT, 60)

# connect to device
ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=0.5)
while True:
    line = ser.readline()
    if len(line) and line[0] != "#":
        print("[SERIAL] Received %s" % line.rstrip())
        parse(line)
    client.loop()
