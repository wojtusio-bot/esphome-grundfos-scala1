# ESPHome Grundfos SCALA1

Local Bluetooth integration for **Grundfos SCALA1** pressure boosters using **ESP32 + ESPHome + Home Assistant**.

> Project status: **experimental external component**
>
> The reference unit is a SCALA1 3-45. Physical START and STOP are confirmed working.

## What already works

- BLE connection to SCALA1
- local operation without Grundfos Cloud
- physical **START**
- physical **STOP**
- pump running state
- ready / idle state
- manual STOP state
- dry-run / no-water state
- raw A4 diagnostics
- raw frame diagnostics
- Home Assistant entities through ESPHome
- reusable ESPHome external component

## Tested hardware

- Pump: Grundfos SCALA1 3-45
- Product no.: 99530405
- Application firmware: `99545258V01.00.02.000 01`
- BLE firmware: `99545256V03.00.05.00 001`
- ESP32: ESP32-WROOM-32 / esp32dev
- ESPHome: 2026.9.0

Other SCALA1 variants or firmware versions may behave differently. Please report results in Issues.

## Install as an ESPHome external component

Add the repository:

```yaml
external_components:
  - source: github://wojtusio-bot/esphome-grundfos-scala1@main
    components: [scala1]
    refresh: 1h
```

Configure BLE:

```yaml
esp32_ble_tracker:

ble_client:
  - mac_address: "AA:BB:CC:DD:EE:FF"
    id: scala1_ble
    auto_connect: true
```

Then add SCALA1:

```yaml
scala1:
  id: scala1_pump
  ble_client_id: scala1_ble
  update_interval: 5s

  control:
    name: "SCALA1 Control"

  connected:
    name: "SCALA1 BLE Connected"

  running:
    name: "SCALA1 Pump Running"

  no_water:
    name: "SCALA1 No Water"

  manual_stop:
    name: "SCALA1 Manual STOP"

  state:
    name: "SCALA1 State"

  state_code:
    name: "SCALA1 State Code"
```

The full example, including optional diagnostics, is here:

- [examples/scala1_external_component.yaml](examples/scala1_external_component.yaml)

The original large YAML reference implementation remains here:

- [examples/scala1_esp32.yaml](examples/scala1_esp32.yaml)

## BLE protocol

The pump uses:

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

The external component reproduces that sequence.

See [docs/protocol.md](docs/protocol.md).

## Current state codes

| Code | Meaning |
|---|---|
| `01 00` | Pump running |
| `6D 01` | Enabled / ready / idle |
| `01 01` | Manual STOP |
| `08 01` | No water / dry-run fault |

## Optional diagnostics

The component can expose:

- `a4_raw`
- `a4_hex`
- `rx_frames`
- `last_frame`

The meaning of A4 is **not yet confirmed**, so it remains raw diagnostic data.

## Roadmap

### v0.1
- [x] BLE connection
- [x] START
- [x] STOP
- [x] running / idle / manual stop / dry-run states
- [x] raw frame diagnostics
- [x] protocol notes
- [x] reusable ESPHome external component

### v0.2
- [ ] test the external component on additional SCALA1 units
- [ ] automatic pump discovery
- [ ] stronger command acknowledgement / retry logic
- [ ] improved reconnect handling
- [ ] compatibility matrix for more firmware versions

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
