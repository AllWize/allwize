# AllWize - Parking Use Case

This Use Case example uses an ultrasonic HC-SR04 sensor to detect an object over the device.
If there is something closer than 200mm (defined in the THRESHOLD_DISTANCE setting) it sends
a 1, otherwise it sends a 0. The measure is taken every SLEEP_TIME milliseconds.

The HC-SR04 is a 5V digital sensor.
If you are using the AllWize K1 you must set the sensor power jumper to 5V and
connect your sensor to the digital grove connector (the one closer to the USB connector).
The connect the white cable to the ECHO pin and the yellow one to the TRIG pin
in the HC-SR04.

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
