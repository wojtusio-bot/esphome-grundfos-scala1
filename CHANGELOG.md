# Changelog

## Unreleased

### Added

- initial public repository
- documented SCALA1 BLE service and characteristic
- documented confirmed status and A4 read requests
- documented physical START/STOP split-write sequence
- documented state codes:
  - `01 00` running
  - `6D 01` enabled / idle
  - `01 01` manual STOP
  - `08 01` no water / dry-run
- working ESPHome reference configuration
- compatibility issue template
- contribution and privacy guidance

### Tested

Reference hardware:

- Grundfos SCALA1 3-45
- Product no. 99530405
- Application firmware `99545258V01.00.02.000 01`
- BLE firmware `99545256V03.00.05.00 001`
- ESP32-WROOM-32
- ESPHome 2026.9.0

Physical START and STOP have been confirmed on the reference unit.
