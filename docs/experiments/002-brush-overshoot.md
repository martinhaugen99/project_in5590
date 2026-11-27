# 002: Brush stop at the edge (optimization task)

- **Status:** 50, 60 and 70 % measured (the runs of 001 Part B, 2026-11-27). 80, 90 and 100 % planned (D033).
- **Decision:** D033 (target and duty range), D025 (the optimization task). README test ladder, step 4
- **Run by:** Martin
- **Date:** 2026-11-27
- **Git commit and dirty flag** (`git rev-parse --short HEAD`, `git status --short`): not recorded for the runs from 001. The firmware was built from uncommitted changes on top of 805a667.
- **Firmware build:** the full build, `env:leonardo`. From 80 %, the build with `TRACK_MAX_DUTY_PCT = 100` (D033)
- **Table surface and lighting:** one lab table, the same as 001. The demo uses a similar table.

## Question

Which approach duty stops the brush 16–18 mm before the table edge, so that the full 20 mm push of the actuator moves it 2–4 mm past the edge, while both tracks keep a gap to the edge?

## Setup

- **Robot:** the same as experiment 001.
  - Arms removed, arm servo unplugged (D032).
  - The brush down (`servo brush 1500`), and the actuator fully retracted.
  - The 12 V adapter connected, also when you flash (D029).
- **Firmware:** flash the build with the speed cap at 100 % (D033). The edge gate still stops each track when its sensor stops seeing the table.
- **Controller:** `approach [duty]` (D023). Each track stops for good when its sensor stops seeing the table, so the robot squares up on its own.
- **Tuned:** the approach duty.
  - 50, 60 and 70 % are done (from 001).
  - Next: 80, 90 and 100 %, in that order, then finer steps around the target.
- **Safety:** keep the catch below the edge for the faster runs, and a hand at the cut-off switch. `stop` ends a run at once.
- **Geometry:** the brush sits about 49 mm ahead of the track fronts (001, Part B: gap plus brush value). At the target, the track gaps are about 66 mm.

## Procedure

1. Tape the start mark about 20 cm from the edge, the same as in 001. Put the robot on it, square to the edge.
2. `en 1`, then `approach <duty>`. Wait for the `result` line.
3. Measure with a ruler:
   - how far the brush stands from the edge: negative before the edge, positive past it
   - the gap from the front of each track to the edge
4. Copy the `result` line, and any `tof ... fault` line, into the raw data.
5. **Coarse sweep:** five runs each at 80 %, then 90 %, then 100 %.
6. **Stop rule (D033):** if any track gap in a run is below 30 mm, do not try a higher duty.
7. **Fine steps:** find the two duties whose mean brush values lie on either side of −17 mm, for example 70 % (−23 mm) and 80 %. Test the duties between them in steps of 2 % (chosen), five runs each. Stop once a duty has all five runs inside the band.
8. After the last run, note the `tof` line (error counts).

## Measured quantities

- Brush position relative to the edge in mm: negative before the edge, positive past it.
- Gap from each track front to the edge in mm.
- `t_total_ms` from the `result` line: the time from the start of the tracks until both have stopped.

## Choosing the best duty (D033)

- **Target band:** the brush stops between −18 and −16 mm (16–18 mm before the edge).
- **Constraint:** both gaps above 0 in every run at that duty (D025).
- **Best:** the duty with the most runs inside the band.
- **Tie:** the duty whose mean is closest to −17 mm.

## Baseline (D025)

Three runs on 2026-11-22 at full duty, with the old edge test sketch:
- The brush stood 30–40 mm past the edge.
- The track gaps were 20–30 mm.

That sketch stopped both tracks together when either sensor saw the edge, and it was never committed. It also used the old sensor mounts. It is a different controller, so compare against it only with that note.

## Reality gap (D022)

The CAD model puts each sensor point 92.44 mm ahead of the front of its track, with the sensor mounts of 2026-11-27 (it was 63.25 mm with the old mounts, D023). On the real robot:
- **Sensor cone:** each sensor sees a cone, not a line, so the stop can come later than the point straight below the sensor.
- **Brush-arm mount:** it is off center, so the two sensor points are not the same distance ahead of the tracks.
- **Track slip:** the tracks slip, so the turn point moves when one track stops first.

## Raw data

Save the `result` lines and the ruler values as CSV files in `data/002-brush-overshoot/`, named `<date>_<duty>.csv`, with the same columns as `data/001-edge-calibration/2026-11-27_B-approach.csv`.

**From experiment 001 Part B** (2026-11-27): the same table, start mark and setup, with a catch below the edge. Run by run in the 001 report.

| Runs | Duty (%) | Outcomes | First side | `t_total_ms` | Brush (mm) | Gaps (mm) |
|---|---|---|---|---|---|---|
| 1–5 | 50 | 5 × done | R, R, B, L, B | 2026–2668 | −39 to −35 | 80–91 |
| 6–10 | 60 | 5 × done | R, B, L, B, R | 1337–1729 | −35 to −28 | 78–87 |
| 11–15 | 70 | 5 × done | R, R, R, R, R | 1186–1287 | −24 to −22 | 70–75 |

**New runs (D033):**

| Run | Duty (%) | Outcome | First side | `t_total_ms` | Gap left (mm) | Gap right (mm) | Brush (mm) | Fault lines |
|---|---|---|---|---|---|---|---|---|
| | 80 | | | | | | | |

## Result

| Duty (%) | Runs | Brush min / mean / max (mm) | Runs in −18 to −16 | Smallest gap (mm) | Mean `t_total_ms` | Passes the constraint |
|---|---|---|---|---|---|---|
| 50 | 5 | −39 / −37.0 / −35 | 0 | 80 | 2353 | yes |
| 60 | 5 | −35 / −32.0 / −28 | 0 | 78 | 1473 | yes |
| 70 | 5 | −24 / −23.0 / −22 | 0 | 70 | 1227 | yes |
| 80 | | | | | | |
| 90 | | | | | | |
| 100 | | | | | | |

**So far:** the brush stopped before the edge in every run, 22–39 mm short, because the new mounts make the sensors look about 40 mm ahead of the brush (001, Part B findings). 70 % comes closest, 5–8 mm short of the band.

**Chosen duty:**
