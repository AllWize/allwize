# Allwize - WIZE 2 TheThings.IO

Listens to messages on the same channel, data rate and CF and
forwards them to an MQTT broker.
This example is meant to run on a Wemos D1 board (ESP8266).

Configuration: copy the `configuration.sample.h` file to `configuration.h` and
edit it to configure your WiFi network credentials and the TheThings.IO mapping.

The payload format is very simple at the moment being just a comma separated list of values.
The topic is configured to be a placeholder with the node identification
(now using the message CI file) and an index for each value in the payload.

The TheThings.IO mapping is encoded as a JSON document inside the code and parsed on boot.
This document is an array of devices. Each device has the following fields:

* `id` for the node id, currently using the CI field
* `token` for the thing token in TheThings.io
* `keys` is an array of keys that will be mapped to values in the message payload

Always remember to connect the antenna before powering the board!

## License

Copyright (C) 2018 by AllWize ([http://allwize.io](http://allwize.io))

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
