# Decisions

Numbered record of project decisions (D001, D002, …). Each entry gives the context, the options, the choice and the reasons. To change a decision, add a new entry that supersedes the old one, and set the old entry's status to "Superseded by D###". Update `CLAUDE.md` whenever a decision changes the stack or the hardware.

| ID | Date | Decision | Status |
|---|---|---|---|
| D001 | 2026-10-10 | Host-side Python tooling: uv, ruff, mypy, pytest, pre-commit | Accepted |
| D002 | 2026-10-10 | Hydra 1.3 for experiment configuration | Accepted |
| D003 | 2026-10-21 | Concept change: the M.A.T.S desk cleaner replaces the DeskPal desk companion | Accepted |
| D004 | 2026-10-21 | Compute: one Arduino Nano ESP32 runs both layers | Accepted |
| D005 | 2026-10-21 | Power: 12 V bench supply and laptop USB, no battery | Accepted |
| D006 | 2026-10-22 | Drive: DC gearmotors with encoders, L293D driver chips, printed TPU tracks | Accepted |
| D007 | 2026-10-22 | Edge sensing: two time-of-flight sensors, one ahead of each track | Accepted |
| D008 | 2026-10-22 | Dump mechanism: servo-driven arms and a linear actuator | Accepted |
| D009 | 2026-10-22 | Display and the seven states | Accepted |
| D010 | 2026-10-22 | IMU: SparkFun OpenLog Artemis | Accepted |
| D011 | 2026-10-24 | CAD files in Git LFS with file locks | Accepted |

## D001: Host-side Python tooling

- **Date:** 2026-10-10
- **Status:** Accepted
- **Decided by:** Martin

### Context

Martin and Yuki share the host-side Python code (experiments, analysis, tools). Both need the same Python and package versions, one code style, and checks that run before each commit. `CLAUDE.md` rules out Docker and global installs.

### Options

1. **uv + ruff:**
   - uv manages Python, the environment and the dependencies, with a committed lock file.
   - Ruff formats, lints and sorts imports.
2. **uv + ruff + isort:** Same as option 1, but isort sorts imports as a separate step.
3. **pip + venv:** Plain `pip` and `venv` with a requirements file, plus separate tools for formatting and linting.

### Choice

Option 1:
- **uv** manages Python 3.13 (`.python-version`), the environment (`.venv/`) and the dependencies (`pyproject.toml` plus the committed `uv.lock`).
- **ruff** formats, lints and sorts imports (rule `I`). isort is not used. Lint rules: the defaults plus `I`, `B`, `UP`, `SIM` and `D`, with the Google docstring convention.
- **mypy** runs in strict mode.
- **pytest** runs the tests, with the markers `slow` and `hardware`.
- **pre-commit** runs generic file checks, plus ruff, mypy and `uv lock --check` through `uv run`.

### Reasons

