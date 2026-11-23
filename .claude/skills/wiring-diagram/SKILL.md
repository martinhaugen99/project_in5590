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

- `01_signals.yml`: Arduino Leonardo to edge sensors, display (through dividers), servo signals, buttons and encoders.
- `02_drivers.yml`: Arduino Leonardo to both L293D chips, track motors, linear actuator and pull-down resistors.
- `03_power.yml`: 12 V wall adapter, cut-off switch, LM2596, BEC12S-PRO, laptop USB, and every power and ground wire.

Render each sheet to SVG next to its source, and commit the `.yml` and the `.svg`:

```bash
uv run wireviz -f s electronics/wiring/<sheet>.yml
```

## Sources of truth (read before editing)

- The sheets are the source of truth for pins and wiring (D015). `firmware/include/pins.h`
  must match them. If a sheet and `pins.h` disagree, stop and ask Martin. Do not change
  either file to make them agree.
- Arduino Leonardo pins: [A000057 full pinout](https://docs.arduino.cc/resources/pinouts/A000057-full-pinout.pdf) (D020).
  Use the header label as `pins` (D0–D13, A0–A5, 3V3, 5V, GND, ICSP-1 to ICSP-6) and add the
  function in `pinlabels` (for example `D2 SDA`, `ICSP-4 COPI`).
- Power: D005, D014 and D021 in `docs/decisions.md`.
- Parts, datasheets and pin numbers of each part: `bom.csv` and the linked datasheets. Cite the
  source in the connector's `notes`.
- Net names: keep the names from `electronics/schematics.pdf` (for example `MOTOR_ENABLE`,
  `+5V_SERVO`, `+12V_UNSWITCHED`), so the old and the new drawings can be compared.

## Rules

- Draw every physical wire. Each wire belongs to a cable or a bundle (`category: bundle`)
  between two connectors, and carries its net name in `wirelabels`.
- Designators: U = board or chip, J = connector or lead set, R = resistor, SW = switch,
  PS = power supply or converter. Keep U1–U3 and J4–J13 from `schematics.pdf`.
  Taken so far: U4 = OpenLog Artemis (dropped in D020, do not reuse), R1/R2 = MOTOR_ENABLE
  network, R3 = TFT RESET pull-up, R4–R9 = driver input pull-downs (one block, `R4_R9`),
  R10–R17 = TFT signal dividers (one block, `R10_R17`), C1–C4 = L293D bypass capacitors,
  PS1 = 12 V wall adapter, PS2 = LM2596 (3.3 V, D021), PS3 = BEC12S-PRO, SW1 = cut-off switch.
- Quote YAML list items that contain a comma, such as `"1,2EN"`. Unquoted, the comma splits
  the item in two and the pin labels shift.
- A net that reaches several parts goes through a bus node: a connector with `style: simple`
  named after the net (`BUS_3V3`, `BUS_GND`). A rail fed on another sheet is a `RAIL_*`
  node whose `notes` name that sheet.
- Layout: rails and buses on the left, the Leonardo in the middle, the parts on the right. To
  get this, connection sets start at the bus or rail, also for the bus feeds into the Leonardo.
- One bundle per part. Its power wires come from the buses, its signal wires from the Leonardo,
  in separate connection sets that use the same cable.
- Keep `notes` short and break them into lines with a YAML `|` block. Long lines make the
  boxes, and the whole drawing, very wide.
- Factory leads keep their real colors: encoder brown +V, blue 0V, black A, white B, orange Z
  (J12/J13); servo brown GND, red V+ (J10/J11).
- Pins that are not connected stay visible, with a note such as "NC, insulate".
- Never invent physical values (wire gauge, length, switch rating, currents). Leave them out
  and list them as open questions for Martin.
- The Leonardo has 5 V logic. A wire from a Leonardo output to a 3.3 V input goes through a
  divider or is driven open-drain, and the part's `notes` say which (D020). The two 3.3 V
  nets, +3V3_LOGIC (Leonardo 3V3 pin) and +3V3_TOF (LM2596), are never joined.
- Hardware actions (powering, flashing) are out of scope for this skill.

## Color convention for hookup wires (drawing convention, proposed 2026-11-20)

Use these unless the real build uses other colors. Then draw the real colors.

| Net | Code |
|---|---|
| GND | BK |
| +12 V (unswitched, switched, actuator) | RD |
| +3V3_TOF (LM2596 output, edge sensors) | PK |
| +5V_SERVO (BEC output) | OG |
| +5V_USB (Leonardo 5V pin) | VT |
| +3V3_LOGIC | RDWH (red with white stripe) |
| I2C SDA / SCL | BU / YE |
| MOTOR_ENABLE | WH |
| Driver inputs LEFT_IN1, LEFT_IN2, RIGHT_IN1, RIGHT_IN2, ACT_IN1, ACT_IN2 | GN, GY, BN, TQ, LB, OL |
| Driver outputs to a motor or the actuator (first / second lead) | BUWH / BNWH |
| ACT_FEEDBACK (actuator pot wiper) | GNWH |
| TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK (same color on both sides of a divider) | GN, WH, BN, TQ |
| Other signals | GN, GY, WH, BN, TQ, LB, OL; distinct within one bundle |

## Workflow

1. Read the sources above for the parts on the sheet.
2. Edit the sheet's `.yml`.
3. Render the SVG with the command above.
4. Check it visually: render a PNG to the scratchpad
   (`uv run wireviz -f p -o <scratchpad> electronics/wiring/<sheet>.yml`) and look at it.
   Do not commit PNG files.
5. Check that every pin in `pins.h` appears on a sheet, and that every Leonardo pin on a sheet is in `pins.h`.
6. Report the open questions and the files changed.

## Minimal example

```yaml
connectors:
  U1:
    type: Arduino Leonardo
    pins: [D2, D3, D5]
    pinlabels: [D2 SDA, D3 SCL, D5]
  J7:
    type: SATEL-VL53L4CD (left)
    pinlabels: [GND, VDD, SCL, SDA, GPIO1, XSHUT]
    pins: [1, 2, 3, 4, 5, 6]
    notes: GPIO1 not connected

cables:
  W1:
    category: bundle
    colors: [BU, YE, GN]
    wirelabels: [SDA, SCL, TOF_L_XSHUT]

connections:
  -
    - U1: [D2, D3, D5]
    - W1: [1-3]
    - J7: [4, 3, 6]
```
