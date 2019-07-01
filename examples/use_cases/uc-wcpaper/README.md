# AllWize - WC paper monitoring

This example use case uses a simple analog potentiometer to calculate
the angle of a lever lying over the paper roll. If the angle is greater than a
certain threshold an alarm should be trigger to notify **someone** that we are
about to run out of paper.

If you are using the AllWize K1 you must set the sensor power jumper to the nominal value of
the board you are using (5V for Uno/Leonardo, 3V3 almost anywhere else) and
connect your potentiometer to the analog grove connector (the middle one).
Check that the yellow cable goes to the center pin of the pot.

To calibrate the sensor change the following settings to proper values:

#define VALUE_FOR_0_DEG     526     // value of the pot when the lever is completely horizontal
#define VALUE_FOR_90_DEG    175     // value of the pot when the lever is completely vertical
#define ANGLE_THRESHOLD     22      // angle at which the lever lies flat over an empty toilet roll

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
