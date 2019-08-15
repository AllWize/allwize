# AllWize

Arduino-compatible library to interface **RC1701HP-OSP/WIZE radio modules**.

[![version](https://img.shields.io/badge/version-1.1.3-brightgreen.svg)](CHANGELOG.md)
[![travis](https://travis-ci.com/Allwize/allwize.svg?branch=dev)](https://travis-ci.com/Allwize/allwize)
[![codacy](https://img.shields.io/codacy/grade/5b0345d3b4994a1eb2e51f02fa9a5d22/dev.svg)](https://www.codacy.com/app/Allwize/allwize/dashboard)
[![license](https://img.shields.io/github/license/AllWize/allwize.svg)](LICENSE)

[![web](https://img.shields.io/badge/web-http%3A%2F%2Fallwize.io-yellowgreen.svg)](http://allwize.io)
[![twitter](https://img.shields.io/twitter/follow/allwize_iot.svg?style=social)](https://twitter.com/intent/follow?screen_name=allwize_iot)

Compatible radios:

*   RadioCrafts RC1701HP-OSP (Ondeo version)
*   RadioCrafts RC1701HP-WIZE (Wize version)

Compatible platforms:

*   AVR (Arduino Uno, Arduino Leonardo)
*   SAMD21 (Arduino Zero, Arduino MKR family, AllWize K2)
*   ESP8266
*   ESP32

## AllWize K2 support

The AllWize K2 is a SAMD21-based board with an RC1701HP-WIZE radio module.

### Arduino IDE

To add support for it in the Arduino IDE you must install a custom board. 
Full instrucations can be found at the [AllWize Wiki](http://wiki.allwize.io/index.php?title=Allwize_K2#Arduino_IDE), but a summary is:

* Install the current upstream Arduino IDE at the 1.8.7 level or later. The current version is on the Arduino website.
* Start Arduino and open the Preferences window.
* Enter https://raw.githubusercontent.com/AllWize/allwize-boards/master/package_allwize_boards_index.json into the Additional Board Manager URLs field. You can add multiple URLs, separating them with commas.
* Open Boards Manager from Tools > Board menu and install "Allwize SAMD Boards (32-bits ARM Cortex-M0+)"
* Don't forget to select your AllWize board from Tools > Board menu after installation.

### PlatformIO

The AllWize K2 is not yet supported officially in PlatformIO, still you can configure it manually. Check the [AllWize Wiki](http://wiki.allwize.io/index.php?title=Allwize_K2#Platform_IO) for further instructions.

## Documentation

Visit the [online documentation for the AllWize Library](https://allwize.github.io/allwize/classAllWize.html).

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
