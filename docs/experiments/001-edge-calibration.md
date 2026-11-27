# 001: Edge calibration

- **Status:** Done (2026-11-27). Part A run 3 and Part B pass.
- **Decision:** D023 (edge gate and edge approach). README test ladder, step 3
- **Run by:** Martin
- **Date:** 2026-11-27
- **Git commit and dirty flag** (`git rev-parse --short HEAD`, `git status --short`): not recorded. The firmware was built from uncommitted changes on top of 805a667.
- **Firmware build:** the full build, `env:leonardo`, step 3 (D031)
- **Table surface and lighting:** one lab table. The demo uses a similar table, not this one.

## Question

Does the edge gate tell the table from the edge reliably on the demo table? And does each track stop with a gap in front of it at 50, 60 and 70 % duty?

## Setup

- **Robot:**
  - The arms are removed, and the arm servo is unplugged from J11 (D032).
  - The brush is down, and the actuator is fully retracted.
  - Brush positions (measured 2026-11-27): `servo brush 1500` puts the brush on the table (down), and `servo brush 1700` lifts it off the table (up).
  - New sensor mounts (2026-11-27): they point further away from the brush and sit higher than on 2026-11-22.
  - The 12 V adapter is connected (D029).
- **Firmware:** step 3 (D031). The edge gate is always on (D023):
  - A sensor sees the table when its reading has status 0 and is at most 75 mm (`EDGE_TABLE_MAX_MM`).
  - The threshold is halfway between the values measured on 2026-11-22: about 50 mm on the table, and more than 100 mm past the edge. Those values came from the old sensor mounts, so Part A replaces them.
- **Console:** `uv run pio device monitor -d firmware`.
- **Safety:**
  - A soft catch below the edge, such as a box with a cushion.
  - A hand at the cut-off switch.
  - `stop` ends a run at once.
- **Geometry (CAD):** with the new sensor mounts, each sensor point is 92.44 mm ahead of the front of its track (Martin, 2026-11-27). With the old mounts it was 63.25 mm (D023).

## Procedure

### A. Readings and noise

1. **On the table:** robot still, both sensors over the table. `stream on` for about 10 s, then `stream off`.
2. **Past the edge:** place the robot by hand with both sensors past the edge and the tracks fully on the table. `stream on` for about 10 s, then `stream off`.
3. From each part, note the min and the max of `tof_l_mm` and `tof_r_mm`, and every status other than 0.
4. Repeat 1–3 with the brush up (`servo brush 1700`).
5. Repeat 1–4 on each surface you will use at the demo.

**Pass:**
- On the table: every reading has status 0 and is at most 75 mm.
- Past the edge: every reading is above 75 mm or has a status other than 0.
- Report the smallest margin to 75 mm on each side. A small margin means the threshold needs a change in `firmware/include/config.h`, with the measured values as its source.

### B. Gap at the edge

1. Tape a start mark about 20 cm from the edge. Put the robot on it, square to the edge.
2. `en 1`, then `approach 50`. Wait for the `result` line.
3. Measure with a ruler:
   - the gap from the front of the left track to the edge
   - the gap from the front of the right track to the edge
   - how far the brush stands past the edge (for experiment 002)
4. Copy the `result` line, and any `tof ... fault` line, into the raw data.
5. Put the robot back on the mark. Five runs in total.
6. Repeat 2–5 with `approach 60`, then with `approach 70`.
7. **Stop the sweep:** if any run ends with a gap of 0 or less, do not try a higher duty.

The `result` line is `result,<outcome>,<duty_pct>,<first_side>,<t_first_ms>,<t_total_ms>,<enc_l>,<enc_r>` (README).

**Pass:** at each duty, both gaps are above 0 in every run, and every outcome is `done`. The highest duty that passes is the top duty for experiment 002.

Going above 70 % needs a higher `TRACK_MAX_DUTY_PCT` and a decision entry (D023, D025). It is not part of this experiment.

## Measured quantities

- `tof_l_mm` and `tof_r_mm` in mm, with their range status.
- Gap from each track front to the edge in mm (ruler).
- Brush overshoot past the edge in mm (ruler).
- Times and encoder counts from the `result` line.

## Raw data

Save the stream lines of part A and the `result` lines of part B as CSV files in `data/001-edge-calibration/`, named `<date>_<part>.csv`.

### Part A

All on one lab table, 2026-11-27.

**Run 1: `stream on`, brush down, right sensor not fully in its mount.** Files: `2026-11-27_A1-table-brush-down_right-loose.csv` and `2026-11-27_A2-edge-brush-down_right-loose.csv`.

| Brush | Position | Lines | `tof_l_mm` min–max (mean) | `tof_r_mm` min–max (mean) | Status ≠ 0 |
|---|---|---|---|---|---|
| Down | On the table | 80 (7.9 s) | 64–72 (68.2) | 61–68 (65.0) | 0 |
| Down | Past the edge | 96 (9.5 s) | 853–871 (861) | 112–182 (130) | 0 |

