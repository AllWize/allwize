# AllWize Library Change Log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [1.1.6] 2021-03-02
### Fixed
- Fix byte order in wize frame timestamp

### Added
- Added several examples (TPL5111, transparent master)
- Added MAC2 check only flag methods
- Wize channel numbering (CHANNEL_100 to CHANNEL_150)
- Using seconds since boot for wize frame timestamp

### Deprecated
- Removed deprecated operator_id

## [1.1.5] 2020-05-27
### Fixed
- Fix wrong L6 size

### Changed
- Wize2MQTT example now outputs a JSON with all data in it, also pings every 30s

## [1.1.4] 2019-08-22
### Fixed
- Fix wrong buffer length when CI not WIZE (undocumented?)

### Added
- Examples using CayenneLPP and MBUSPayload payload encoders
- Non-blocking delay for esp8266 boards

### Changed
- Reduce payload size when using AllWize_LoRaWAN class by merging Wize and LoRaWAN headers
- Change examples to not use pointer to class objects
- AllWize_LoRaWAN class using main class frame counter
- Overall examples clean up

## [1.1.3] 2019-08-14
### Fixed
- Fix documentation issues preventing correct checkout (#9)

### Added
- Added missing documentation to some methods
- Added support for [AllWize K2 board definition](https://github.com/AllWize/allwize-boards)

### Changed
- Removed dependency on SoftwareSerial for Atmel AVR platform

## [1.1.2] 2019-07-31
### Fixed
- Removed dependency on ESPSoftwareSerial for ESP8266 since it's already in SDK
- Fixed keywords.txt file (thanks to @per1234)

### Added
- Board info to ESP8266 examples

### Changed
- Moved soft_reset to softReset to fit standards

## [1.1.1] 2019-07-05
### Fixed
- Avoid reset if wiring problems in ESP8266 boards
- Reset line after reception

### Added
- Allow overriding USE_MEMORY_CACHE setting
- Define monitor_speed in platformio.ino files

### Changed
- ESP8266 examples pinout change
- Refactor examples

## [1.1.0] 2019-06-26
### Fixed
- Fixed variable collision when debug enabled
- Removed unneeded flushes
- Check datarates depending on module type
- Fix WIZE module returning only 255 bytes on TEST0
- Define C and CI fields as per standard
- MBUS4 support fixed
- Fixed sercom definition for AllWize K2
  
### Added
- Support for physical config pin
- UID examples
- Online docs
- Methods to change timeout and baudrate 

### Changed
- Moved OMS and RC1701HP definitions to their own files

## [1.0.3] 2019-04-05
### Added
- Example support for the AllWize K2 and Moteino boards
- setUID and setMID methods
- Added air quality example with MH-Z16 and BME280

### Fixed
- Fixed snfloat

## [1.0.2] 2018-10-17
### Fixed
- Fixed channel numbering

## [1.0.1] 2018-10-11
### Fixed
- Fixed possible overflow (thanks to @mariusmonton)

## [1.0.0] 2018-10-05
First supported release
