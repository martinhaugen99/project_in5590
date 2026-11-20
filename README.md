# M.A.T.S

M.A.T.S (Mechanized Assistant for Tabletop Sanitation) is a small animatronic robot on tracks that cleans a desk, built for IN5590 (Rapid Prototyping of Robotic Systems) at UiO. It pushes debris with a brush to the table edge, lowers a bucket over the edge and pushes the debris into it. A face on a display shows what it is doing and what it needs, and it must never drive off the table. This repository holds the CAD, electronics, firmware, host-side tools and documentation. **A test to check git on lab pc**.

## Repository layout

```
project_in5590/
├── cad/                # SolidWorks source (structure and lock workflow: cad/README.md)
├── configs/            # Hydra configs for experiments
├── docs/               # Deliverables, decisions, experiment reports
├── electronics/        # Wiring schematic and datasheets
├── firmware/           # Robot firmware for the Arduino Nano ESP32 (PlatformIO, C++)
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

The software that runs the robot is the firmware in `firmware/`, written in C++ for the Arduino Nano ESP32. The Python package in `src/mats/` only holds host-side tools for experiments on the laptop.

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

The firmware runs on the Arduino Nano ESP32 and is built with [PlatformIO](https://docs.platformio.org/), which `uv sync` installs into the project environment (D013). The pins come from `electronics/schematics.pdf` and are listed in `firmware/include/pins.h`.

**Step 1 (current): bench bring-up.** A serial console tests each part on its own. There is **no edge reflex yet**, so keep the tracks off the ground. The safety layer and the dump cycle come in step 2.

### Build, flash and connect

Run from the repository root:

```bash
uv sync                                    # installs PlatformIO and clang-format
uv run pio run -d firmware                 # build (the first build downloads about 1 GB to ~/.platformio)
uv run pio run -d firmware -t upload       # flash over USB (DFU)
uv run pio device monitor -d firmware      # open the serial console (Ctrl+C to quit)
```

Format the C++ code with `uv run clang-format -i firmware/src/*.cpp firmware/src/*.h firmware/include/*.h`. The pre-commit hook does this on every commit.

### Console commands

| Command | What it does |
|---|---|
| `help` | List the commands |
| `status` | One block with every reading |
| `stream on` / `stream off` | CSV status lines every 100 ms, for logging |
| `en 1` / `en 0` | Enable or disable both L293D chips (tracks and actuator share the enable pin) |
| `stop` | Stop the tracks and the actuator, and disable the drivers. The servos keep their pulses |
| `drive L\|R\|B <duty> <ms>` | Run the left, right or both tracks at -100 to 100 %, for at most 3000 ms |
| `enc reset` | Zero the encoder counts |
| `tof` | Distance and range status of both edge sensors (status 0 = valid) |
| `servo brush\|bucket <us>` / `servo brush\|bucket off` | Send a 500–2500 µs pulse, or stop the pulses |
| `act extend\|retract <ms>` | Jog the linear actuator for at most 500 ms |
| `btn` | Raw level of the two-button ladder |
| `imu on <baud>` / `imu off` | Echo the lines from the OpenLog Artemis |
| `face test` / `face <text>` | Display test pattern, or one word of text |

### Bench checklist

Do these in order, with the tracks off the ground and a hand near the cut-off switch:

1. **Wiring:** the schematic marks the display supply (`TFT_VCC_VERIFY`) and backlight (`CHECK MODULE`) as not yet checked. Check them before power-up.
2. **Boot:** flash and open the console. `tracks`, `actuator`, `servos` and `edge sensors` must report `ok`, and the display must show "M.A.T.S".
3. **Motors stay off through reset:** with 12 V on, press reset. `MOTOR_ENABLE` (D0) must stay low during reset and boot (multimeter), and the actuator must not move. D0 and D1 are the boot UART of the ESP32-S3.
4. **Encoders:** the schematic shows no pull-up resistors on `ENC_L_A`/`ENC_R_A`, which D005 asks for. Check whether they are fitted. Turn each track by hand and watch the counts in `status`. Without phase B, the counts only count up until the motors have been driven.
5. **Directions:** `en 1`, then `drive L 30 500` must move the left track forward, and `drive R 30 500` the right track. `act extend 300` must extend the actuator. If one moves the wrong way, flip `LEFT_MOTOR_REVERSED`, `RIGHT_MOTOR_REVERSED` or `ACT_EXTEND_ON_IN1` in `firmware/include/config.h`.
6. **Values for step 2:** record these and note them in `docs/`:
   - Actuator feedback in mV at both ends. Jog in short steps: the PQ12-P has no limit switches, so stop as soon as the mV value stops changing.
   - Servo pulse in µs for brush up, brush down, bucket stowed and bucket lowered. Start at 1500 µs and move in small steps. Also note what the bucket does after `servo bucket off`.
   - `btn` with no button pressed, with the start button pressed and with the bucket-emptied button pressed.
   - `tof` on the table and with each sensor held past the edge.
7. **IMU:** on the OpenLog Artemis, turn on "Output serial data to TX pin" in its serial logging menu and note the baud rate. Then run `imu on <baud>` and check that lines arrive.

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