**Run 2: mounts fixed, `tof` run many times.** Martin reported the spread of the repeated readings.

| Brush | Position | `tof_l_mm` | `tof_r_mm` | Status | Gate |
|---|---|---|---|---|---|
| Down (1500 µs) | On the table | 67 ± 3 | 64 ± 3 | 0 | table, both |
| Down (1500 µs) | Past the edge | 906 | 206, as low as 170 | 0 | NO TABLE, both |
| Up (1700 µs) | On the table | 103 ± 5 | 82 ± 5 | 0 | NO TABLE, both |
| Up (1700 µs) | Past the edge | 1072 | 48 | 0 | left NO TABLE, **right table** |

One `tof` line of each, as reported:

```
down, on table:  tof L 67 mm (status 0, 33 ms ago) table, err 847 code 2 rec 5 | R 64 mm (status 0, 5 ms ago) table, err 2215 code 2 rec 26
down, past edge: tof L 906 mm (status 0, 9 ms ago) NO TABLE, err 4 code 2 rec 1 | R 206 mm (status 0, 15 ms ago) NO TABLE, err 0 code 0 rec 0
up, on table:    tof L 103 mm (status 0, 32 ms ago) NO TABLE, err 847 code 2 rec 5 | R 82 mm (status 0, 44 ms ago) NO TABLE, err 2215 code 2 rec 26
up, past edge:   tof L 1072 mm (status 0, 18 ms ago) NO TABLE, err 4 code 2 rec 1 | R 48 mm (status 0, 15 ms ago) table, err 0 code 0 rec 0
```

At the end of run 1, `tof` showed L err 86, code 5, rec 2, and R err 1354, code 2, rec 21. The error counts in run 2 come from two different boots: 847 and 2215, or 4 and 0. Code 2 means that no device answered at the address (README, edge-sensor faults).

**Run 3: `stream on` for about 10 s each, mounts fixed, a spare sensor fitted** (the right sensor, judging by the change in its readings; to confirm). The pins are back to D1 for the right encoder and D5 for the right XSHUT. Files: `2026-11-27_A1-table-brush-down_spare-sensor.csv`, `..._A2-edge-brush-down_...`, `..._A3-table-brush-up_...` and `..._A4-edge-brush-up_...`.

| Brush | Position | Lines | `tof_l_mm` min–max (mean, sd) | `tof_r_mm` min–max (mean, sd) | Status ≠ 0 | Gate |
|---|---|---|---|---|---|---|
| Down (1500 µs) | On the table | 117 (11.6 s) | 63–67 (64.8, 1.0) | 60–67 (63.3, 1.2) | 0 | table, every line |
| Down (1500 µs) | Past the edge | 112 (11.1 s) | 874–887 (879.7, 2.3) | 597–647 (625.9, 9.8) | 0 | NO TABLE, every line |
| Up (1700 µs) | On the table | 105 (10.4 s) | 109–114 (111.3, 1.0) | 84–89 (86.3, 1.0) | 0 | NO TABLE, every line |
| Up (1700 µs) | Past the edge | 110 (10.9 s) | 1090–1109 (1099.6, 3.8) | 86–170 (123.8, 32.5) | left: 109 of 110 lines | NO TABLE, every line |

With the brush up past the edge, the right readings jump between two groups, about 86–103 mm and 141–170 mm.

### Part B

2026-11-27, brush down (`servo brush 1500`), spare sensor on the right, catch below the edge. File: `2026-11-27_B-approach.csv` (all fields of each `result` line, plus the ruler values). A negative brush value means the brush stopped before the edge.

| Run | Duty (%) | Outcome | First side | `t_total_ms` | Gap left (mm) | Gap right (mm) | Brush past edge (mm) | Fault lines |
|---|---|---|---|---|---|---|---|---|
| 1 | 50 | done | R | 2026 | 85 | 80 | −35 | not watched |
| 2 | 50 | done | R | 2227 | 90 | 87 | −37 | not watched |
| 3 | 50 | done | B | 2277 | 90 | 91 | −38 | not watched |
| 4 | 50 | done | L | 2668 | 90 | 90 | −39 | not watched |
| 5 | 50 | done | B | 2569 | 88 | 86 | −36 | not watched |
| 6 | 60 | done | R | 1337 | 80 | 78 | −28 | not watched |
| 7 | 60 | done | B | 1384 | 87 | 84 | −34 | not watched |
| 8 | 60 | done | L | 1434 | 85 | 84 | −34 | not watched |
| 9 | 60 | done | B | 1482 | 84 | 86 | −35 | not watched |
| 10 | 60 | done | R | 1729 | 80 | 80 | −29 | not watched |
| 11 | 70 | done | R | 1187 | 72 | 72 | −24 | not watched |
| 12 | 70 | done | R | 1237 | 72 | 75 | −23 | not watched |
| 13 | 70 | done | R | 1236 | 74 | 74 | −23 | not watched |
| 14 | 70 | done | R | 1287 | 72 | 72 | −22 | not watched |
| 15 | 70 | done | R | 1186 | 70 | 73 | −23 | not watched |

