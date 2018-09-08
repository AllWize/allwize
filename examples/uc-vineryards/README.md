# AllWize - Vineryard monitoring

This example use cases uses a simple analog higrometer to measure humidity in
the soil as a measure of vineyard hidro-stress and a SI7021 I2C sensor to
retrieve environment temperature and humidity.

The higrometer is an analog sensor, but some also sport a comparator fed by a
potentiometer. Those will then also have a digital output. We are using here the
analog reading so we need that output. In case yours has a potentiometer please verify
it still has an analog out.

To calibrate the sensor immerse it in water and check the analog value
(see the getHumidity method in the code). Then set HIGROMETER_MIN to that value.

If you are using the Allwize K1 you must set the sensor power jumper to the nominal value of
the board you are using (5V for Uno/Leonardo, 3V3 almost anywhere else) and
connect your sensor to the analog grove connector (the middle one).
Check that the yellow cable goes to the analog output of the sensor (sometimes labeled A0).

The SI7021 is very straight forward to use. If using the K1 check that the yellow cable is
connected to SCL and the white one to SDA.

Always remember to connect the antenna before powering the board!

## License

Copyright (C) 2018 by AllWize (http://allwize.io)

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
