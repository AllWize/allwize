# AllWize Library Change Log

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

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
