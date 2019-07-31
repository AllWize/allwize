#
#
# AllWize - WIZE 2 Serial Bridge
#
# Listens to messages in the serial port and forwards them to an MQTT broker.
# Requires pyseria and paho-mqtt packages: `pip install pyserial paho-mqtt`.
# You might also need to give permissions to the dialout group to the current user
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
import paho.mqtt.client as mqtt

# configuration
SERIAL_PORT = "/dev/ttyACM0"
SERIAL_BAUD = 115200
MQTT_TOPIC = "team/%s/field_%d"
MQTT_SERVER = "localhost"
MQTT_PORT = 1883
#MQTT_USER =
#MQTT_PASS =
MQTT_QOS = 2
MQTT_RETAIN = 0

# parse message
def parse(data):
    parts = data.rstrip().split(",")
    fields = parts[5:]
    ci = parts[3]
    index = 0
    for payload in fields:
        index = index + 1
        topic = MQTT_TOPIC % (ci, index)
        print("[MQTT] Sending %s => %s" % (topic, payload))
        client.publish(topic, payload, MQTT_QOS, MQTT_RETAIN)

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
