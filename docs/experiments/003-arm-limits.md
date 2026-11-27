# 003: Arm limits

- **Status:** Cancelled (D032). The arm servo buzzes at every pulse, also without the arms, so the arms are removed for the demo. The procedure below stays for later work on the arms.
- **Decision:** D030 (the arms move for the animations, without the bucket)
- **Run by:** Martin
- **Date:**
- **Git commit and dirty flag** (`git rev-parse --short HEAD`, `git status --short`):
- **Firmware build:** the full build, `env:leonardo`, step 3 (D031)
- **Table surface:**

## Question

Which servo pulses can the arms use safely at any time? "Safely" means the arms never touch the robot or the table, never change an edge-sensor reading, and never make the robot rock or overload the BEC.

## Setup

- **Robot:**
  - Fully wired, with the bucket off the arms.
  - The brush is down with its servo off, and the actuator is fully retracted.
  - The 12 V adapter is connected, so the edge sensors have power while the bootloader runs (D029).
- **Firmware:** step 3, flashed with `uv run pio run -d firmware -t upload` (D031).
  - `servo bucket` moves the arms whenever both tracks are stopped, also on the table.
  - The tracks are locked while the arm servo gets pulses.
- **Console:** `uv run pio device monitor -d firmware`. Send `en 0` first, so the tracks cannot move during any step.
- **Safety:**
  - Keep a hand at the cut-off switch. It also stops the servos (D030).
  - Keep fingers away from the gears and the arm hubs.
  - The first pulse moves the arms at full servo speed to the pose of that pulse.
- **Units:**
  - One degree of arm is about 4.2 µs, if the servo is linear (D030). Check this on the robot.
  - The servo accepts 750–2250 µs (D028).

## Procedure

Find out first which direction raises the arms. Send `servo bucket 1500`, then `servo bucket 1520`, and watch which way they move.

### L1: Upper end

1. Robot on the table.
2. From 1500 µs, step toward the head in 10 µs steps.
3. Stop at the first contact with the head, the body or the lid, or when the servo starts to buzz. Record the pulse.
4. **L1** = that pulse, backed off by a margin. Proposal: 40 µs, about 10° of arm.
5. Hold L1 for 10 s.

**Pass:** no contact and no buzz.

### L2: Lower end

1. Robot on a flat part of the table.
2. From 1500 µs, step toward the table in 10 µs steps.
3. Stop at the first contact of the arms or their tips with the table, the brush arm, the actuator or the tracks. Record the pulse.
4. **L2** = that pulse, backed off by the same margin.
5. Measure the gap between the arm tips and the table at L2 with a ruler.

**Pass:** a clear gap at L2.

### L3: Sensor cones

An arm in the cone of an edge sensor can make a sensor past the edge read as table, and the edge gate would then let that track drive (D030). This is the most important limit.

1. **On the table:**
   1. `servo bucket` at REST (chosen by eye), then `stream on`. Note `tof_l_mm` and `tof_r_mm` over about 2 s to see the noise.
   2. Sweep from L2 to L1 in 20 µs steps, and wait about 1 s at each step.
   3. Record `tof_l_mm`, `tof_r_mm`, `table_l` and `table_r` at each step.
2. **At the edge:**
   1. Place the robot by hand with both sensors past the edge and the tracks fully on the table. `tof` must show `NO TABLE` on both sides.
   2. Have a soft catch below the edge.
   3. Repeat the sweep.
3. **A failing pulse:** the allowed range ends one margin (40 µs) before the first failing pulse. Move L1 or L2 inward to match.

**Pass:**
- On the table: `table_l` and `table_r` stay 1, and the distances stay within the noise.
- At the edge: `table_l` and `table_r` stay 0, and both distances stay above 100 mm (the value past the edge, D023).

### L4: Pose without pulses

1. For each of UP (L1), FORWARD and LOW (L2): send `servo bucket <pulse>`, wait 1 s, then `servo bucket off`.
2. Note where the arms settle. This is also their pose at boot, before the first pulse.
3. With the arms in that pose, place the robot with both sensors past the edge. Check that `tof` shows `NO TABLE` on both sides.

**Pass:** the arms settle off the table and outside the cones. If they do not, the firmware must keep a pulse on the arm servo at all times, from the first tick after boot (D030, firmware next steps).

### L5: Speed

`servo bucket` jumps at full servo speed, which is the worst case.

1. Robot on the table, `stream on`.
2. Jump between L2 and L1 five times, about 1 s apart.
3. Then repeat at the edge: sensors past the edge, a soft catch below, and a hand ready to hold the robot.

**Pass:**
- The robot does not slide or rock.
- The tracks stay on the surface.
- `table_l` and `table_r` do not change (1 on the table, 0 at the edge).

**If the full jump fails:** find the largest jump that passes, in steps of 100 µs. The firmware then ramps the arms, and the ramp is tested again after the firmware step.

### L6: Current

1. Power off.
2. Put a multimeter (A range) in series with the +5V_SERVO wire of J11 (W7 wire 2, sheet 01).
3. Read the current:
   1. Holding at REST.
   2. During the jump from L2 to L1. Note the highest reading.
   3. Pressing against the upper stop: send a pulse one margin past L1, for at most 2 s.

**Pass:** every reading is at most 2.68 A (D028, BEC budget).

## Measured quantities

- Servo pulses in µs.
- `tof_l_mm` and `tof_r_mm` in mm, with `table_l` and `table_r`.
- Gap between the arm tips and the table in mm.
- Servo current in A.

## Raw data

Copy the stream lines of L3 and L5 from the console into CSV files in `data/003-arm-limits/`, one file per step, named `<date>_<step>.csv`.

| Step | Pulse (µs) | `tof_l_mm` | `tof_r_mm` | `table_l` | `table_r` | Note |
|---|---|---|---|---|---|---|
| | | | | | | |

## Result

Each value becomes a named constant in `firmware/include/config.h` in the firmware step of D030, with this experiment as its source.

| Limit or pose | Value | Proposed constant | Pass |
|---|---|---|---|
| Direction: a higher pulse moves the arms | up / down | – | – |
| L1, upper end (UP) | µs | `ARM_UP_US` | |
| HIGH | µs | `ARM_HIGH_US` | – |
| FORWARD (hold pose) | µs | `ARM_FORWARD_US` | |
| REST | µs | `ARM_REST_US` | – |
| L2, lower end (LOW) | µs | `ARM_LOW_US` | |
| Gap at the tips at L2 | mm | – | |
| L3, sensor cones, on the table | range in µs | – | |
| L3, sensor cones, at the edge | range in µs | – | |
| L4, pose without pulses | where the arms settle | – | |
| L5, largest jump that passes | µs | `ARM_MAX_JUMP_US` (only if needed) | |
| L6, current holding / moving / at the stop | A | – | |
| Arm degrees per µs (check of the 4.2 µs per degree) | | – | – |
