# Project Specific Agents Instructions

Placeholders written as `[FILL IN: …]` are facts only Martin can supply. Don't guess them; ask.
Items marked **TBD** are open design decisions. They are settled through `docs/decisions.md`, not assumed.

## Project overview

This project builds an animatronic tabletop robot from scratch. The robot shows emotions on a display, drives on tracks (belts), can clean a target such as a desk, and must never drive off the table. The work covers component selection, CAD, 3D printing, electronics, firmware and software, integration and experiments. It is the course project for IN5590 (Rapid Prototyping of Robotic Systems) at UiO.

Build window: early October → late November, ending with a public demo November 30th. Weigh every decision against that deadline. A robot that reliably shows emotions and stays on the table while cleaning on demo day is the ultimate goal.

Logical architecture. Both layers run on the two cores of the microcontroller (D004):
```
edge sensors (2), encoders, ──► LOW LEVEL ───────────────────► driver chip 1 ──► track motors (rear sprockets)
actuator position, IMU          edge reflex, dump interlock, ──► servo signals ──► brush-arm and bucket-arm servos
                                motor, servo and actuator    ──► driver chip 2 ──► linear actuator (brush push)
                                control, watchdog
                                   ▲ commands     │ state / events
buttons ──────────────────► HIGH LEVEL: cleaning behavior, emotion state ──► display renderer
```

## Requirements (draft)

- **Must:** show at least 7 states that people can tell apart (neutral, working, happy, sad, surprised, scared, angry); drive forward, reverse and turn in place on tracks; detect the table edge and stop or back away before any part of a track leaves the surface, at every reachable speed; run one dump cycle at the press of a button (lower the brush, drive to the edge, square up, lower the bucket, push, stow, back away), with the tracks locked while the bucket is lowered or the brush is pushed forward; reach past the table edge with the bucket, keep it upright and never tip the robot; have a physical switch that cuts motor power.
- **Should:** clean a table on its own by repeating the dump cycle along different edges; raise the collection rate to at least 80 % by mass through tuning (the optimization task); react to events with the states (dump cycle done, bucket full, picked up, unplanned edge, blocked).
- **Could:** a camera that finds dirty spots.
- **Priority when goals conflict:** edge safety > motor cut-off > cleaning > expression. M.A.T.S drives to the edge on purpose on every dump cycle, so cleaning and edge avoidance conflict all the time, not just in rare cases. The dump cycle never overrides the edge stop or the dump interlock.

## Hardware (chosen in D004–D010; verify against the datasheets before relying on it)

