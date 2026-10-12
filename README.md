# SAMD21 F´ Demo

> [!WARNING]
> Initial work in progress. It builds, but hasn't been tested on hardware.

A lightweight [F´ (F Prime)](https://fprime.jpl.nasa.gov) deployment for the Microchip ATSAMD21J17D, a 48 MHz
Cortex-M0+ with 128 KB of flash and 16 KB of SRAM. It starts from the
[Arduino deployment cookiecutter](https://github.com/fprime-community/fprime-arduino-deployment-cookiecutter), which
doesn't fit (it needs about 140 KB of flash and 24 KB of RAM), and replaces its communications stack and active
components with the smaller ones from [fprime-atmega128-demo](https://github.com/nathancheek/fprime-atmega128-demo).
What's left is a blinking LED, a Space Packet link and reset handling, with about half of the SRAM free for more
features.

## A lighter CCSDS communications stack

The Space Packet link (F´ GDS on the bench, another subsystem in flight) carries CCSDS Space Packets, each starting with
a 2-byte sync word (0xC1F5) and ending in a CRC-16. Sync word, CRC, and APIDs are all configurable in
`config/ComCfg.fpp`. The comms stack skips TC/TM transfer frames, since those are built for space-to-ground links and
add unnecessary complexity.

* `ComSpacePacket` replaces the cookiecutter's `ComCcsds` subtopology, with no buffer manager or com queue.
* There are no active components: everything runs from the rate groups in `loop()`, using `LiteCmdDispatcher` and
  `PassiveTlmPacketizer`.
* There are no events. Command results show up in the `cmdDisp.CommandsDispatched` and `cmdDisp.CommandErrors`
  channels.
* All telemetry is sent once per second via packetized telemetry packets.
* F´ GDS reads the link through the framing plugin in `gds/space_packet_crc.py`.

## Other features

The components are in `SAMD21Demo/Components`.

* **Reset reason, asserts and watchdog.** `SAMD21Reset` reports why the MCU last reset, prints asserts on the console
  before resetting, reboots on command and runs a watchdog. The watchdog also recovers from hard faults, which the
  core's fault handler stops on.
* **Heartbeat LED.** fprime-arduino's `LifeLed` blinks the built-in LED. Its `LED_PERIOD` parameter sets the period.
* **Parameter defaults.** `DefaultPrmDb` stands in for a parameter database until there's storage for one (the J17D
  has 4 KB of read-while-write EEPROM for this). Components start with their parameter defaults, `PRM_SET` changes a
  value until the next reset, and `PRM_SAVE` fails.

## Memory

| | Bytes | |
|---|---:|---|
| Flash | 44,720 | 34% of 128 KB |
| Static RAM (data, bss and noinit) | 3,232 | 20% of 16 KB |
| Stack, reserved in `Main.cpp` | 2,048 | |
| Free RAM | 11,104 | 68% |

| Static RAM | Bytes |
|---|---:|
| F´ component instances | 1,557 |
| F´ OS layer | 629 |
| Arduino UARTs: Serial1, Serial2 and Serial3, each with 64-byte receive and transmit buffers | 564 |
| Arduino `Wire`, unused but linked in by fprime-arduino's I2C drivers | 180 |
| Other | 302 |

`config/PlatformCfg.fpp` cuts the F´ OS handles to the size of the implementations used here, which saves about
500 bytes. The linker fails if static RAM leaves less than the reserved stack. Nothing reserves heap: anything that
calls `malloc` takes from the free RAM, as does a stack deeper than 2 KB.

### Adding more

Measured by adding a probe component to this deployment in different shapes. Most of the cost is flash, for the
code F´ generates per command, channel and parameter:

| Adding | Flash | RAM |
|---|---:|---:|
| A passive component with one rate group port | 264 | 16 |
| Another instance of it | 96 | 16 |
| Commands, with the first command (no arguments) | 416 | 0 |
| Each further command | 45 | 0 |
| Each command argument (U32) | 55-63 | 0 |
| Telemetry, with the first U32 channel | 376 | 0 |
| Each further U32 channel | 115 | 0 |
| Each channel in a telemetry packet | 10 | 0 |
| Parameters, with the first U32 parameter (includes its PRM_SET and PRM_SAVE commands and a lock) | 1,244 | 48 |
| Each further U32 parameter | 486 | 12 |
| A component like `SAMD21Reset`: 4 commands with an argument, 5 channels, 1 parameter | 2,920 | 64 |
| Another instance of it | 496 | 56 |

Some limits are fixed in RAM whether they're used or not:

* `LiteCmdDispatcher` has 24 opcode slots (`CMD_DISPATCHER_DISPATCH_TABLE_SIZE`), 8 bytes each. 12 are used, and each
  parameter takes 2.
* `PassiveTlmPacketizer` keeps 103 bytes of channel values for each of `MAX_PACKETIZER_PACKETS` (2) packets. The one
  packet used has 76 bytes of channels.

## Hardware

The deployment builds the [Fab SAM core](https://github.com/qbolsee/ArduinoCore-fab-sam)'s "Generic x21J" board, a
generic 64-pin SAMD21J with MattairTech's Xeno pin map, set up by fprime-arduino's `ATSAMD21J17D` toolchain:

* ATSAMD21J17D, flashed over SWD (no bootloader)
* 32.768 kHz crystal on PA00/PA01 (the toolchain describes how to run without one)
* Built-in LED on PB02
* A serial adapter on Serial3 (SERCOM4, TX PB08, RX PB09) for console output, and another on Serial2 (SERCOM0, TX PA06,
  RX PA07) for the Space Packet link. Serial1 isn't used, since its RX pin, PA31, is also SWDIO.

The UARTs have 64-byte receive buffers and the link is read at 10 Hz, so a command longer than 64 bytes (with sync
word, header and CRC) can lose bytes at 115200 baud. The commands in this deployment are all shorter.

## Getting started

Clone the project with its submodules, then set up a virtual environment:

```sh
git clone --recurse-submodules https://github.com/nathancheek/fprime-samd21-demo.git
cd fprime-samd21-demo
python3 -m venv fprime-venv
. fprime-venv/bin/activate
pip install -r requirements.txt
```

Then follow the [arduino-cli installation guide](https://github.com/fprime-community/fprime-arduino/blob/main/docs/arduino-cli-install.md)
and install the Fab SAM core:

```sh
arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/qbolsee/ArduinoCore-fab-sam/master/json/package_Fab_SAM_index.json
arduino-cli core update-index
arduino-cli core install Fab_SAM_Arduino:samd@1.12.0
```

Building, flashing and running F´ GDS are covered in the
[deployment's README](SAMD21Demo/SAMD21DemoDeployment/README.md).
