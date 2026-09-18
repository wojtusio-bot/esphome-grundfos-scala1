# Grundfos SCALA1 BLE protocol notes

These notes document behavior observed from Bluetooth HCI captures while using Grundfos GO with a Grundfos SCALA1 3-45.

This is reverse-engineered behavior, not official Grundfos protocol documentation.

## GATT

- Service UUID: `FE5D`
- Characteristic UUID: `859CFFD1-036E-432A-AA28-1A0085B87BA9`
- Characteristic properties observed: Read, Write Without Response, Notify

## Frame conventions

Observed request frames begin with `27`.

Observed reply frames begin with `24`.

For replies, the expected complete frame length is:

```text
frame_length = frame[1] + 4
```

Examples:

- reply length byte `0x12` -> 22-byte frame
- reply length byte `0x16` -> 26-byte frame

Notifications may arrive fragmented and must be reassembled.

## Confirmed read requests

### Status request

```text
27 07 E7 F8 0A 03 56 00 08 24 94
```

### A4 request

```text
27 07 E7 F8 0A 03 5B 00 A4 12 A3
```

The semantic meaning of A4 is **not yet confirmed**. Keep it as raw diagnostic data.

## Confirmed state codes

| Code | Meaning |
|---|---|
| `01 00` | Pump running |
| `6D 01` | Enabled / ready / idle |
| `01 01` | Manual STOP |
| `08 01` | No water / dry-run fault |

## Physical STOP command

Grundfos GO sends the command as two BLE writes.

### STOP fragment 1 — 20 bytes

```text
27 14 E7 F8 0A 90 56 00 06 01 2F 01 00 00 07 01 01 00 00 00
```

### STOP fragment 2 — 4 bytes

```text
00 00 6A 76
```

A short delay is followed by the synchronization frame.

### Sync fragment 1 — 20 bytes

```text
27 17 E7 F8 0A 93 54 00 01 00 DA 01 00 00 0A 05 0A 00 0A 00
```

### Sync fragment 2 — 7 bytes

```text
00 00 00 00 00 55 F8
```

## Physical START command

### START fragment 1 — 20 bytes

```text
27 14 E7 F8 0A 90 56 00 06 01 2F 01 00 00 07 01 00 00 00 00
```

### START fragment 2 — 4 bytes

```text
00 00 2F D6
```

Then send the same synchronization frame shown above.

## Timing

The working ESPHome reference implementation currently uses:

- 2 ms between command fragments
- 30 ms before the sync frame
- 2 ms between sync fragments
- 35 ms before polling status

These values are based on successful physical START/STOP testing. They may be relaxed or parameterized later.

## Important implementation detail

Sending the 24-byte command as a single BLE write can change reported state without reliably producing the desired physical pump action.

For the tested firmware, reproducing the app's split-write behavior is required for reliable real-world START/STOP control.

## Tested firmware

Reference test unit:

- Grundfos SCALA1 3-45
- Product no. 99530405
- Application firmware: `99545258V01.00.02.000 01`
- BLE firmware: `99545256V03.00.05.00 001`

## Unknowns / next targets

- CRC algorithm and frame construction rules
- exact semantic meaning of A4
- alarms and alarm history
- runtime counters
- pressure, flow and power parameters
- configuration registers / writable settings
- compatibility across SCALA1 firmware revisions
