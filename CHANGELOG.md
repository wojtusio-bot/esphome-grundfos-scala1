# Changelog

## Unreleased

### Added

- reusable ESPHome external component under `components/scala1`
- physical START/STOP control implemented directly in C++
- split BLE writes matching Grundfos GO behavior
- 6-second post-connect pairing/authentication settle period
- status polling and fragmented GENI reply reassembly
- Home Assistant entities for:
  - control
  - BLE connectivity
  - pump running
  - no-water / dry-run
  - manual STOP
  - decoded state
  - state code
  - optional A4 diagnostics
  - RX frame diagnostics
- external component example configuration
- local compile-test configuration
- GitHub Actions ESPHome validation workflow
- initial public repository
- documented SCALA1 BLE service and characteristic
- documented confirmed status and A4 read requests
- documented physical START/STOP split-write sequence
- compatibility issue template
- contribution and privacy guidance

### Confirmed protocol states

- `01 00` running
- `6D 01` enabled / idle
- `01 01` manual STOP
- `08 01` no water / dry-run

### Tested reference hardware

- Grundfos SCALA1 3-45
- Product no. 99530405
- Application firmware `99545258V01.00.02.000 01`
- BLE firmware `99545256V03.00.05.00 001`
- ESP32-WROOM-32
- ESPHome 2026.9.0

Physical START and STOP have been confirmed on the reference unit.