| Area | Choice | Decision |
|---|---|---|
| Compute architecture | MCU only: both layers on the two cores of the microcontroller. A laptop on USB powers it and logs data | D004 |
| Microcontroller | [Arduino Nano ESP32](https://store.arduino.cc/products/nano-esp32): ESP32-S3, 3.3 V logic | D004 |
| Single-board computer | None for the Must and Should goals | D004 |
| Display | 2.8" SPI TFT module, ILI9341 controller, 240 × 320 pixels (board V1.2), in a fixed head. Datasheet not yet linked | D009 |
| Dirt detection (Could) | Raspberry Pi 5 with a Raspberry Pi camera | D004 |
| Edge sensing | 2 × VL53L4CD time-of-flight sensors ([ST SATEL-VL53L4CD](https://no.rs-online.com/web/p/sensor-development-tools/2410906), RS 241-0906) on the brush bracket, one ahead of each track. No rear sensors | D007 |
| Motors + driver | 2 × [DAGU DG01D 48:1](https://www.sparkfun.com/hobby-gearmotor-140-rpm-pair.html) gearmotors, about 140 RPM without load, suggested 4.5 V; 2 × Yumo E6A2-CW3C encoders, 200 pulses per turn, open collector ([datasheet](https://www.fa.omron.co.jp/data_pdf/cat/e6a2-c_ds_e_8_2_csm490.pdf)); 2 × [TI L293DNE](https://www.ti.com/lit/gpn/l293d), one for the track motors, one for the linear actuator | D006 |
| Tracks | Printed TPU bands with teeth on the inside, printed PLA sprockets with guide ridges, front sprocket in a slotted mount for tension | D006 |
| Arm actuators | 2 × [DFRobot SER0006](https://www.dfrobot.com/product-255.html) micro servos (180°) for the brush arm and the bucket arms, plus a larger lab servo as a spare; [Actuonix PQ12-100-12-P](https://cdn.robotshop.com/media/f/fir/rb-fir-42/pdf/actuonix-pq12-datasheet.pdf) linear actuator, 20 mm stroke, 12 V, position feedback | D008 |
| IMU | [SparkFun OpenLog Artemis](https://www.sparkfun.com/sparkfun-openlog-artemis.html) with its built-in ICM-20948, data over serial | D010 |
| Power | 12 V bench supply for the actuators and the encoders, with a cut-off switch on the robot; two adjustable step-down converters for the track motors and the 5 V servos (model not yet confirmed); laptop USB for the microcontroller, edge sensors and display. No battery | D005 |

When a choice changes, record it in a new decision entry and update its row with the part, its key specs, a datasheet link and the decision ID (D###).
- Already on hand or provided by the lab: every part in `bom.csv` with status `lab` (confirmed 2026-10-24); printer Isaac (Creality Ender-3 V3 SE); PLA and TPU filament; CNC access
- Budget: unknown, faculty decides. Parts the lab does not have are requested from the faculty with links to RS, Farnell or Digikey, and the faculty decides whether to buy them (`docs/ASSIGNMENT.md`). Items to order have status `to_order` in `bom.csv`.

## Key technologies

- **No Docker.** Every part of the project uses a project-local, pinned environment, and nothing is installed globally on the host (see "Environment setup").
- CAD: SolidWorks, AutoDesk
- Firmware toolchain and language: TBD (the MCU is the Arduino Nano ESP32, D004)
- High-level language and framework: the high level runs on the microcontroller next to the low level (D004), with no separate high-level computer or framework.
- Host-side Python (experiments, analysis, tools): package `mats` in `src/mats/`, Python 3.13.
  - Environment and dependencies: uv. `pyproject.toml` holds version ranges, `uv.lock` pins exact versions and is committed.
  - Formatting, linting and import sorting: ruff (the `I` rules replace isort in this project).
  - Type checking: mypy in strict mode. Tests: pytest. Git hooks: pre-commit.
  - Experiment configuration: Hydra 1.3 (`hydra-core>=1.3.7,<1.4`). See "Experiments with Hydra".
  - All tool settings live in `pyproject.toml` and `.pre-commit-config.yaml`.
- Version control: git, hosted on GitHub (`martinhaugen99/project_in5590`)

## Repository layout (proposed; create folders only when needed)

```
project_in5590/
├── cad/              # SolidWorks source + STEP exports (structure and lock workflow: cad/README.md)
├── print/            # STL/3MF + print settings per part
├── electronics/      # Wiring diagram, schematic, pinout.md (single source of truth for pins)
├── firmware/         # Low-level controller code
├── software/         # Code for the Raspberry Pi 5 (only if the Could camera is added)
├── assets/emotions/  # Source graphics/animations for each state
├── src/mats/         # Host-side Python package (experiments, analysis, tools)
├── configs/          # Hydra configs for experiments
├── tests/            # Host-side tests
├── data/             # Logged experiment data (large raw files gitignored)
├── outputs/          # Hydra run folders (gitignored); multirun/ for sweeps
├── docs/             # project_plan.md, decisions.md, experiments/, report/
├── bom.csv           # item, part, function, MPN, supplier, link, datasheet, qty, unit price (NOK), status
├── pyproject.toml    # Python project, dependencies and tool settings
├── uv.lock           # Exact Python package versions (committed)
├── .python-version   # Python version for uv
├── .pre-commit-config.yaml
└── README.md
```

## Mission & scope

- Build the MVP first: drive on tracks → edge stop → one dump cycle at the press of a button. Autonomous cleaning, the collection-rate optimization and the reactions come only after the MVP works on a real table.
- Every experiment must be traceable to a git commit, its parameters, and the raw data.
- Declare work complete only when firmware builds without warnings, `uv run ruff format --check .`, `uv run ruff check .`, `uv run mypy` and `uv run pytest -q` pass, and the docs reflect any change in behavior.

## Capabilities & limits

### You can
- Modify code, configs, BOM, pinout, print settings and documentation.
- Research components and compare 2–3 options with trade-offs: cost, shipping time to Norway, power draw, size and mass, library support, and what the choice forces on the rest of the system. Link datasheets.
- Build firmware and run host-side tests and linters in the project-local environment.
- Produce analysis artifacts (plots, experiment reports) in their designated directories.
- Suggest improvements and challenge choices that put the deadline or safety at risk.

### Escalate or stop when
- An action would send commands to real hardware (motors, servos, flashing firmware). Always get explicit confirmation first, and state which limits are in place (speed cap, edge stop active, robot on a stand or the floor).
- Physical parameters (dimensions, masses, currents, stopping distance, sensor thresholds) are missing or look implausible. Ask rather than invent numbers.
- A change would break an interface between subsystems: pinout, inter-board protocol, power rails, or the emotion-state definition.
- Something would cost money. Recommend, and Martin orders.
- Adding unpinned dependencies, or installing anything globally.
- A task would create a module, package or subsystem that wasn't named in the request. Name it and wait.
- An approach has failed twice. Report rather than continue.

### Never
- Commit secrets, large binaries (raw video, big logs) or build outputs.
- Disable or bypass safety limits (edge stop, watchdog, speed cap) to make a test pass, or let high-level code override the edge stop.
- State electrical or mechanical specs or library APIs from memory. Check the datasheet or official docs and cite them.

## Interaction guidelines

- Martin writes most of the code. Default to explaining, proposing and reviewing, and write a module only when asked for that specific module. If a task seems to need three files, say so and wait rather than writing three files.
- Report before a second attempt. If an approach fails, stop and say what failed, what it suggests and what the options are.
- One deliverable per turn, then stop. Prefer stopping too early.
- Answer in English unless asked otherwise. Code, comments, commit messages and docs are in English.
- Use a friendly, concise tone focused on actions, evidence and next steps.
- The general collaboration rules live in `~/.claude/CLAUDE.md` and are deliberately not repeated here.

## Development workflow

### Environment setup
- No Docker. Dependencies are pinned in lock files and installed only through the project's environment definitions, never ad hoc.
- Dev machine(s): [FILL IN: which machine is primary, its OS, and which one flashes the robot]
- Python: uv, with Python 3.13 pinned in `.python-version`. `uv sync` creates `.venv/` from `uv.lock`. Add packages only with `uv add <pkg>` or `uv add --dev <pkg>`, and commit `pyproject.toml` and `uv.lock` together. Never use `pip install` or a global Python. uv itself is the only tool installed outside the project. It keeps a shared download cache in `~/.cache/uv`, and pre-commit keeps its hook environments in `~/.cache/pre-commit`.
- Firmware: [FILL IN after the MCU decision: toolchain, e.g. PlatformIO, which keeps toolchains per project]
- On-robot computer: none (D004). If the Could camera is added: [FILL IN: OS image, how code is deployed to the Raspberry Pi 5]

### Core commands (fill in during scaffolding)
```bash
# Set up after clone:     git lfs install --local && uv sync && uv run pre-commit install
# Format:                 uv run ruff format .
# Lint + sort imports:    uv run ruff check --fix .
# Type check:             uv run mypy
# Host-side tests:        uv run pytest -q
# All git hooks:          uv run pre-commit run --all-files
# Run an experiment:      uv run mats-experiment name=<name> [key=value ...]
# Build firmware:         [FILL IN]
# Flash firmware:         [FILL IN]   # hardware action: needs confirmation
# Serial monitor / logs:  [FILL IN]
# Run high-level code:    part of the firmware (D004)
```
Import sorting is part of `uv run ruff check --fix .` (rule `I`). The global `uv run isort .` step does not apply in this project.

### Verification & audit trail
- After changing runnable code, run the smallest meaningful subset of build, lint and tests, and include the relevant output in summaries.
- Hardware test ladder: bench with tracks off the ground → floor → table with a barrier → bare table.
- Record the commands executed (purpose and outcome) so the work stays reproducible.

## Technical standards

### Robotics conventions
- SI units internally (m, kg, s, rad, m/s). Convert raw sensor values at the driver boundary.
- Body frame: x forward, y left, z up (ROS REP-103). Positive yaw is counter-clockwise seen from above.
- Physical parameters come from CAD, datasheets or measurement, never guesses. Record their source in `docs/`.
- Edge-sensor thresholds are calibrated per surface and stored as named, documented constants.

### Low-level controller
- Fixed-rate, non-blocking control loop. Document loop rates in `docs/`.
- The edge check runs every loop iteration, before any motor command is applied.
- Motors stop on invalid sensor readings, and when commands stop arriving (watchdog timeout documented in `docs/`).
- The speed cap lives in firmware, not only in high-level code.
- A track may move forward only while the edge sensor ahead of it sees the table. The robot reverses only along the path it has just driven, and never farther than the encoders measured on the way forward.
- The dump interlock lives in the low level. The bucket is lowered only when the tracks are stopped and both edge sensors see the edge ahead, and the tracks stay locked until the bucket is up and the brush is back.
- If the USB cable comes loose, pull-down resistors on the driver inputs stop the motors.

### Inter-board protocol (only if there are two processors, for example when the Could camera on a Raspberry Pi 5 is added)
- Framed with a checksum and versioned from day one (a `HELLO` message carrying the protocol version).
- Message definitions live in a single source shared or generated for both sides, never in hand-maintained copies. Include a host-side test.

### Display & emotions
- The behavior layer sets one state from a single shared enum (the seven states in D009), and the renderer draws it. Rendering never blocks the control loop.

### Code style
- Python: type hints, Google-style docstrings, 4-space indents, 88-character lines, f-strings, pathlib over os, ALL_CAPS constants, `_private` methods. Ruff formats, lints and sorts imports (rules: defaults + `I`, `B`, `UP`, `SIM`, `D` with the Google convention). mypy runs in strict mode. Settings live in `pyproject.toml`; change them there, not with inline flags.
- C/C++ (if used): [FILL IN: language standard, formatter/style, naming convention]

### Mechanical & 3D printing
- Parametric CAD: drive the key dimensions (motor mounts, track width, display cutout, sensor positions, wall thickness, clearances) from named parameters.
- Each CAD module has one owner (`cad/README.md`). Dimensions shared between modules are listed in `docs/cad_parameters.md`, and each part holds them as its own global variables with the same names. No linked equation files (D012).
- For each printed part, record the printer, material, nozzle, layer height, infill, orientation, supports and tested clearances.
- Mount the edge sensors ahead of the tracks by more than the *measured* stopping distance at top speed.
- Keep the center of gravity low and between the tracks, also in the worst pose (bucket lowered past the edge and full, brush pushed forward). Check it in CAD using real component masses.

## Testing & validation

- Bring up each component alone on the bench before integrating it.
- Essential tests: the edge stop triggers at every speed; the watchdog stops the motors; the motors stop when the USB cable comes loose; the dump interlock ignores drive commands while the bucket is lowered or the brush is pushed forward; every state renders; the protocol round-trips (if there are two processors).
- Host-side tests in `tests/` mirror the source modules. Use Arrange-Act-Assert, and mark slow or hardware-in-the-loop tests.
- Each experiment gets `docs/experiments/NNN-name.md`: question, setup, procedure, measured quantity, pass criterion, raw data, result. Candidates:
  - edge detection across surfaces, lighting and speeds
  - stopping distance vs speed
  - reach, tipping and servo torque of the bucket arms
  - collection rate before and after tuning (the optimization task)
  - whether people recognize each state

### Experiments with Hydra
- Each experiment type gets a config in `configs/` (later as config groups in subfolders) and an entry point in `src/mats/`. `src/mats/experiment.py` is the example to copy.
- Physical values in configs (speeds, thresholds, distances) follow "Robotics conventions": their source is recorded in `docs/`, and they are never guessed.
- Use `@hydra.main(version_base="1.3", ...)`. Pass `config_path` as an absolute path (see `CONFIG_DIR` in `experiment.py`). A relative path fails when the script runs as an installed command.
- Every run writes `outputs/<name>/<date>_<time>/` with `.hydra/config.yaml`, `.hydra/overrides.yaml`, `git.yaml` (commit and `dirty` flag) and the log. Sweeps (`--multirun`) go to `multirun/`. Do not sweep over `name`, since it is part of the folder path.
- `outputs/` and `multirun/` are gitignored. The experiment report in `docs/experiments/NNN-name.md` names the run folder and copies the key results. Raw data that must be kept goes in `data/`.
- Runs for a report should come from a clean, committed tree (`dirty: false` in `git.yaml`).
- Stay on Hydra 1.3.x. Hydra 1.4 has breaking changes, so an upgrade is a decision entry in `docs/decisions.md`.

## Pre-production policy

Early development: no backward-compatibility shims, no deprecated code kept around, and refactors may freely change names and behavior. Tests are updated to match immediately.

## Git workflow

- Imperative commit subjects of 50 characters or less. Keep `main` runnable and use short-lived feature branches.
- Add tests alongside changes in behavior.
- SolidWorks files (`.SLDPRT`, `.SLDASM`, `.SLDDRW`) are in Git LFS as lockable files (D011). Lock a file before editing it, and push before unlocking it (`cad/README.md`). STL, 3MF and STEP files stay in plain git.
- Pre-commit hooks (`.pre-commit-config.yaml`):
  - From `pre-commit-hooks`: trailing whitespace, end of file, YAML/TOML syntax, merge conflicts, private keys, and files larger than 3000 KB. The trailing-whitespace and end-of-file hooks skip `cad/`, and the size check skips files in Git LFS.
  - Local hooks: `uv lock --check`, ruff check, ruff format and mypy. These run through `uv run` with the versions in `uv.lock`.
  - Do not bypass the hooks with `--no-verify`. Firmware formatters and linters are added once the toolchain is chosen.

## Safety

- Power comes from a 12 V bench supply and the laptop USB, with no battery (D005). If a battery is added later: use a protected pack or a BMS, fuse the battery output, charge only with a proper charger, and never charge unattended.
- Follow the lab's rules for the 3D printers and CNC machines.

## Governance & maintenance

- Owner: Martin. Team: Markus Yuki Nishimura (Yuki)
- Record decisions in `docs/decisions.md` as numbered entries (D001, D002, …) with context, options, choice and reasons. Update this file whenever a decision changes the stack or hardware.
