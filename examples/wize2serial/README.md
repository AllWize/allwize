# AllWize - WIZE 2 Serial Bridge

Listens to messages on the same channel, data rate and CF and forwards them via serial.

The payload format is very simple at the moment being just a comma separated list of values.
The topic is configured to be a placeholder with the node identification
(now using the message CI file) and an index for each value in the payload.

The `serial2mqtt.py` script show how to parse and forward the messages to an MQTT broker

Always remember to connect the antenna before powering the board!

## License

Copyright (C) 2018-2019 by AllWize ([http://allwize.io](http://allwize.io))

AllWize library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

AllWize library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with AllWize library.  If not, see <http://www.gnu.org/licenses/>.
