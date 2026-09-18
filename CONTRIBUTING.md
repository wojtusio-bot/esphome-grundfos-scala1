# Contributing

Thanks for helping reverse engineer Grundfos SCALA1 support for ESPHome and Home Assistant.

## Useful test information

When opening an issue, include:

- exact SCALA1 model
- product number
- application firmware
- BLE firmware
- ESP32 board
- ESPHome version
- whether START physically starts the pump
- whether STOP physically stops the pump
- decoded state code before and after the command

## Logs

ESPHome DEBUG logs are useful.

Bluetooth HCI captures are even more useful when they contain a single clearly described action, for example:

1. connect with Grundfos GO
2. wait 5 seconds
3. press STOP
4. wait 5 seconds
5. press START
6. wait 5 seconds
7. stop capture

## Privacy

Before uploading logs or captures, remove or anonymize:

- Wi-Fi SSIDs and passwords
- Home Assistant API keys
- precise GPS location
- phone identifiers
- unrelated Bluetooth devices
- personal account information

Pump model, product number and firmware versions are useful and normally safe to include.

## Development direction

The current YAML is a working reference implementation. The next major goal is a reusable ESPHome external component so users do not need a large custom YAML file.
