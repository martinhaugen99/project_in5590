# M.A.T.S

M.A.T.S (Mechanized Assistant for Tabletop Sanitation) is a small animatronic robot on tracks that cleans a desk, built for IN5590 (Rapid Prototyping of Robotic Systems) at UiO. It pushes debris with a brush to the table edge and over it. A face on a display shows what it is doing and what it needs, and it must never drive off the table. This repository holds the CAD, electronics, firmware, host-side tools and documentation.

## Repository layout

```
project_in5590/
├── cad/                # SolidWorks source (structure and lock workflow: cad/README.md)
├── configs/            # Hydra configs for experiments
├── docs/               # Deliverables, decisions, experiment reports
├── electronics/        # Wiring schematic and datasheets
├── firmware/           # Robot firmware for the Arduino Leonardo (PlatformIO, C++)
├── print/              # Print settings for the printed parts
├── src/mats/           # Host-side Python package (experiments, analysis, tools)
├── tests/              # Host-side tests (pytest)
├── bom.csv             # Bill of materials
├── pyproject.toml      # Python project, dependencies and tool settings
├── uv.lock             # Exact versions of every Python package (committed)
├── .python-version     # Python version used by uv (3.13)
├── .pre-commit-config.yaml
└── CLAUDE.md           # Project rules for coding agents
```

More folders, such as `data/`, are added when they are needed.

The software that runs the robot is the firmware in `firmware/`, written in C++. It runs on an Arduino Leonardo (D020). The Python package in `src/mats/` only holds host-side tools for experiments on the laptop.

## Setup

