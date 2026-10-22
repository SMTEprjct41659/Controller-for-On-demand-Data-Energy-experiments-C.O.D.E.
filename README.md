# C.O.D.E. — Controller for On-demand Data & Energy Experiments

An ESP32-based measurement and automation platform for renewable-energy power experiments.

C.O.D.E. controls experiment hardware, samples electrical and thermal measurements, calculates summary statistics, and reports results through Telegram.

The project was developed for the Science, Mathematics, Technology, and Environment project:

> Development of a Voltage Stabilization System for Renewable Energy-Based Power Generation Systems

The project received a Gold Medal and third place in an English oral presentation competition.

> **Project status:** Experimental and educational platform. Hardware calibration and electrical safety validation are required before collecting research data.

## Quick Navigation

- [Features](#features)
- [Experiments](#experiments)
- [Repository Structure](#repository-structure)
- [Hardware and Wiring](#hardware-and-wiring)
- [Software Requirements](#software-requirements)
- [Getting Started](#getting-started)
- [Telegram Operation](#telegram-operation)
- [Diagnostic Tools](#diagnostic-tools)
- [Safety](#safety)
- [Credential Security](#credential-security)

## Features

- ESP32-based experiment controller
- INA226 voltage, current, and power measurement
- Dual I²C bus support
- DS18B20 temperature monitoring
- Generator and capacitor relay control
- Automatic multi-run experiment sequences
- Statistical summaries for voltage, current, power, ripple, temperature, and efficiency
- Telegram-based remote control and reporting
- Standalone I²C scanning tool
- Standalone INA226 calibration tools
- Configurable pins, timing, sampling, addresses, and relay logic

## Experiments

### Experiment 1 — Renewable-Source Power Measurement

Measures the input voltage, current, and power of a selected renewable-energy source.

- 9 runs per experiment
- 5-minute measurement period per run
- 1-second sampling interval
- Hydro, wind, and solar operating modes
- Per-run summaries and a final report

### Experiment 2 — Capacitor Branch Testing

Tests a capacitor branch while measuring its electrical behavior and automatically discharging it after each run.

- Five capacitor branches supported by the relay controller
- Branch 1 selected by default by the master controller
- 9 runs per experiment
- 5-minute measurement period per run
- Automatic capacitor discharge
- Voltage, current, and power statistics

### Experiment 3 — Converter Performance and Temperature

Measures converter input and output performance while monitoring thermal behavior.

- 9 runs per experiment
- 1-hour measurement period per run
- Input and output voltage, current, and power
- Conversion-efficiency calculation
- Two DS18B20 temperature channels
- Mean, minimum, maximum, and ripple reporting

## Repository Structure

```text
.
├── I2CScanner/
│   └── I2CScanner.ino
│
├── INA226Calibrator/
│   ├── INA226CalibratorI.ino
│   └── INA226CalibratorO.ino
│
├── MasterController/
│   ├── MasterController.ino
│   ├── Experiment1.cpp
│   ├── Experiment1.h
│   ├── Experiment2.cpp
│   ├── Experiment2.h
│   ├── Experiment3.cpp
│   ├── Experiment3.h
│   ├── INA226ModuleI.cpp
│   ├── INA226ModuleI.h
│   ├── INA226ModuleO.cpp
│   ├── INA226ModuleO.h
│   ├── RelayController.cpp
│   ├── RelayController.h
│   ├── StatsEngine.cpp
│   ├── StatsEngine.h
│   ├── telegramchatbot.cpp
│   ├── telegramchatbot.h
│   └── config.h
│
└── README.md
```

### MasterController File Roles

| File | Role |
|---|---|
| `MasterController.ino` | Main firmware entry point and experiment scheduler |
| `Experiment1.cpp/.h` | Renewable-source power measurement |
| `Experiment2.cpp/.h` | Capacitor branch switching, measurement, and discharge |
| `Experiment3.cpp/.h` | Converter performance and temperature experiment |
| `INA226ModuleI.cpp/.h` | Primary-bus input measurements |
| `INA226ModuleO.cpp/.h` | Secondary-bus output measurements |
| `RelayController.cpp/.h` | Generator, capacitor, and discharge-relay control |
| `StatsEngine.cpp/.h` | Measurement statistics and reporting helpers |
| `telegramchatbot.cpp/.h` | Wi-Fi connection and Telegram communication |
| `config.h` | Pin assignments, I²C addresses, relay logic, and timing |

## Hardware and Wiring

The system is designed for an ESP32-compatible board connected to:

- INA226 input and output power monitors
- Two DS18B20 temperature sensors
- Generator and capacitor relays
- Renewable-energy source or generator
- DC-DC converter and load
- Wi-Fi network for Telegram communication

### Default I²C Configuration

| Bus | SDA | SCL | Purpose |
|---|---:|---:|---|
| Primary bus | GPIO 21 | GPIO 22 | INA226 input monitor |
| Secondary bus | GPIO 16 | GPIO 17 | INA226 output monitor |

The INA226 modules may use the same I²C address because they are connected to separate buses.

### Default Relay and Sensor Pins

| Function | GPIO |
|---|---:|
| Generator relay | 4 |
| Capacitor branch 1 | 23 |
| Capacitor branch 2 | 13 |
| Capacitor branch 3 | 14 |
| Capacitor branch 4 | 25 |
| Capacitor branch 5 | 26 |
| Capacitor discharge relay | 32 |
| DS18B20 data line | 19 |

Review `MasterController/config.h` before wiring the hardware. Relay polarity is controlled by `RELAY_ACTIVE_LOW`.

## Software Requirements

- Arduino IDE
- ESP32 board support package
- `INA226` library
- `OneWire` library
- `DallasTemperature` library
- `UniversalTelegramBot` library

The ESP32 framework provides the Wi-Fi, secure client, FreeRTOS, and queue functionality.

## Getting Started

1. Install Arduino IDE.
2. Install ESP32 board support.
3. Install the required libraries.
4. Open `MasterController/MasterController.ino`.
5. Review the settings in `MasterController/config.h`.
6. Configure Wi-Fi and Telegram credentials locally.
7. Select the correct ESP32 board and serial port.
8. Upload the firmware.
9. Open Serial Monitor at `115200` baud.
10. Use Telegram to select and start an experiment.

The repository intentionally contains blank Telegram and Wi-Fi credentials. Add real credentials only to a private local copy.

## Telegram Operation

The Telegram interface allows the user to:

1. Select Experiment 1, 2, or 3
2. Select hydro, wind, or solar mode
3. Start the selected experiment
4. Stop a running experiment
5. Receive run summaries and final reports

Only one experiment can run at a time.

## Diagnostic Tools

### I2CScanner

Upload:

```text
I2CScanner/I2CScanner.ino
```

This sketch scans both ESP32 I²C buses and helps verify that the INA226 devices are connected and visible at the expected addresses.

### INA226Calibrator

The calibration sketches are located in:

```text
INA226Calibrator/
```

- `INA226CalibratorI.ino` — primary INA226 module
- `INA226CalibratorO.ino` — output INA226 module

Use these sketches during hardware setup and sensor verification before running the full controller.

## Configuration

Most hardware and timing settings are located in:

```text
MasterController/config.h
```

Configurable settings include:

- I²C pins
- I²C addresses
- Relay pins
- Relay active-high or active-low behavior
- Experiment durations
- Number of runs
- Sampling interval
- Settling and pre-measurement delays

Default experiment timing:

| Setting | Value |
|---|---:|
| Sampling interval | 1 second |
| Experiments 1 and 2 | 5 minutes per run |
| Experiment 3 | 1 hour per run |
| Runs per experiment | 9 |

## Safety

This project controls electrical sources, capacitors, relays, converters, and loads. Operate it only with suitable protection and supervision.

- Verify relay wiring before applying power.
- Confirm that all relays start in the OFF state.
- Test capacitor discharge paths.
- Use suitable fuses and current limits.
- Use wires and connectors rated for the expected current.
- Test each relay with the power source disconnected.
- Keep an emergency disconnect accessible.
- Never connect an unknown power source directly to the circuit.
- Stop operation immediately if sensors, relays, or measurements behave unexpectedly.

## Credential Security

Never commit the following to GitHub:

- Wi-Fi passwords
- Telegram bot tokens
- Telegram chat IDs
- API keys
- Other private credentials

If credentials have ever been published, rotate them immediately. For production use, store credentials in a local ignored configuration file or a secure secrets-management system.

## Project Status

C.O.D.E. is an experimental and educational platform for renewable-energy power-system measurements and controller development.

Sensor calibration, relay polarity, electrical limits, and measurement accuracy must be validated for each physical setup before collecting research data.

## License

No license has currently been specified. Until a license is added, reuse and redistribution remain subject to the default copyright rules.
