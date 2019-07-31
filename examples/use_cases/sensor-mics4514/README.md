# AllWize - Pollution monitoring

This example use cases uses a MICS-4514 sensor to retrieve CO and NO2 values.

If you are using the AllWize K1 you must connect the grove cable to the analog
port (the middle one). Connect the yellow cable leading to that port (A0) to the NOX pin
and the white cable to the RED pin.

You will also need a simple dupond cable between D8 and the PRE pin in the sensor,
this pin us used to trigger the pre-heat process.

The sensor is a 5V sensor so ensure you have the power jumper properly set.

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