1. Install [uv](https://docs.astral.sh/uv/getting-started/installation/) once. It is the only tool installed outside the project.
2. From the repository root:

   ```bash
   git lfs install --local     # Git LFS hooks for the SolidWorks files (cad/README.md)
   uv sync                     # creates .venv/ with the exact versions in uv.lock
   uv run pre-commit install   # runs the checks below on every git commit
   ```

uv downloads Python 3.13 if it is not on your machine. Do not use `pip install` or activate the environment by hand. `uv run <command>` always runs inside the project environment and syncs it first.

## Daily commands

```bash
uv run ruff format .               # format code
uv run ruff check --fix .          # lint and sort imports
uv run mypy                        # type check (strict)
uv run pytest -q                   # run tests
uv run pre-commit run --all-files  # run every hook on all tracked files
```

## Adding dependencies

```bash
uv add <package>          # needed by the code
uv add --dev <package>    # only needed for development (tools, tests)
```

Both commands update `pyproject.toml` and `uv.lock`. Commit both files together. After pulling changes from the other team member, run `uv sync`.

## Firmware

The firmware runs on the Arduino Leonardo (D020) and is built with [PlatformIO](https://docs.platformio.org/), which `uv sync` installs into the project environment (D013). The pins and every wire are drawn in the WireViz sheets in `electronics/wiring/` (D015), and `firmware/include/pins.h` must match them. Render a sheet with `uv run wireviz -f s electronics/wiring/<sheet>.yml`, which needs Graphviz (`brew install graphviz`). `electronics/wiring/Project_wiring-diagram.svg` is an overview drawn in draw.io. It is not the source of truth, and on 2026-11-26 it differs from the sheets (for example in the XSHUT pins), so wire from the sheets.

**Step 2: edge gate and edge approach (D023).** A serial console tests each part on its own and starts the edge approach for the optimization task (D025). The edge gate is always on: a track drives forward only while the edge sensor ahead of it sees the table, and reverse is refused. A motor watchdog in a timer interrupt turns the drivers off if the control loop stalls for about 51 ms while anything moves (D024). An edge sensor that fails three polls in a row is faulted, which stops its track, and the firmware restarts it while nothing moves (D026). The rest of the dump cycle (push the debris over the edge, pull the brush back, back away) comes later.

**Step 3 (D031):**
- **No buttons:** the robot has none. `start` on the console is the start trigger, and `stop` and the cut-off switch stop it.
- **Arms:** removed for the demo, and the arm servo is unplugged from J11. It buzzes at every pulse (D032). `servo bucket` is still in the firmware: do not use it.
- **Display:** rotated for the head with `DISPLAY_ROTATION` in `firmware/include/config.h` (value 1). Set it to 3 if the text is upside down.

### Before the first flash

- **Checked on 2026-11-27:** everything is connected, and Martin has checked the voltages.
- **Uploads and reset (D029):** upload, and press reset, only with the 12 V adapter connected. The bootloader drives D13, the left XSHUT, up to 5 V for up to 8 s, and the sensor must have power then. Avoid the reset button.
- **Edge-sensor supply:** set the LM2596 to 3.3 V and measure its output before the edge sensors are connected. It was set above 5.2 V for the motors, and more than 3.6 V destroys the sensors (D021).
- **Servo supply:** measure 5.2 V at the BEC output before the servos are connected (D014).
- **Power order:** plug in the laptop USB first and the 12 V adapter second. Unplug the adapter first (D021). The edge sensors get their power from the 12 V adapter, so they fail to start if it comes in after the boot. The firmware then retries them once per second, and prints `tof L recovered` and `tof R recovered` when they run (D026).
- **Open checks from D020:** measure about 2.2 kΩ from VDD to SDA and to SCL on both sensor boards (power off), and test the display and the sensors for damage if they were ever powered while wired straight to the Leonardo.

### Build, flash and connect

Run from the repository root:

```bash
uv sync                                    # installs PlatformIO and clang-format
uv run pio run -d firmware                 # build (the first build downloads the AVR toolchain to ~/.platformio)
uv run pio run -d firmware -t upload       # flash over USB (hardware action)
uv run pio device monitor -d firmware      # open the serial console (Ctrl+C to quit)
```

The upload opens the USB port at 1200 baud, which restarts the Leonardo into its bootloader, and then waits for the bootloader port to appear (PlatformIO board `leonardo`). Close the serial monitor before uploading.

There is one build (`env:leonardo`). The bench build of D027 was removed in D031.

The build prints the flash and RAM use. The limits are 28,672 bytes of flash and 2,560 bytes of RAM (D020), and step 3 uses 27,146 and 911 bytes (D031). Flash is the tight one: only 1,526 bytes are left. The display libraries take about 8.6 KB and the console about 5.1 KB (D026). Keep float math out of the firmware (track duty is an integer percent), and wrap console strings in `F()` so they stay in flash.

Format the C++ code with `uv run clang-format -i firmware/src/*.cpp firmware/src/*.h firmware/include/*.h`. The pre-commit hook does this on every commit.

### Console commands

| Command | What it does |
|---|---|
| `help` | List the commands |
| `stream on` / `stream off` | CSV status lines every 100 ms, for logging. The last three columns are `table_l`, `table_r` (1 = the sensor sees the table) and `run` (approach state: 0 idle, 1 starting, 2 driving, 3 squaring, 4 done, 5 aborted) |
| `en 1` / `en 0` | Enable or disable both L293D chips (tracks and actuator share the enable pin). `en 0` also ends an approach run |
| `stop` | End an approach run, stop the tracks and the actuator, and disable the drivers. The servos keep their pulses |
| `start` | The start trigger (D031). Until the dump cycle is written, the same as `approach` at the default 50 % |
| `approach [duty]` | Edge approach at 1–100 % (default 50 %): both tracks drive forward, and each stops for good when its sensor stops seeing the table. Prints one `result` line at the end (below) |
| `drive L\|R\|B <duty> <ms>` | Run the left, right or both tracks for at most 3000 ms. The duty is capped at 100 % (D033). Forward is refused while the sensor ahead of the track does not see the table, and reverse is refused |
| `enc reset` | Zero the encoder counts |
| `tof` | Distance and range status of both edge sensors (status 0 = valid), and `table` or `NO TABLE`. Then `err` (failed polls and start-ups since the reset), `code` (the last error, see below) and `rec` (recoveries). `FAULT` means the sensor did not start or failed three polls in a row. The firmware restarts it while nothing moves |
| `servo brush\|bucket <us>` / `servo brush\|bucket off` | Send a pulse, or stop the pulses. The pulse is clamped to 500–2500 µs for the brush (SER0006) and 750–2250 µs for the bucket (Parallax Standard Servo, D028). Do not use `bucket`: the arms are removed and the arm servo is unplugged (D032). The firmware still refuses it while a track runs, and locks the tracks while it gets pulses (D031) |
| `act extend\|retract <ms>` | Jog the linear actuator at full speed for at most 500 ms. Refused while a track runs, and the tracks are locked while it moves |
| `face <text>` | One word of text on the display. It blocks the loop while it draws, so the watchdog stops any moving track |

**Edge-sensor faults (D026).** When a sensor faults, the console prints one line, such as `tof L fault: code 2, own addr no, 0x29 yes`. When it runs again, it prints `tof L recovered`.
- **`code`:** the last error.
  - 2: no device answered at the address
  - 3: a data byte was not acknowledged
  - 4: other bus error
  - 5: timeout
  - 6: a read came back short
  - 7: wrong model ID at start-up
  - 8: a start-up wait ran out
- **`own addr yes`:** the sensor still answered at its own address, so the bus had a glitch.
- **`own addr no, 0x29 yes`:** the sensor reset itself and is back at its default address, after a glitch on XSHUT or on its 3.3 V supply.
- **`no` and `no`:** the sensor is off the bus: no power, a loose wire, or a bus held low.

### Bench checklist

Do these in order, with the tracks off the ground and a hand near the cut-off switch:

1. **Wiring:** check the wiring against sheets 01–03, especially the display dividers (R10–R17), the XSHUT wires (D13 to the left sensor, D5 to the right sensor, D029) and the 3.3 V rail of the edge sensors. With power on and the sensors running, measure XSHUT at J7: it should be close to 3.3 V (D029).
2. **Boot:** flash and open the console. `edge sensors` must report `ok`, and the display must show "M.A.T.S", readable in the head. If the text is upside down, set `DISPLAY_ROTATION` to 3 in `firmware/include/config.h` and flash again (D031).
3. **Motors stay off through reset:** with 12 V on, press reset. `MOTOR_ENABLE` (D12) must stay low during reset and boot (multimeter), and the actuator must not move. The pins float during reset, and R2 holds the line low (D020).
4. **Encoders:** turn each track by hand and watch the counts with `stream on`. Without phase B, the counts only count up until the motors have been driven.
5. **Directions:** put a surface about 50 mm under both sensors, so the edge gate allows forward motion. Then `en 1`, and `drive L 30 500` must move the left track forward, and `drive R 30 500` the right track. `act extend 300` must extend the actuator. If one moves the wrong way, flip `LEFT_MOTOR_REVERSED`, `RIGHT_MOTOR_REVERSED` or `ACT_EXTEND_ON_IN1` in `firmware/include/config.h`. The track directions were checked on 2026-11-22.
6. **Values measured on 2026-11-22** (D023): the slowest reliable duty, 50 %. The sensor values of that day came from the old sensor mounts.
   **Measured on 2026-11-27, with the new sensor mounts** (experiment 001, Part A):
   - `tof` with the brush down: 60–67 mm on the table, and at least 597 mm past the edge (run 3, 10 s streams).
   - From CAD: 92.44 mm from each sensor point to the front of its track.
   - Brush servo: 1500 µs puts the brush on the table (down), and 1700 µs lifts it off (up).
   - With the brush up, the right sensor reads 84–170 mm both on the table and past the edge, so it cannot tell them apart. At the 75 mm threshold all of these count as no table, so the gate refuses forward. Drive only with the brush down.

   **Still to measure** for the rest of the dump cycle:
   - The arm limits (experiment 003) are cancelled: the arms are removed for the demo (D032).
   - Actuator feedback (raw) at both ends. Jog in short steps: the PQ12-P has no limit switches, so stop as soon as the value stops changing.

### Edge approach test ladder (step 2)

Each step needs the one before it to pass. Keep a hand at the cut-off switch, the brush down with its servo off, the arm servo unplugged (D032), and the actuator fully retracted.

1. **Bench, tracks off the ground**, with a board about 50 mm under both sensors (`tof` shows `table` on both sides):
   - `en 1`, then `drive B 50 500`. Both tracks must stop after 500 ms, and the console must still answer. This failed before D024.
   - `drive B 50 2000`, and pull the board away from one sensor. That track must stop at once, and the other keep running.
   - **Sensor faults (D026):** `stream on`, then `drive B 50 2000` ten times. Note every `tof ... fault` line and the `err` counts in `tof`. Each fault must stop its track and be followed by `tof ... recovered` within about 1 s after the tracks stop.
   - Pull the SDA wire of one sensor during `drive B 50 2000`. Its track must stop, and a fault line must print. Plug it back in: the console must print `recovered`.
   - Boot with the 12 V adapter unplugged, then plug it in. Both sensors must print `recovered` within about 1 s.
   - **Watchdog:** `drive B 30 3000`, then at once `face hi`. The draw blocks the loop, so the console must print `watchdog: loop stalled, drivers off` and both tracks must stop. `en 1` to go on.
   - `drive L -30 500` must be refused (reverse is off).
   - `start` must run the approach at 50 % and print a `result` line. Pull the board away to end it.
   - `approach`, then pull the board away from one side and then the other. Each track must stop on its own side, and the console must print `result,done,...`.
2. **Floor:** `approach` with no edge in reach. The tracks must stop after 8 s with `result,timeout,...`. Check that `stop` ends a run at once.
3. **Table with a catch below the edge:** drive straight at the edge with `approach 50`, then 60 and 70. Measure the gap between each track front and the edge (experiment 001, [`docs/experiments/001-edge-calibration.md`](docs/experiments/001-edge-calibration.md)). Do not go faster than a duty that leaves a clear gap.
4. **Bare table:** approach runs at each duty for the optimization (experiment 002, D025, [`docs/experiments/002-brush-overshoot.md`](docs/experiments/002-brush-overshoot.md)). Measure with a ruler where the brush stops relative to the edge, and the gap in front of each track. The target is the brush 16–18 mm before the edge (D033). The cap is 100 % since D033: test 80, 90 and 100 % in that order, and stop raising the duty once a track gap falls below 30 mm.

The result line is `result,<outcome>,<duty_pct>,<first_side>,<t_first_ms>,<t_total_ms>,<enc_l>,<enc_r>`:
- **outcome:** `done`, `timeout`, `square_timeout`, `sensor` (a sensor stopped delivering readings), `interlock`, or `stopped` (`stop` or `en 0`).
- **first_side:** `L` or `R` for the track that stopped first, `B` if both stopped in the same tick, `-` if no track reached the edge (for example on a timeout).
- **Times:** from the start of the tracks, until the first track stopped and until the end.
- **Encoder counts:** for the run.

## Running experiments (Hydra)

Experiments are configured with [Hydra](https://hydra.cc/docs/1.3/intro/). The example entry point is `src/mats/experiment.py`, with its config in `configs/experiment.yaml`.

```bash
uv run mats-experiment                                      # default config
uv run mats-experiment name=stopping_distance notes="first try"
uv run mats-experiment --multirun notes=a,b                 # one run per value
```

Each run gets its own folder, `outputs/<name>/<date>_<time>/` (or `multirun/<name>/<date>_<time>/<job>/`), containing:

| File | Content |
|---|---|
| `.hydra/config.yaml` | The full config used for the run |
| `.hydra/overrides.yaml` | The command-line overrides |
| `git.yaml` | Git commit and whether there were uncommitted changes (`dirty`) |
| `experiment.log` | The run log |

`outputs/` and `multirun/` are not committed. Each experiment report in `docs/experiments/` names its run folder and copies the key results. Sweep over parameters, not over `name`: `name` is part of the folder path, so sweeping it splits the runs across folders.
