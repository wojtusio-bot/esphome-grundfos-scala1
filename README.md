# ESPHome Grundfos SCALA1

Experimental local Bluetooth integration for **Grundfos SCALA1** pressure boosters using an **ESP32 + ESPHome + Home Assistant**.

> Project status: **early v0.1 / reverse engineering in progress**
>
> The current reference implementation has been physically tested with a SCALA1 3-45 and supports real START/STOP control plus basic state decoding.

## What already works

- BLE connection to SCALA1
- local control without Grundfos Cloud
- physical **START**
- physical **STOP**
- pump running state
- manual STOP state
- dry-run / no-water state
- raw diagnostic frame capture
- Home Assistant entities through ESPHome

## Tested hardware

- Pump: Grundfos SCALA1 3-45
- Product no.: 99530405
- Application firmware: 99545258V01.00.02.000 01
- BLE firmware: 99545256V03.00.05.00 001
- ESP32: ESP32-WROOM-32 / esp32dev
- ESPHome: 2026.9.0

Other SCALA1 variants or firmware versions may behave differently. Please report results in Issues.

## BLE protocol

The pump advertises the Grundfos service:

- Service UUID: `FE5D`
- Characteristic UUID: `859CFFD1-036E-432A-AA28-1A0085B87BA9`
- Characteristic properties: Read, Write Without Response, Notify

The protocol is GENI-like and was reverse engineered from Bluetooth HCI captures generated while using Grundfos GO.

A key discovery is that START/STOP is **not sent as one long BLE write**. Grundfos GO sends:

1. command fragment 1
2. command fragment 2
3. short delay
4. synchronization frame fragment 1
5. synchronization frame fragment 2

This split is necessary for reliable physical control.

See [docs/protocol.md](docs/protocol.md).

## Current state codes

| Code | Meaning |
|---|---|
| `01 00` | Pump running |
| `6D 01` | Enabled / ready / idle |
| `01 01` | Manual STOP |
| `08 01` | No water / dry-run fault |

## Quick start

The current known-working reference implementation is in:

- [examples/scala1_esp32.yaml](examples/scala1_esp32.yaml)

You need:

- ESP32 with Bluetooth
- ESPHome
- Home Assistant
- the Bluetooth MAC address of your SCALA1

Do **not** publish your Wi-Fi password, Home Assistant API keys, or other secrets in GitHub.

## Roadmap

### v0.1
- [x] BLE connection
- [x] START
- [x] STOP
- [x] running / idle / manual stop / dry-run states
- [x] raw frame diagnostics
- [x] protocol notes

### v0.2
- [ ] convert reference YAML into a reusable ESPHome external component
- [ ] automatic pump discovery
- [ ] retry / command acknowledgement logic
- [ ] better reconnect handling
- [ ] compatibility testing with more SCALA1 firmware versions

### v0.3
- [ ] reverse engineer additional Grundfos GO parameters
- [ ] alarms and alarm history
- [ ] counters / runtime
- [ ] pressure / flow / power if exposed by this model
- [ ] configurable pump settings

## Contributing

If you own a SCALA1 and want to help, useful contributions include:

- model and product number
- application firmware version
- BLE firmware version
- ESPHome logs
- anonymized Bluetooth HCI captures
- confirmation whether START/STOP works on your firmware

Please remove personal data, Wi-Fi credentials, API keys, phone identifiers and unrelated Bluetooth traffic before publishing captures.

## Disclaimer

This is an independent community reverse-engineering project and is **not affiliated with or endorsed by Grundfos**.

Use at your own risk. Remote pump control can affect water supply and connected equipment. Keep the pump's original safety functions enabled.

## License

MIT License. See [LICENSE](LICENSE).