- `uv.lock` pins exact versions, so `uv sync` gives both team members the same environment. uv is the only tool installed outside the project.
- One tool for formatting, linting and import sorting means one config and no conflicts between tools.
- The hooks call the tools through uv, so they use the versions in `uv.lock`, and hook runs and manual runs agree.
- Python 3.13 rather than 3.14: hydra-core 1.3.7 lists Python 3.7–3.11 on [PyPI](https://pypi.org/project/hydra-core/1.3.7/). Every check and experiment run passed on 3.13. 3.14 was not tested.

### Consequences

- Never use `pip install`. Add packages with `uv add` or `uv add --dev`, and commit `pyproject.toml` and `uv.lock` together.
- uv and pre-commit keep caches outside the project, in `~/.cache/uv` and `~/.cache/pre-commit`.
- The commands are listed in `README.md` and in `CLAUDE.md` ("Core commands").

## D002: Hydra 1.3 for experiment configuration

- **Date:** 2026-10-10
- **Status:** Accepted
- **Decided by:** Martin

### Context

The experiments need configurable parameters, and each run must be traceable to a git commit, its parameters and its raw data (`CLAUDE.md`, "Mission & scope"). Planned experiments:
- edge-sensor thresholds
- stopping distance vs speed
- follow-controller tuning
- emotion recognition

The HUSKYLENS runs its own detection model, so no model is trained on the laptop.

**Note (2026-10-24):** D003 replaced following with cleaning, and D004 dropped the HUSKYLENS. The Hydra choice still holds. The current list of planned experiments is in D003.

### Options

1. **Version:**
   - hydra-core 1.3.7: the latest stable release, 14 Sep 2026.
   - 1.4.0rc1: a prerelease from 10 Oct 2026 with breaking changes. It also deprecates `version_base` ([release notes](https://github.com/facebookresearch/hydra/releases)).
2. **Where the configs live.** Hydra reads a relative `config_path` as a Python package name when the script runs as an installed command:
   - (a) `configs/` at the repository root, passed to Hydra as an absolute path.
   - (b) `src/mats/configs/` inside the package.
   - (c) Root `configs/` with a relative path, run as `uv run python src/mats/experiment.py` with no named command.

### Choice

- `hydra-core>=1.3.7,<1.4`, used as `@hydra.main(version_base="1.3", ...)` ([version_base](https://hydra.cc/docs/1.3/upgrades/version_base/)).
- Option 2a: the configs are in `configs/`, and `CONFIG_DIR` in `src/mats/experiment.py` passes the absolute path.
- Each run writes `outputs/<name>/<date>_<time>/` (sweeps: `multirun/`) with these files:
  - `.hydra/config.yaml`
  - `.hydra/overrides.yaml`
  - `git.yaml` (commit and `dirty` flag)
  - the log

### Reasons

- A seven-week build needs a stable tool. A prerelease with breaking changes is a risk with no benefit for this project.
- Root `configs/` is easy for both team members to find, and the absolute path is a one-line fix.
- Hydra saves the config and the overrides of each run, and `git.yaml` adds the commit. Together they meet the traceability rule.

### Consequences

- The absolute config path works only with the editable install that uv uses by default.
- `outputs/` and `multirun/` are gitignored. Each experiment report in `docs/experiments/` names its run folder and copies the key results. Raw data to keep goes in `data/`.
- Upgrading to Hydra 1.4 needs a new decision entry.

## D003: Concept change from desk companion to desk cleaner

- **Date:** 2026-10-21
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

The first draft of Deliverable 1 (9 October) proposed DeskPal, a desk companion that shows emotions and follows its person. The feedback said that it was too similar to Cozmo, both in appearance and in function, and asked us to start from a particular problem and design the robot around it. The feedback also asked for measurable goals, a consistent sketch, a BOM based on the course equipment, cable power instead of a battery, and more time for parts, CAD and construction. The drafts are in `docs/archive/`, and the current draft is in `docs/`.

### Options

1. Keep the desk companion and make it different from Cozmo.
2. A new concept built around a problem: a robot that cleans a desk.

### Choice

Option 2: **M.A.T.S** (Mechanized Assistant for Tabletop Sanitation).
- It pushes debris ahead of a brush to the table edge, lowers a bucket on two arms over the edge, and pushes the debris into the bucket. A person empties the bucket.
- Its face shows what it is doing and what it needs (D009).
- Following a person is dropped.
- The optimization task is the collection rate: the share of a weighed amount of spread debris, by mass, that ends up in the bucket.

### Reasons

- The table edge becomes part of the task, not only a hazard: the robot drives to the edge on purpose to empty its brush. Robot vacuums and Cozmo only avoid edges.
- The face shows the work instead of serving play, which ties the expression to a use case.
- The brush, the arms and the bucket put most of the work into the physical design, which is the main focus of the course project.

### Consequences

- The requirements, the logical architecture and the priority order in `CLAUDE.md` change. The robot meets the edge on every dump cycle, so cleaning and edge safety conflict all the time. The dump cycle never overrides the edge stop.
- Planned experiments:
  - edge stop across tables, approach angles and speeds
  - stopping distance vs speed
  - reach, tipping and servo torque of the bucket arms
  - collection rate before and after tuning
  - emotion recognition
- The Python package is renamed from `deskpal` to `mats` (2026-10-24). The experiment command is `uv run mats-experiment`.

## D004: Compute: one Arduino Nano ESP32

- **Date:** 2026-10-21
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

The low level runs the edge reflex, the motor, servo and actuator control, the dump interlock and the watchdog in a fixed-rate loop. The high level runs the cleaning behavior, the emotion state and the display renderer. The feedback asked us to use the course equipment. The course lists the Arduino Uno Q, the Arduino Nano ESP32 and the Raspberry Pi 5, and allows a laptop on USB (`docs/ASSIGNMENT.md`).

### Options

1. ESP32-S3-DevKitC-1, bought, with a HUSKYLENS 2 smart camera for person detection (first draft).
2. Arduino Nano ESP32 from the lab, one microcontroller for both layers.
3. Raspberry Pi 5 alone, or a Raspberry Pi 5 for vision together with a microcontroller.

### Choice

Option 2: [Arduino Nano ESP32](https://store.arduino.cc/products/nano-esp32) (ESP32-S3, 3.3 V logic).
- One core runs the low-level control loop. The other core runs the cleaning behavior and the display renderer.
- A laptop on USB powers the board and logs the data for the experiments.
- A Raspberry Pi 5 with a Raspberry Pi camera is used only for the Could goal: a camera that finds dirty spots.

### Reasons

- It is course equipment, as the feedback asked.
- The new concept needs no person detection, so the Must and Should goals need no camera and no second computer.
- One processor avoids a protocol between two boards.

### Consequences

- There is no inter-board protocol unless the Could camera is added.
- The high level can only send commands to the low level, so it can never override the edge stop or the dump interlock.
- The microcontroller has few free pins. The deliverable's risk table lists how to save pins.
- The firmware toolchain is still open.

## D005: Power: 12 V bench supply and laptop USB

- **Date:** 2026-10-21
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

The first draft proposed a 2S Li-ion pack with protected cells, a fuse and an external charger. It left open issues: the cell length against the holder, the series wiring of the holder, and the fuse rating. The feedback recommended a cable power source instead of a battery, and said that a battery can be added later.

### Options

1. A 2S Li-ion pack with a fuse and a charger (first draft).
2. Cable power: a 12 V bench supply for the actuators, and the laptop USB for the electronics.

### Choice

Option 2:
- A 12 V bench supply from the lab powers all actuators and the encoders through one cable. A switch on the robot cuts the power to all motors, servos and the linear actuator.
- The linear actuator runs directly on 12 V.
- One adjustable step-down converter feeds the track motors, set so that the motors get about their suggested 4.5 V at full speed. A second converter feeds the servos at 5 V. The converter model is not yet confirmed.
- The encoders run on 12 V, connected before the cut-off switch. Pull-up resistors to 3.3 V keep their open-collector outputs at the microcontroller's logic level.
- The laptop USB powers the microcontroller, the edge sensors and the display. All grounds are connected.

### Reasons

- The feedback recommended it.
- It removes the open battery issues of the first draft.

### Consequences

- Two cables leave the robot at the rear.
- If the USB cable comes loose, the microcontroller loses power, and pull-down resistors on the driver inputs stop the motors.
- When the converter model is known, check it and the driver chips against the stall currents of the motors and servos.
- The battery rules in `CLAUDE.md` apply only if a battery is added later.

## D006: Drive: DC gearmotors with encoders, L293D driver chips, printed TPU tracks

- **Date:** 2026-10-22
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

The robot drives on tracks. The first draft proposed Pololu micro metal gearmotors with magnetic encoders, a TB6612FNG driver and a bought Pololu track set, all ordered from abroad. The feedback asked us to use the course and lab equipment. TPU filament is available at the lab.

### Options

1. Pololu gearmotors, encoders, driver and tracks, bought (first draft).
2. Lab parts: DC hobby gearmotors with separate encoders and L293D driver chips, and tracks printed in TPU.

### Choice

Option 2:
- 2 × [DAGU DG01D 48:1 hobby gearmotor](https://www.sparkfun.com/hobby-gearmotor-140-rpm-pair.html) with an output shaft on both sides, about 140 RPM without load at a suggested 4.5 V. Each one drives a rear sprocket.
- 2 × Yumo E6A2-CW3C encoders, 200 pulses per turn, open-collector outputs, the same as the Omron E6A2-CW3C ([datasheet](https://www.fa.omron.co.jp/data_pdf/cat/e6a2-c_ds_e_8_2_csm490.pdf)). The second motor shaft turns each encoder through a printed TPU coupling. Each encoder has its own bracket, because its shaft takes at most 10 N of side load.
- 2 × [TI L293DNE](https://www.ti.com/lit/gpn/l293d), two motor channels per chip: one chip for the track motors, one for the linear actuator. The fallback for the track motors is an L298N driver module from the lab.
- Tracks printed in TPU as closed bands with teeth on the inside, on printed PLA sprockets with guide ridges. The front sprocket runs free in a slotted mount, so the track tension can be adjusted after printing.

### Reasons

- All parts are in the lab, so nothing has to be ordered for the drive.
- Printed tracks and sprockets let us set the sprocket diameter in CAD.

### Consequences

- The top speed is the motor speed times the sprocket circumference, for example about 0.26 m/s with a 35 mm sprocket. The firmware caps the speed so that the robot stops within the distance between the edge sensors and the tracks.
- The firmware detects a stalled motor from the encoders and cuts it, to protect the driver chips.
- Test prints in I2: a TPU track section with sprockets, the encoder couplings, and several bore sizes for the plastic motor shafts.

## D007: Edge sensing: two time-of-flight sensors, one ahead of each track

- **Date:** 2026-10-22
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

Edge safety has the highest priority, and M.A.T.S drives to the edge on purpose on every dump cycle (D003). The edge sensors must stop the robot, guide the approach so that it ends square to the edge, and show when the bucket is in place. The first draft proposed four Pololu VL53L4CD time-of-flight sensors, two at the front and two at the rear. Edge sensors are not in the course equipment, so they must be bought through the faculty from RS, Farnell or Digikey.

### Options

1. Sensor type: downward IR reflectance, downward time-of-flight distance, or a mechanical feeler.
2. Placement: four sensors, two at the front and two at the rear (first draft), or two at the front, one ahead of each track.

### Choice

- Two downward VL53L4CD time-of-flight sensors, from one [ST SATEL-VL53L4CD](https://no.rs-online.com/web/p/sensor-development-tools/2410906) kit (RS 241-0906, two boards per kit). They sit at the left and right ends of the brush bracket, one ahead of each track and ahead of the bristles.
- No rear sensors. The robot reverses only along the path it has just driven, and never farther than the encoders measured on the way forward. Before it turns in place near an edge, it backs away until its corners stay on the table during the turn.

### Reasons

- They measure distance, so one threshold works with the brush up or down: the brush lifts only a few millimeters, while the drop at an edge is the full height of the table. If the arm fails and the sensors tilt up, they read a long distance, and the reflex stops the robot.
- With one sensor ahead of each track, each track stops on its own sensor, so the robot ends square to the edge and also sees an edge that it meets at an angle.
- In the dump position, the bucket hangs under the sensors, so they show that the bucket is in place before the push starts.

### Consequences

- The order list goes to the faculty in I1. There is no spare. Until the sensors arrive, the drive and the arms are tested with lab parts.
- The low level checks the edge sensors in every loop iteration before it applies any motor command. A track may move forward only while the sensor ahead of it sees the table.

## D008: Dump mechanism: servo-driven arms and a linear actuator

- **Date:** 2026-10-22
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

The dump cycle needs a brush that can be lowered and raised, a push that moves the pile over the edge, and a bucket that swings from above the head to a position past the table edge and stays upright. The course lists hobby servos, and the lab has a linear actuator.

### Options

For the bucket arms:
1. One micro servo that turns the arm shaft through 1:1 gears.
2. A larger hobby servo from the lab.
3. A gear ratio of up to about 1.4:1 instead of 1:1. The servo turns 180° and the arms swing 118°, so this still covers the swing and gives about 40 % more torque.

### Choice

- **Brush arm:** one central arm at the front of the body, raised and lowered by a [DFRobot SER0006](https://www.dfrobot.com/product-255.html) micro servo. The brush, an RS PRO nylon brush strip (RS 326-9814, bought), slides on a guide rail at the tip of the arm. An [Actuonix PQ12-100-12-P](https://cdn.robotshop.com/media/f/fir/rb-fir-42/pdf/actuonix-pq12-datasheet.pdf) linear actuator (20 mm stroke, 12 V, position feedback) pushes it forward and pulls it back. The rail takes the side loads.
- **Bucket arms:** two arms outside the tracks, fixed to a shaft through the middle of the body, turned by one SER0006 through 1:1 gears (option 1). The arm hubs and the body leave room for the larger servo (option 2), and option 3 stays open.
- **Bucket:** hangs between the arm tips on two ball bearings, with ballast in its bottom to keep it upright in every arm position. It rests on its pivots in open hooks, so a person can lift it off to empty it.

### Reasons

- The actuators are lab equipment. Only the brush strip is bought.
- The fallbacks for a weak servo fit without a new body.
- The rail keeps the side loads off the actuator, so it only pushes along its own axis.

### Consequences

- Bench torque test of the bucket servo in I2. Reach and tipping tests in I4.
- The servos do not report their position, so the push starts only when both edge sensors see the bucket.
- Test how the servos behave without power and without a signal, and give the stowed bucket a mechanical rest that holds it up without the servo.
- The 20 mm stroke must cover the stop gap between the brush and the edge.
- The bearing size is measured when the bearings are in hand, and the bucket pivots are designed around it.

## D009: Display and the seven states

- **Date:** 2026-10-22
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

The face is the main way the robot shows its state (topic C: animatronics). The first draft proposed a bought Adafruit 2.0" ST7789 display and six emotions linked to following a person.

### Options

1. Display: Adafruit 2.0" IPS TFT with ST7789, bought (first draft), or a 2.8" SPI TFT with ILI9341 from the lab.
2. States: six emotions (first draft), or seven states linked to cleaning events.

### Choice

- **Display:** 2.8" SPI TFT module with the ILI9341 controller, 240 × 320 pixels (board V1.2), from the lab. It sits in a fixed head on top of the body.
- **States:** Neutral, Working, Happy, Sad, Surprised, Scared and Angry, drawn as animations. Each one is linked to an event that the robot can sense (Table 1 in the deliverable).

### Reasons

- The display is lab equipment.
- Five of the states are basic emotions in Ekman's theory. Neutral is the rest state, and Working shows effort, which ties the face to the work (D003).

### Consequences

- One shared enum holds the seven states, and the renderer draws the current one.
- Surprised uses the IMU (D010) together with the edge sensors.
- Recognition test with at least 10 people outside the group.
- The datasheet of the display module is not yet linked.

## D010: IMU: SparkFun OpenLog Artemis

- **Date:** 2026-10-22
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

The IMU detects tilt and pick-up. It is a Should part: it triggers the Surprised state, and it can stop the robot when it tilts. The course lists a SparkFun 9DoF IMU breakout (ISM330DHCX and MMC5983MA, Qwiic). The lab gave us a SparkFun OpenLog Artemis.

### Options

1. The SparkFun 9DoF IMU breakout from the course list.
2. The SparkFun OpenLog Artemis from the lab.

### Choice

Option 2: [SparkFun OpenLog Artemis](https://www.sparkfun.com/sparkfun-openlog-artemis.html) with its built-in ICM-20948 IMU. It sends its data over its serial TX pin to the microcontroller.

### Reasons

- It is the IMU that the lab gave us.

### Consequences

- It takes a serial input pin on the microcontroller, which has few free pins (D004).

## D011: CAD files in Git LFS with file locks

- **Date:** 2026-10-24
- **Status:** Accepted
- **Decided by:** Martin

### Context

Martin and Yuki both edit the SolidWorks files. These files are binary, so git cannot merge two versions: if two people edit the same file at the same time, one of them loses their work. Git also stores a full copy of a binary file in every commit, and the pre-commit hook rejects files larger than 3000 KB, which assemblies can exceed.

### Options

1. Plain git, with edits agreed in chat.
2. Git LFS without locks.
3. Git LFS with lockable files.

### Choice

Option 3:
- `.gitattributes` tracks `*.SLDPRT`, `*.SLDASM` and `*.SLDDRW` (upper and lower case) in Git LFS as lockable.
- STL, 3MF and STEP files stay in plain git.
- The structure and the lock workflow are in `cad/README.md`.

### Reasons

- Lockable files are read-only until the user locks them with `git lfs lock`. The lock is held on the GitHub LFS server, and Git LFS stops a push that changes a file locked by another user (`git lfs help config`, `lfs.<url>.locksverify`). A lock request to the GitHub repository succeeded on 2026-10-24.
- Git LFS keeps the full copies out of the git history. The large-file hook skips files tracked by Git LFS (source of `check_added_large_files` in pre-commit-hooks v6.0.0).
- Git for Windows includes Git LFS (version 3.7.1 on Martin's machine), so nothing is installed globally.

### Consequences

- Each clone runs `git lfs install --local` once (`README.md`, `cad/README.md`).
- Lock a file before editing it. Commit and push before unlocking it, because `git lfs unlock` refuses a file with uncommitted changes.
- GitHub limits LFS storage and bandwidth, so commit CAD files at useful points, not after every save.
- STEP and STL files larger than 3000 KB are still blocked by the hook. Adding them to Git LFS needs a new decision.
- The trailing-whitespace and end-of-file hooks skip `cad/`, so they do not rewrite exported STEP files.

## Template

```
## D###: <title>

- **Date:** YYYY-MM-DD
- **Status:** Proposed | Accepted | Superseded by D###
- **Decided by:** <name(s)>

### Context
### Options
### Choice
### Reasons
### Consequences
```
