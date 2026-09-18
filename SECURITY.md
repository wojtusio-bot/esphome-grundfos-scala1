# Security and privacy

This project communicates locally with a water pressure pump over Bluetooth.

## Do not publish secrets

Never commit or attach:

- Wi-Fi passwords
- Home Assistant API keys
- ESPHome encryption keys
- exact home GPS coordinates
- private account tokens
- full Android bugreports without reviewing them first

The repository ignores common Bluetooth capture and ZIP formats by default, but always review files before committing.

## Safety

A software command can enable or disable the pump. Keep all original Grundfos protections and installation safeguards active.

Do not use this project to bypass dry-run protection, thermal protection, pressure limits or other built-in safety features.

## Reporting security issues

For now, avoid posting credentials or sensitive captures in a public issue. Open an issue containing only a description of the problem and no secrets.
