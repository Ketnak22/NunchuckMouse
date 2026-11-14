# NunchuckMouse

Wii Nunchuck jako mysz USB. Drążek rusza kursorem, Z to lewy klik, C to prawy. Firmware w Arduino na RP2040-Zero, osadzonym na własnej płytce z gniazdem Wii, zaprojektowanej w KiCadzie. Działający prototyp, zbudowany i używany przeze mnie.

---

A Wii Nunchuck used as a USB mouse. An RP2040-Zero reads the Nunchuck over I2C and shows up on the PC as a regular HID mouse, so no driver is needed. The module sits on a small PCB I designed in KiCad, with a Wii extension socket.

Status: working prototype, built and used by me.

![The board with a Nunchuck plugged in](docs/board.jpg)

## Controls

| Nunchuck | Mouse |
|---|---|
| stick | move the cursor |
| Z | left click |
| C | right click |

The Nunchuck can be unplugged and plugged back in while the mouse is running. Unplugging it releases any held button.

The RGB LED on the module shows the state:

- red: no Nunchuck
- green: Nunchuck connected and working

## Board

A two-layer, 33 × 42.5 mm board with:

- an RP2040-Zero module,
- a 6-pin Wii extension socket,
- four 2.5 mm mounting holes.

There are no other parts. The Nunchuck is powered from the module's 3.3 V regulator.

| Socket pin | Signal | RP2040 |
|---|---|---|
| 1 | 3.3 V | 3V3 |
| 2 | SCL | GP1 |
| 3 | DET | GP11 |
| 4 | not connected | |
| 5 | SDA | GP0 |
| 6 | GND | GND |

The LED is the WS2812 on the module, on GP16.

The KiCad project, its libraries and the Gerbers sent for manufacturing (11.12.2024) are in [`hardware/`](hardware/). The board was built from these files, so I haven't changed the design since. The only change is the removal of a duplicate entry in `sym-lib-table`.

| Top | Bottom |
|---|---|
| ![Top](docs/board-top.png) | ![Bottom](docs/board-bottom.png) |

Schematic: [`docs/schematic.svg`](docs/schematic.svg)

### Known issues in the design files

DRC in KiCad 10.0.8 reports 10 errors. All of them come from the RP2040-Zero footprint, which draws a U-shaped cutout under the module on the Edge.Cuts layer:

- the cutout is open at one end, so KiCad reports the board outline as not closed (2 errors),
- the bottom row of module pads is 0.37–0.42 mm from that cutout, below the 0.5 mm edge clearance rule (8 errors).

The board's own outline is closed. The cutout lines are in the Gerbers too, but the manufacturer didn't cut anything there: the finished board is solid under the module. The remaining warnings are footprints that differ from their library copies, and silkscreen clipped by the socket's mounting pads. In the schematic, ERC reports unused module pins without no-connect flags and a short dangling wire next to GP1. None of this affects the connections.

### Pins

The first version of the firmware set I2C to GP4/GP5, which don't reach the socket on this board. An I2C scan on the assembled board found the Nunchuck on GP0/GP1 and confirmed DET on GP11, the same as in the schematic and PCB. So the design files were right and the firmware was fixed.

The board uses the RP2040-Zero footprint from [`hardware/RP2040-Zero-KiCAD-main/`](hardware/RP2040-Zero-KiCAD-main/). Its author marks it as untested, but the pins this board needs are in the right places.

## Building and flashing

You need [PlatformIO](https://platformio.org/). The platform and library versions are pinned in `platformio.ini`.

```
pio run
```

To flash, hold the BOOT button on the module while plugging in USB. A drive called `RPI-RP2` appears. Then either run

```
pio run -t upload
```

or copy `.pio/build/rp2040_zero/firmware.uf2` to that drive.

## Warning

The Nunchuck runs on 3.3 V only. Don't connect its power pin to 5 V, it will damage it.

## Related

[PicoNunchuck](https://github.com/Ketnak22/PicoNunchuck) reads a Nunchuck with the Pico SDK in C++. This project turns it into a USB mouse in Arduino, on its own board.

## License

The firmware and my board design are under the MIT license, see [`LICENSE`](LICENSE).

Third-party parts in `hardware/` keep their own licenses:

- `RP2040-Zero-KiCAD-main/` (RP2040-Zero symbol and footprint) is under the CERN Open Hardware Licence v2 – Permissive (CERN-OHL-P), see its [`LICENSE`](hardware/RP2040-Zero-KiCAD-main/LICENSE).
- `WiiConnector/` contains a copy of the KiCad `Connector` symbol library and Wii socket footprints from a KiCad library merge request. They fall under the KiCad libraries license (CC-BY-SA 4.0 with an exception for designs that use them).