`tof` after all the runs: `tof L 663 mm (status 0, 42 ms ago) NO TABLE, err 10 code 5 rec 2 | R 304 mm (status 0, 37 ms ago) NO TABLE, err 57 code 3 rec 0`. The counts are since the boot, so they include Part A run 3. Code 5 is a timeout, and code 3 a data byte that was not acknowledged (README).

## Result

### Part A findings

Run 3 is the basis. Runs 1 and 2 stay as history: the right sensor sat loose in run 1, and run 2 used the sensor that the spare replaced.

- **Brush down: passes at 75 mm.**
  - On the table, every line reads 60–67 mm with status 0, so the margin is 8 mm. The noise is small (sd 1.0–1.2 mm).
  - Past the edge, every line reads 597 mm or more with status 0, far above 75 mm.
  - The new mounts sit higher, so the table readings rose from about 50 mm (2026-11-22) to about 63–65 mm.
- **Brush up: the right sensor cannot tell the table from the edge.**
  - It reads 84–89 mm on the table and 86–170 mm past the edge, so the ranges overlap.
  - At 75 mm, all of these count as no table, so the gate refuses forward with the brush up. That is safe, but only while the threshold stays below 84 mm.
  - A higher threshold, such as halfway for the brush down, would make the right sensor see table past the edge with the brush up. So drive only with the brush down, and raise the threshold only together with a firmware interlock for that.
  - With the old right sensor (run 2), the brush up gave 48 mm past the edge, which counts as table even at 75 mm.
- **The left sensor past the edge with the brush up** reads about 1100 mm with status 2 in 109 of 110 lines. The gate counts that as no table, which is correct.
- **The right sensor past the edge reads about 626 mm** with the brush down, while the left reads about 880 mm. Something about 60 cm below may be in its view. This does not matter for the threshold.
- **Sensor errors:** runs 1 and 2 had hundreds to thousands per boot, mostly code 2. Run 3 recorded none in the stream; check `tof` for the error counts during Part B.

| Item | Value | Pass |
|---|---|---|
| Smallest margin to 75 mm on the table (brush down) | 8 mm (both sensors, largest 67 mm) | yes |
| Smallest margin to 75 mm past the edge (brush down) | 522 mm (right, smallest 597 mm) | yes |
| Brush up, on the table and past the edge | right 84–170 mm, all no table at 75 mm | safe at 75 mm only: drive with the brush down |
| Smallest gap at 50 % | 80 mm (run 1, right) | yes |
| Smallest gap at 60 % | 78 mm (run 6, right) | yes |
| Smallest gap at 70 % | 70 mm (run 15, left) | yes |
| Highest duty that passes (top duty for 002) | 70 %, the speed cap (`TRACK_MAX_DUTY_PCT`) | – |

### Part B findings

- **Every run passes.** All 15 runs ended with `done`, and both gaps stayed at 70 mm or more.
- **The gaps shrink with speed:** the mean gap is 87.7 mm at 50 %, 82.8 mm at 60 % and 72.6 mm at 70 %. The two gaps of a run differ by at most 5 mm, so the robot ends square to the edge.
- **No false stops on the table.** Within each duty the gaps lie within 11 mm of each other, and they shrink steadily with speed. A reading that crossed 75 mm on the table would stop a track early, at a random place, with a much larger gap. None of the runs shows that, which agrees with Martin's view that the 8 mm margin from Part A did not cause early stops.
- **The brush stops 22–39 mm before the edge in every run.**
  - The ruler values put the brush 45–52 mm ahead of the nearer track front (gap plus brush value).
  - The new mounts put the sensor points 92.44 mm ahead of the track fronts (CAD), so the sensors look about 40 mm ahead of the brush. The tracks stop before the brush reaches the edge (Martin: the sensors tilt further forward).
  - The PQ12 pushes the brush 20 mm (D008). At 70 % (22–24 mm before the edge), a full push leaves the brush face 2–4 mm short of the edge. At 50 % it stays 15–19 mm short.
- **Sensor errors during the runs:** the left had 10 errors and 2 recoveries, and the right 57 errors and no recoveries. That is far fewer than in runs 1 and 2, and no run ended with `sensor`.
- **Higher duties:** at 70 % the gaps are still 70–75 mm. Duties above 70 % need a higher `TRACK_MAX_DUTY_PCT`, which D023 allows only after this experiment shows a gap. That change needs a decision entry first.
