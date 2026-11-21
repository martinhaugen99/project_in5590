---
description: Draw or update the M.A.T.S wiring diagrams with WireViz, showing every physical wire from pin to pin. Use when asked for a wiring diagram or schematic, or when pins, power rails or connected parts change.
allowed-tools: Bash(uv run wireviz *)
---

# Wiring diagrams with WireViz

WireViz turns a YAML description of connectors and wires into a drawing where every wire
is a colored line from one pin to another. WireViz 0.4.1 is a dev dependency in `uv.lock`.
It needs Graphviz (`dot`) on the PATH, installed with `brew install graphviz` on macOS.

## Files

Source files live in `electronics/wiring/`, one per sheet:

- `01_signals.yml`: Nano ESP32 to edge sensors, display, servo signals, IMU, buttons and encoders.
- `02_drivers.yml`: Nano ESP32 to both L293D chips, track motors, linear actuator and pull-down resistors.
- `03_power.yml`: 12 V wall adapter, cut-off switch, LM2596, BEC12S-PRO, laptop USB, and every power and ground wire.

Render each sheet to SVG next to its source, and commit the `.yml` and the `.svg`:

```bash
uv run wireviz -f s electronics/wiring/<sheet>.yml
```

## Sources of truth (read before editing)

- The sheets are the source of truth for pins and wiring (D015). `firmware/include/pins.h`
  must match them. If a sheet and `pins.h` disagree, stop and ask Martin. Do not change
  either file to make them agree.
- Nano ESP32 header pins: ABX00083 datasheet, section 12 (JP1-1 to JP1-15, JP2-1 to JP2-15).
  Use the header pin as `pins` and the Arduino label (D0, A4/SDA, 3V3) as `pinlabels`.
- Power: D005 and D014 in `docs/decisions.md`.
- Parts, datasheets and pin numbers of each part: `bom.csv` and the linked datasheets. Cite the
  source in the connector's `notes`.
- Net names: keep the names from `electronics/schematics.pdf` (for example `MOTOR_ENABLE`,
  `+5V_SERVO`, `+12V_UNSWITCHED`), so the old and the new drawings can be compared.

## Rules

- Draw every physical wire. Each wire belongs to a cable or a bundle (`category: bundle`)
  between two connectors, and carries its net name in `wirelabels`.
- Designators: U = board or chip, J = connector or lead set, R = resistor, SW = switch,
  PS = power supply or converter. Keep U1–U3 and J4–J13 from `schematics.pdf`.
  Taken so far: U4 = OpenLog Artemis, R1/R2 = MOTOR_ENABLE network, R3 = TFT RESET pull-up,
  R4–R9 = driver input pull-downs (one block, `R4_R9`), C1–C4 = L293D bypass capacitors,
  PS1 = 12 V wall adapter, PS2 = LM2596, PS3 = BEC12S-PRO, SW1 = cut-off switch.
- Quote YAML list items that contain a comma, such as `"1,2EN"`. Unquoted, the comma splits
  the item in two and the pin labels shift.
- A net that reaches several parts goes through a bus node: a connector with `style: simple`
  named after the net (`BUS_3V3`, `BUS_GND`). A rail fed on another sheet is a `RAIL_*`
  node whose `notes` name that sheet.
- Layout: rails and buses on the left, the Nano in the middle, the parts on the right. To get
  this, connection sets start at the bus or rail, also for the bus feed into the Nano.
- One bundle per part. Its power wires come from the buses, its signal wires from the Nano,
  in separate connection sets that use the same cable.
- Keep `notes` short and break them into lines with a YAML `|` block. Long lines make the
  boxes, and the whole drawing, very wide.
- Factory leads keep their real colors: encoder brown +V, blue 0V, black A, white B, orange Z
  (J12/J13); servo brown GND, red V+ (J10/J11).
- Pins that are not connected stay visible, with a note such as "NC, insulate".
- Never invent physical values (wire gauge, length, switch rating, currents). Leave them out
  and list them as open questions for Martin.
- Hardware actions (powering, flashing) are out of scope for this skill.

## Color convention for hookup wires (drawing convention, proposed 2026-11-20)

Use these unless the real build uses other colors. Then draw the real colors.

| Net | Code |
|---|---|
| GND | BK |
| +12 V (unswitched, switched, actuator) | RD |
| VM_TRACK (LM2596 output) | PK |
| +5V_SERVO (BEC output) | OG |
| USB_VBUS | VT |
| +3V3_LOGIC | RDWH (red with white stripe) |
| I2C SDA / SCL | BU / YE |
| MOTOR_ENABLE | WH |
| Driver inputs LEFT_IN1, LEFT_IN2, RIGHT_IN1, RIGHT_IN2, ACT_IN1, ACT_IN2 | GN, GY, BN, TQ, LB, OL |
| Driver outputs to a motor or the actuator (first / second lead) | BUWH / BNWH |
| ACT_FEEDBACK (actuator pot wiper) | GNWH |
| Other signals | GN, GY, WH, BN, TQ, LB, OL; distinct within one bundle |

## Workflow

1. Read the sources above for the parts on the sheet.
2. Edit the sheet's `.yml`.
3. Render the SVG with the command above.
4. Check it visually: render a PNG to the scratchpad
   (`uv run wireviz -f p -o <scratchpad> electronics/wiring/<sheet>.yml`) and look at it.
   Do not commit PNG files.
5. Check that every pin in `pins.h` appears on a sheet, and that every Nano pin on a sheet is in `pins.h`.
6. Report the open questions and the files changed.

## Minimal example

```yaml
connectors:
  U1:
    type: Arduino Nano ESP32
    pinlabels: [3V3, GND, A4/SDA, A5/SCL, D4]
    pins: [JP1-2, JP1-14, JP1-8, JP1-9, JP2-9]
  J7:
    type: SATEL-VL53L4CD (left)
    pinlabels: [GND, VDD, SCL, SDA, GPIO1, XSHUT]
    pins: [1, 2, 3, 4, 5, 6]
    notes: GPIO1 not connected

cables:
  W1:
    category: bundle
    colors: [RDWH, BK, BU, YE, GN]
    wirelabels: [+3V3_LOGIC, GND, SDA, SCL, TOF_L_XSHUT]

connections:
  -
    - U1: [JP1-2, JP1-14, JP1-8, JP1-9, JP2-9]
    - W1: [1-5]
    - J7: [2, 1, 4, 3, 6]
```
