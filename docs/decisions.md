# Decisions

Numbered record of project decisions (D001, D002, …). Each entry gives the context, the options, the choice and the reasons. To change a decision, add a new entry that supersedes the old one, and set the old entry's status to "Superseded by D###". Update `CLAUDE.md` whenever a decision changes the stack or the hardware.

`docs/part_2/` (the Project 2 deliverable: README, assignment text and poster) was removed on 2026-11-23. Some entries below still cite it. The files are in git history at commit `5519f4a`.

| ID | Date | Decision | Status |
|---|---|---|---|
| D001 | 2026-10-10 | Host-side Python tooling: uv, ruff, mypy, pytest, pre-commit | Accepted |
| D002 | 2026-10-10 | Hydra 1.3 for experiment configuration | Accepted |
| D003 | 2026-10-21 | Concept change: the M.A.T.S desk cleaner replaces the DeskPal desk companion | Accepted; optimization task superseded by D022 |
| D004 | 2026-10-21 | Compute: one Arduino Nano ESP32 runs both layers | Accepted; microcontroller and two-core split superseded by D020 |
| D005 | 2026-10-21 | Power: 12 V bench supply and laptop USB, no battery | Accepted; bench supply superseded by D017, encoder pull-up level by D020, edge-sensor supply by D021 |
| D006 | 2026-10-22 | Drive: DC gearmotors with encoders, L293D driver chips, printed TPU tracks | Accepted; encoder coupling superseded by D018 |
| D007 | 2026-10-22 | Edge sensing: two time-of-flight sensors, one ahead of each track | Accepted |
| D008 | 2026-10-22 | Dump mechanism: servo-driven arms and a linear actuator | Accepted; bucket-arm gears superseded by D019 |
| D009 | 2026-10-22 | Display and the seven states | Accepted; IMU trigger for Surprised superseded by D020 |
| D010 | 2026-10-22 | IMU: SparkFun OpenLog Artemis | Superseded by D020 (IMU dropped for the demo) |
| D011 | 2026-10-24 | CAD files in Git LFS with file locks | Accepted |
| D012 | 2026-10-24 | CAD: module owners and a table of shared dimensions | Accepted |
| D013 | 2026-11-19 | Firmware toolchain: PlatformIO through uv, Arduino-ESP32 core 3.3.12 | Accepted; platform, core and pin numbering superseded by D020 |
| D014 | 2026-11-20 | Step-down converters: LM2596 for the motors, Mateksys BEC12S-PRO for the servos | Accepted; LM2596 use and motor supply superseded by D021 |
| D015 | 2026-11-20 | Wiring diagrams in WireViz are the source of truth for pins and wiring | Accepted; display supply pin superseded by D020 |
| D016 | 2026-11-21 | Cut-off switch: SCI R13-66 rocker, bench supply current limit 2.5 A | Accepted; current limit superseded by D017, worst-case estimate by D021 |
| D017 | 2026-11-21 | Power source: 12 V wall adapter replaces the bench supply | Accepted |
| D018 | 2026-11-21 | Encoder couplings printed in PLA instead of TPU | Accepted |
| D019 | 2026-11-22 | Bucket-arm gears: 20-tooth servo gear and 10-tooth rod gear | Accepted |
| D020 | 2026-11-22 | Microcontroller: Arduino Leonardo replaces the Arduino Nano ESP32 | Accepted |
| D021 | 2026-11-22 | Power rails: track motors on the BEC 5.2 V rail, LM2596 at 3.3 V for the edge sensors | Accepted |
| D022 | 2026-11-22 | Optimization task: squaring up to the table edge replaces the collection rate | Accepted; measure and controller superseded by D025 |
| D023 | 2026-11-22 | Firmware step 2: edge gate, interlocks and the edge approach | Accepted; handling of a hung loop superseded by D024 |
| D024 | 2026-11-22 | Motor watchdog in a timer interrupt and one-try edge-sensor reads | Accepted |
| D025 | 2026-11-22 | Optimization task: how far the brush stops past the table edge | Accepted |

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
- **Status:** Accepted; the optimization task is superseded by D022 (squaring up to the table edge)
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
- **Status:** Accepted; the microcontroller and the two-core split are superseded by D020 (Arduino Leonardo)
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
- **Status:** Accepted; the bench supply is superseded by D017 (12 V wall adapter), the encoder pull-up level by D020 (5 V), and the edge-sensor supply by D021 (LM2596 at 3.3 V)
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
- One adjustable step-down converter feeds the track motors, set so that the motors get about their suggested 4.5 V at full speed. A second converter feeds the servos at 5 V. The converter model is not yet confirmed. See D014.
- The encoders run on 12 V, connected before the cut-off switch. Pull-up resistors to 3.3 V keep their open-collector outputs at the microcontroller's logic level.
- The laptop USB powers the microcontroller, the edge sensors and the display. All grounds are connected.

### Reasons

- The feedback recommended it.
- It removes the open battery issues of the first draft.

### Consequences

- Two cables leave the robot at the rear.
- If the USB cable comes loose, the microcontroller loses power, and pull-down resistors on the driver inputs stop the motors.
- When the converter model is known, check it and the driver chips against the stall currents of the motors and servos. See D014.
- The battery rules in `CLAUDE.md` apply only if a battery is added later.

## D006: Drive: DC gearmotors with encoders, L293D driver chips, printed TPU tracks

- **Date:** 2026-10-22
- **Status:** Accepted; encoder coupling superseded by D018
- **Decided by:** Martin and Yuki

### Context

The robot drives on tracks. The first draft proposed Pololu micro metal gearmotors with magnetic encoders, a TB6612FNG driver and a bought Pololu track set, all ordered from abroad. The feedback asked us to use the course and lab equipment. TPU filament is available at the lab.

### Options

1. Pololu gearmotors, encoders, driver and tracks, bought (first draft).
2. Lab parts: DC hobby gearmotors with separate encoders and L293D driver chips, and tracks printed in TPU.

### Choice

Option 2:
- 2 × [DAGU DG01D 48:1 hobby gearmotor](https://www.sparkfun.com/hobby-gearmotor-140-rpm-pair.html) with an output shaft on both sides, about 140 RPM without load at a suggested 4.5 V. Each one drives a rear sprocket.
- 2 × Yumo E6A2-CW3C encoders, 200 pulses per turn, open-collector outputs, the same as the Omron E6A2-CW3C ([datasheet](https://files.omron.eu/downloads/latest/datasheet/en/e6a2-c_incremental_25-mm-dia._rotary_encoder_datasheet_en.pdf)). The second motor shaft turns each encoder through a printed TPU coupling. Each encoder has its own bracket, because its shaft takes at most 10 N of side load.
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

- The order list goes to the faculty in I1. There is no spare. Until the sensors arrive, the drive and the arms are tested with lab parts. They have been received (`bom.csv`).
- The low level checks the edge sensors in every loop iteration before it applies any motor command. A track may move forward only while the sensor ahead of it sees the table.

## D008: Dump mechanism: servo-driven arms and a linear actuator

- **Date:** 2026-10-22
- **Status:** Accepted; the 1:1 bucket-arm gears are superseded by D019
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
- **Status:** Accepted; the IMU trigger for Surprised is superseded by D020 (IMU dropped)
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
- The module is documented on [LCDwiki (MSP2807)](https://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807), with the [ILI9341 datasheet](https://www.lcdwiki.com/res/MSP2807/ILI9341%20Datasheet.pdf). That it is the same board as the lab's module is not yet confirmed (14 pins, 50 × 86 mm board).
- Confirmed on 2026-11-21: the lab board has 14 pins and measures about 50 × 86 mm, so it is the MSP2807 touch version. The touch controller is not used.

## D010: IMU: SparkFun OpenLog Artemis

- **Date:** 2026-10-22
- **Status:** Superseded by D020 (the IMU is dropped for the demo)
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

## D012: CAD: module owners and a table of shared dimensions

- **Date:** 2026-10-24
- **Status:** Accepted
- **Decided by:** Martin

### Context

Martin and Yuki both make SolidWorks parts (D011). `CLAUDE.md` asks for key dimensions driven by named parameters, and parts in different modules must agree on shared dimensions, such as the bucket shaft diameter and its pivot position. The first plan was one text file of global variables, linked into every part. Martin tested it in SolidWorks 2026 on 2026-10-24, with the text file in a test folder and two parts in its subfolders:
- A value changed in the file updated both parts after a rebuild. Saving the parts did not write their own equations into the file.
- After the folder was copied to another path, the copied parts still linked to the file at the original path, with no warning. In a clone in another folder, the parts would point to the path on Martin's machine.
- A global variable added to the file did not reach the parts on rebuild. It has to be imported again in each part.

The [2025 help](https://help.solidworks.com/2025/english/SolidWorks/sldworks/t_eqn_creating_equations_in_a_text_file.htm) also says not to share one linked file between several models.

### Options

1. One linked text file of global variables for all parts (tested above).
2. Global variables in the top assembly that drive the part dimensions through assembly equations. Not tested.
3. One owner per module, and a table of shared dimensions in `docs/`. Each part holds the values it uses as its own global variables.

### Choice

Option 3:
- Each module folder in `cad/` has one owner, who creates and edits its parts and subassembly. Martin owns `mats.SLDASM`, `layout.SLDPRT` and the STEP exports. The module owners are not decided yet. They are filled in in `cad/README.md` when decided.
- `docs/cad_parameters.md` lists each shared dimension once, with its value, owner, the parts that use it and its source. It also lists the measured masses.
- Each part has global variables with the same names as in the table, and drives its dimensions from them.

### Reasons

- Option 1 breaks without warning in a clone in another folder, and each new variable must be imported by hand in every part.
- With option 2, the top assembly drives parts that the other person owns, so a change in the top assembly needs their locks.
- With one owner per module, two people rarely need the same file. Each module has its own subassembly, so changes stay in the owner's folder.

### Consequences

- A shared value changes in steps: its owner updates the table, commits and pushes it, and tells the other person, and each owner updates their own parts. Until then, the table and the parts can disagree. Interference Detection in the top assembly catches parts that no longer fit.
- The lock workflow of D011 still applies, mainly to `mats.SLDASM` and `layout.SLDPRT`.

## D013: Firmware toolchain

- **Date:** 2026-11-19
- **Status:** Accepted; the platform, the core and the pin numbering are superseded by D020 (Arduino Leonardo)
- **Decided by:** Martin

### Context

The firmware for the Arduino Nano ESP32 (D004) had no toolchain yet. Project 2 is due on 22 November and asks for the robot software with setup and run instructions (`docs/ASSIGNMENT_2.md`). Both team members need the same toolchain and library versions, and `CLAUDE.md` rules out global installs and unpinned dependencies. The pin map is in `electronics/schematics.pdf`, with Arduino pin labels.

### Options

1. **Toolchain:** PlatformIO run through uv, Arduino IDE 2, or Arduino CLI.
2. **Location:** `firmware/` at the root (the layout in `CLAUDE.md`), or `src/firmware/` (the assignment asks for the software in `./src`).
3. **Pin numbering:** the PlatformIO board `arduino_nano_esp32` sets `-DBOARD_HAS_PIN_REMAP`, so the core maps Arduino pin numbers to GPIO numbers. The other option is GPIO numbers everywhere, with `-DBOARD_USES_HW_GPIO_NUMBERS`.
4. **Toolchain storage:** the PlatformIO default `~/.platformio`, or a folder inside the repository.

### Choice

- **Toolchain:**
  - PlatformIO Core 6.2.0 is a uv dev dependency (pinned in `uv.lock`), run as `uv run pio`.
  - The platform is `platformio/espressif32@8.0.0`, the latest release (19 Oct 2026), which ships Arduino-ESP32 core 3.3.12.
  - `firmware/platformio.ini` pins every library to an exact version.
- **Location:** `firmware/` at the root. `README.md` explains this for the assignment's `./src` rule.
- **Pin numbering:** GPIO numbers (`-DBOARD_USES_HW_GPIO_NUMBERS`). `firmware/include/pins.h` uses the `D*`/`A*` names from the schematic and checks each GPIO number with `static_assert` against the [official pinout](https://docs.arduino.cc/resources/pinouts/ABX00083-full-pinout.pdf).
- **C/C++ style:**
  - C++17 for our sources.
  - Formatted by clang-format (`.clang-format`: Google base, 4-space indent, 100 columns).
  - Naming: PascalCase types, snake_case functions and variables, ALL_CAPS constants.
  - clang-format 23.1.5 is pinned in `uv.lock` and runs as a local pre-commit hook.
  - Our sources build with `-Wall -Wextra -Werror`.
- **Storage:** toolchains in `~/.platformio`, treated like the uv cache.

### Reasons

- uv already pins the Python tools, so PlatformIO through uv keeps one install method. `platformio.ini` pins the platform and the libraries, so both clones build the same firmware.
- With the pin remap, the core remaps pin numbers only in its own low-level functions. The `SPI.begin`, `Wire.begin` and `HardwareSerial::begin` class methods are not listed in `io_pin_remap.h`, and libraries that call ESP-IDF directly are not remapped at all. With GPIO numbers, every layer sees the same number.
- The ESP32 toolchains take about 1 GB. Inside the repository folder, their deep paths risk Windows path-length errors.
- The framework compiles with `-std=gnu++2b`, and `build_unflags` does not remove it. So `-std=gnu++17` is set in `build_src_flags`, where it comes last on the command line for the files in `firmware/src/`.

### Consequences

- The commands are in `README.md` and `CLAUDE.md`. The first build downloads the toolchain, so it needs a network connection.
- Each build prints one framework warning from `io_pin_remap.cpp`: "The Arduino API will use GPIO numbers for this build." It confirms the chosen numbering mode.
- The schematic puts parts on pins with a second use, and the firmware works around each one:
  - D0 and D1 are UART0 RX/TX. The firmware sets `MOTOR_ENABLE` (D0) low first in `setup()`, and core logging is off.
  - D12 is the default SPI MISO pin and the IMU input. The display SPI starts without MISO.
  - D8 is the default UART1 TX pin and a track input. Serial1 starts with no TX pin.
- Step 1 of the firmware is a bench console with no edge reflex. The safety layer and the dump cycle follow in step 2, and the state machine with laptop unit tests (`pio test`) in step 3.

## D014: Step-down converters: LM2596 for the motors, Mateksys BEC12S-PRO for the servos

- **Date:** 2026-11-20
- **Status:** Accepted; the LM2596 use and the track motor supply are superseded by D021
- **Decided by:** Martin

### Context

D005 calls for one adjustable step-down converter for the track motors, set so that the motors get about their suggested 4.5 V at full speed, and a second converter for the servos at 5 V. It left the converter model open. The lab has one LM2596 buck converter module and one Mateksys BEC12S-PRO.

### Options

1. The LM2596 feeds the track motors, and the Mateksys feeds the servos.
2. The Mateksys feeds the track motors, and the LM2596 feeds the servos.

### Choice

Option 1:
- An LM2596 buck converter module ([datasheet](https://htckorea.co.kr/image/DK/LM2596_Module_R1.0.pdf)) feeds the track motors through the L293D. Input 4–35 V, at least 1.5 V above the output. Output adjustable from 1.25 V to 30 V, 2 A rated and 3 A at most with a heat sink.
- A [Mateksys BEC12S-PRO](https://www.mateksys.com/?portfolio=bec12s-pro) feeds the two servos at 5.2 V. Input 9–55 V. Output 5.2 V (default), 8 V or 12 V, 5 A continuous and 9 A peak.

### Reasons

- The Mateksys has only three fixed outputs, so it cannot give the motors about 4.5 V. The LM2596 can be set to any voltage in its range.
- 5.2 V is inside the 4.8–6 V operating range of the SER0006 ([DFRobot product page](https://www.dfrobot.com/product-255.html)).
- The SER0006 draws at most 1000 mA when it stalls at 6 V (DFRobot product page). The Mateksys gives 5 A continuously, so both servos can stall at the same time. The same page lists 8000 mA at 4.8 V, which is most likely a typo for 800 mA, since the stall current falls with the voltage.
- The 12 V bench supply is inside the input range of both converters.

### Consequences

- Check the Mateksys output with a multimeter before the servos are connected. 8 V or 12 V would damage them.
- The Mateksys has no protection against a reversed input, so check the polarity of its 12 V input before switching on.
- Set the LM2596 while measuring at the motor terminals at full duty. The L293D drops part of the voltage (SLRS008D, section 6.5), so the converter output reads higher than the motor voltage.
- Neither the DAGU drawing nor the SparkFun page gives the stall current of the DG01D. Measure it, and compare it with the 600 mA continuous limit per L293D channel (SLRS008D, section 6.1) and the 2 A rating of the LM2596. This completes the stall-current check that D005 asks for.

## D015: Wiring diagrams in WireViz are the source of truth for pins and wiring

- **Date:** 2026-11-20
- **Status:** Accepted; the display supply pin (Nano VBUS) is superseded by D020
- **Decided by:** Martin

### Context

`electronics/schematics.pdf` shows the connections with net labels only. It draws no wires and has no power section, so it is hard to trace a signal. `CLAUDE.md` names `electronics/pinout.md` as the single source of truth for pins, but that file was never written. The pins in `firmware/include/pins.h` were taken from `schematics.pdf`.

### Options

1. Write `electronics/pinout.md` as a pin table, and keep `schematics.pdf` as the drawing.
2. Describe every wire in [WireViz](https://github.com/wireviz/WireViz) YAML files in `electronics/wiring/`, render them to SVG, and make these files the source of truth.

### Choice

Option 2:
- WireViz 0.4.1 is a dev dependency in `uv.lock`. It needs Graphviz (`brew install graphviz`).
- Three sheets: `01_signals.yml`, `02_drivers.yml` and `03_power.yml`, each rendered to an SVG next to it with `uv run wireviz -f s electronics/wiring/<sheet>.yml`. The `.yml` and the `.svg` are committed.
- The project skill `.claude/skills/wiring-diagram/SKILL.md` holds the drawing rules and the wire color convention. The robot has no fixed wire colors, so the drawings use that convention.
- The display's J1 jumper is open on the lab board (checked 2026-11-21), so its VCC is drawn on 5 V from Nano VBUS (JP1-12), as the module schematic requires (LCDwiki: J1 open for 5 V, bridged for 3.3 V). VBUS carries the USB-C 5 V directly (Nano ESP32 datasheet 11.4). The LED input stays on 3.3 V.

### Reasons

- The drawing shows every wire from pin to pin, with its color and net name.
- Plain text files: each change shows up in a git diff and can be reviewed.
- No separate pin table to keep in step with the drawing.

### Consequences

- A pin change starts in the sheet, then `pins.h` follows. The skill checks that the two agree.
- `electronics/pinout.md` is not written.
- `schematics.pdf` stays the reference for sheets 02 and 03 until they are drawn. After that, update the source references in `pins.h` and `README.md`.

## D016: Cut-off switch: SCI R13-66 rocker, bench supply current limit 2.5 A

- **Date:** 2026-11-21
- **Status:** Accepted; the 2.5 A current limit is superseded by D017 (the wall adapter has no adjustable limit), and the worst-case estimate by D021
- **Decided by:** Martin

### Context

D005 calls for a switch on the robot that cuts the power to all motors, servos and the linear actuator, but it names no part. Three switches were found in the lab. All three carry AC ratings only:

1. A switch marked 2 A 250 V and 4 A 125 V.
2. An SCI R13-66 rocker, 2 terminals, marked 16 A 125 VAC 1/3 HP, 10 A 250 VAC 1/2 HP, 10(4) A 250 V~ 10E3, 25T85, and 10 A 125 VAC / 6 A 250 VAC.
3. A round rocker marked 6(2) A 250 V~ 1E4 µ, 10 A 125 V~ T105C, T125/55.

Estimated worst-case current through the switch, with every load stalled at the same time:

| Load | Power from 12 V | Source |
|---|---|---|
| 2 servos through the BEC | 2 × 1 A × 5.2 V = 10.4 W, at 88 % efficiency ≈ 11.8 W | Stall current at 6 V ([DFRobot](https://www.dfrobot.com/product-255.html)); efficiency 88–96 % (Matek BEC12S-PRO manual) |
| 2 track motors through the LM2596 | 2 × 0.6 A × about 7.1 V ≈ 8.5 W, at 73 % efficiency ≈ 11.7 W | 600 mA continuous per L293D channel, about 2.6 V drop at 0.6 A ([TI SLRS008D](https://www.ti.com/lit/ds/symlink/l293d.pdf) 6.1, 6.5); 73 % at 12 V in and 3 A (HTC LM2596 datasheet) |
| Linear actuator | 0.21 A × 12 V ≈ 2.5 W | Stall current at 12 V ([Actuonix PQ12](https://cdn.robotshop.com/media/f/fir/rb-fir-42/pdf/actuonix-pq12-datasheet.pdf)) |
| Logic of both L293D chips | 2 × 60 mA × 5.2 V, through the BEC ≈ 0.7 W | ICC1 max (TI SLRS008D 6.5) |
| **Total** | **≈ 27 W, about 2.2 A at 12 V** | |

The motor line assumes the L293D limit, because the DG01D stall current is not measured yet (D014). The encoders connect before the switch (D005) and are not included.

### Options

1. Switch 1.
2. Switch 2, the SCI R13-66 rocker.
3. Switch 3.

### Choice

Option 2: the SCI R13-66 rocker, SW1 on wiring sheet 03, in the + wire after the junction that feeds the encoders. It has two terminals, so either terminal can take the input. The bench supply current limit starts at 2.5 A.

### Reasons

- Switch 2 has the largest rating: 10 A at 250 V AC and 16 A at 125 V AC, about 4–7 times the estimated 2.2 A.
- The R13-66 series is also built in DC versions rated 16 A at 12 V DC ([TME listing](https://www.tme.eu/html/EN/switch-rocker-sci/ramka_1328_EN_pelny.html)). The lab unit carries AC markings only, so a DC rating is not guaranteed for it.
- Switch 1 is rated at most 4 A (AC), less than twice the estimate. Switch 3 has less margin than switch 2.
- 2.5 A stays above the estimated worst case, so it should not limit normal operation, and it caps the current if something shorts.

### Consequences

- Watch the current on the bench supply display during the first tests. If the supply limits the current in normal driving, the estimate is too low: measure the motor stall current (D014) and revisit the limit.
- BOM item 14 and wiring sheet 03 name the part.

## D017: Power source: 12 V wall adapter replaces the bench supply

- **Date:** 2026-11-21
- **Status:** Accepted
- **Decided by:** Martin

### Context

D005 powers the actuators and the encoders from a 12 V bench supply from the lab, and D016 sets its current limit to 2.5 A. The team has a 12 V wall adapter on hand instead, with its output cable cut and split into two wires. Its label reads: Adapter Tech., model ATS036T-W120V; input 100–240 V~ 50–60 Hz, 1 A max; output 12.0 V DC, 3.0 A, 36.0 W; IEC 60950-1; marked LPS, double insulation (Class II), indoor use and efficiency level VI.

### Options

1. Keep the plan: a lab bench supply with a 2.5 A current limit.
2. The 12 V wall adapter.

### Choice

Option 2: the ATS036T-W120V adapter. The + and − wires were identified with a multimeter and labeled.

### Reasons

- It is rated 3.0 A (36 W), above the estimated worst case of about 2.2 A (27 W) in D016.
- It measured 12.27 V with no load (2026-11-21). That is below the encoders' maximum of 13.2 V (5 V −5 % to 12 V +10 %, Omron E6A2-C datasheet) and inside the input range of both converters (LM2596 module 4–35 V, BEC12S-PRO 9–55 V).
- The LPS (Limited Power Source, IEC 60950-1) mark means the output stays limited even with a single fault. For outputs of 30 V or less without an overcurrent device, the limits are a short-circuit current of at most 8 A and at most 100 VA ([Bel Fuse summary](https://www.belfuse.com/resource-library/blog/overview-of-limited-power-source-lps-requirements)). This takes over the protective role of the bench supply's current limit.

### Consequences

- The bench supply in D005 and the 2.5 A current limit in D016 are replaced. The rest of D005 and D016 still applies.
- The LPS limits are well above the 3 A rating. They protect against fire, not against overloading a single part.
- The protection thresholds of the ATS036T-W120V are not published. The datasheet of the sibling model ATS036T-P120 lists over-current protection at up to 170 % and short-circuit protection with automatic recovery, which is not confirmed for this model.
- There is no current display. During the first powered tests, measure the current with a multimeter in series in the + wire, together with the motor stall current (D014).
- BOM item 11, wiring sheet 03 (PS1) and the hardware table in `CLAUDE.md` name the adapter.

## D018: Encoder couplings printed in PLA instead of TPU

- **Date:** 2026-11-21
- **Status:** Accepted
- **Decided by:** Martin

### Context

D006 says that the second motor shaft turns each encoder through a printed TPU coupling. The two couplings on the robot (`cad/body/coupling_drive_motor_encoder.SLDPRT`) are printed in PLA instead. The first print was marked for a redesign in `print/print_params.md`, and the fixed part was committed on 2026-11-19 (commit 3fd1943). BOM item 19 listed TPU for the couplings until 2026-11-21.

### Options

1. A printed TPU coupling, as in D006.
2. A printed PLA coupling.

### Choice

Option 2: two PLA couplings, printed on the Creality Ender-3 V3 SE with 100 % infill and a 0.2 mm layer height (`print/print_params.md`).

### Reasons

- The coupling had to go through a couple of iterations, and since we had no free access to the printer with TPU, the process was faster using PLA. The encoder mount and the motor mount make sure that the two shafts are aligned.

### Consequences

- The TPU encoder coupling in D006 is replaced. The rest of D006 still applies.
- PLA is rigid, so the coupling does not take up an offset between the motor shaft and the encoder shaft. Any offset becomes side load on the encoder shaft, which takes at most 10 N (D006). The encoder mount and the motor mount must keep the shafts in line.
- BOM item 19 and `print/print_params.md` already list PLA for the couplings.

## D019: Bucket-arm gears: 20-tooth servo gear and 10-tooth rod gear

- **Date:** 2026-11-22
- **Status:** Accepted
- **Decided by:** Martin

### Context

D008 turns the bucket-arm rod with one SER0006 micro servo through a pair of 1:1 gears (option 1). It keeps two fallbacks for a weak servo: the larger lab servo (option 2), and a gear ratio of up to about 1.4:1 for more torque (option 3). The arms swing 118° (Deliverable 1, Figure 3), and the servo turns 180° ([DFRobot product page](https://www.dfrobot.com/product-255.html)).

During the CAD work, a test showed that a small gear did not fit on the servo, so the servo gear had to be larger. With 1:1 gears, the rod gear would be just as large, and it would not fit inside the body. Martin changed the gears at that time, but the change was not recorded. The rod gear part `cad/bucket_arms/bucket_arms_gear_rod_reduced.SLDPRT` was added on 2026-11-13 (commit 7eb80b1).

### Options

1. 1:1 gears, with both gears as large as the servo gear must be. The rod gear does not fit inside the body.
2. A gear ratio for more torque, such as 1.4:1 (D008 option 3). The rod gear is then larger than the servo gear, so it fits even less.
3. A 20-tooth servo gear and a 10-tooth rod gear.

### Choice

Option 3:
- Servo gear: 20 teeth, `cad/bucket_arms/bucket_arms_servo_gear.SLDPRT`.
- Rod gear: 10 teeth, `cad/bucket_arms/bucket_arms_gear_rod_reduced.SLDPRT`.
- Both are printed in PLA with 100 % infill (`print/print_params.md`).

### Reasons

- It is the only option where the servo gear fits on the servo and the rod gear fits inside the body.
- The rod turns twice as far as the servo, so the 118° swing needs 59° of servo travel, well inside the 180° range.

### Consequences

- The rod gets at most half of the servo torque, and each degree of the servo moves the arms 2°. D008 option 3 is closed, because more torque through the gears needs a rod gear larger than the servo gear.
- The bench torque test of the bucket servo (D008) matters more. Run it with the bucket, the ballast and a full load before the reach and tipping tests.
- If the micro servo is too weak, the fallback is still the larger lab servo (D008 option 2). Check that it and a gear for it fit in the body.
- A change in arm angle is twice the change in servo angle. The servo end positions are measured on the robot and stored as constants (`firmware/include/config.h`).
- The gears could jump out of mesh in the first fit. A rod mount (`cad/bucket_arms/rod_mount.SLDPRT`, added 2026-11-18) now holds the rod next to its gear (`cad/README.md`).
- Deliverable 1 (text and Figure 3) shows the 1:1 gears and stays as it was handed in. Project 2 describes the change as Iteration 1 in `docs/part_2/README.md`.

## D020: Microcontroller: Arduino Leonardo replaces the Arduino Nano ESP32

- **Date:** 2026-11-22
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

D004 put both control layers on the two cores of an Arduino Nano ESP32. Its pin map used all 22 pins (D0–D13 and A0–A7). When we wired the robot, the Nano did not have enough GPIO pins for our needs, and the wires on its pins made a weak connection (`docs/part_2/README.md`, Iteration 5). Yuki made a new pin map for an Arduino Leonardo from the lab (BOM item 1).

The Leonardo is built around an ATmega32U4 with 5 V logic and one core at 16 MHz:
- Sketches get 28,672 bytes of flash and 2,560 bytes of RAM (`leonardo.upload.maximum_size` and `maximum_data_size` in [ArduinoCore-avr boards.txt](https://github.com/arduino/ArduinoCore-avr/blob/master/boards.txt)).
- The 3V3 pin gives at most 50 mA, and each I/O pin at most 20 mA ([A000057 full pinout](https://docs.arduino.cc/resources/pinouts/A000057-full-pinout.pdf)).
- Hardware SPI is only on the ICSP header: pin 1 CIPO, 3 SCK, 4 COPI (A000057 full pinout).
- The ATmega32U4 reads 3.3 V as high. Its VIH minimum is 0.2 VCC + 0.9 V, which is 1.9 V at 5 V ([ATmega32U4 datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-7766-8-bit-avr-atmega16u4-32u4_datasheet.pdf) 7766J, table 29-1).

The edge sensors and the display are 3.3 V parts:
- **VL53L4CD:** AVDD 2.6–3.5 V. SCL, SDA, XSHUT and GPIO1 take at most 3.6 V (DS13812 Rev 8, tables 11 and 12, `electronics/DS_vl53l4cd.pdf`).
- **SATEL-VL53L4CD board:** it has no regulator and no level shifter. VDD goes straight to the sensor, and the 10 kΩ pull-ups on XSHUT and GPIO1 and the 2.2 kΩ I2C pull-ups (R1 to SDA through solder bridge SB1/SB2, R2 to SCL through SB3/SB4) all go to VDD ([DB4629](https://docs.rs-online.com/762a/A700000008746312.pdf), figure 5).
- **MSP2807 display:** "Logic IO port voltage 3.3V(TTL)" ([LCDwiki](https://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807)).
  - The module schematic connects CS, RESET, DC, SDI and SCK straight to the panel. Its XC6206 regulates only VCC.
  - LCDwiki's Arduino page says the pins "can only input a 3.3V high level" and recommends an external level shifter ([Run Arduino Demo in SPI model](https://www.lcdwiki.com/Run_Arduino_Demo_in_spi_model)).

### Options

1. Keep the Nano ESP32.
2. The Arduino Leonardo from the lab, with Yuki's pin map.

### Choice

Option 2. The pin map:

| Leonardo pin | Net | Connects to | Note |
|---|---|---|---|
| D0 | ENC_L_A | J12 phase A | INT2. Also Serial1 RX |
| D1 | ENC_R_A | J13 phase A | INT3. Also Serial1 TX |
| D2 | SDA | J7, J8 SDA | Hardware I2C |
| D3 | SCL | J7, J8 SCL | Hardware I2C |
| D4 | TOF_R_XSHUT | J8 XSHUT | Open-drain. Swapped, see below |
| D5 | TOF_L_XSHUT | J7 XSHUT | Open-drain. Swapped, see below |
| D6 | LEFT_IN1 | U2 pin 2 (1A) | PWM, Timer4 |
| D7 | LEFT_IN2 | U2 pin 7 (2A) | No PWM |
| D8 | ACT_IN1 | U3 pin 2 (1A) | No PWM |
| D9 | RIGHT_IN2 | U2 pin 15 (4A) | Timer1, taken by the Servo library |
| D10 | ACT_IN2 | U3 pin 7 (2A) | Timer1, taken by the Servo library |
| D11 | RIGHT_IN1 | U2 pin 10 (3A) | PWM, Timer0 |
| D12 | MOTOR_ENABLE | R1, then U2 pins 1 and 9, U3 pin 1 | |
| D13 | – | Built-in LED | Free |
| A0 | ACT_FEEDBACK | J6 pin 5 (pot wiper) | |
| A1 | BUTTON_ADC | Button ladder | |
| A2 | TFT_DC | J9 DC/RS, through a divider | |
| A3 | TFT_CS | J9 CS, through a divider | |
| A4 | BRUSH_SERVO_PWM | J10 signal | |
| A5 | BUCKET_SERVO_PWM | J11 signal | |
| ICSP-4 (COPI) | TFT_MOSI | J9 SDI/MOSI, through a divider | Hardware SPI |
| ICSP-3 (SCK) | TFT_SCK | J9 SCK, through a divider | Hardware SPI |
| ICSP-1 (CIPO) | – | Not connected | Free |

Sources for the pin functions:
- Interrupts and PWM pins: A000057 full pinout.
- The timer of each PWM pin: `digital_pin_to_timer_PGM` in [pins_arduino.h (leonardo)](https://github.com/arduino/ArduinoCore-avr/blob/master/variants/leonardo/pins_arduino.h).
- The Servo library uses Timer1 on the ATmega32U4 ([ServoTimers.h](https://github.com/arduino-libraries/Servo/blob/master/src/avr/ServoTimers.h)) and puts it in normal counting mode (`TCCR1A = 0` in [Servo.cpp](https://github.com/arduino-libraries/Servo/blob/master/src/avr/Servo.cpp)). D9 and D10 then have no PWM.

Yuki's list names the right-motor inputs IN3 (D11) and IN4 (D9), after the L293D channels. The net names stay RIGHT_IN1 and RIGHT_IN2, as on the wiring sheets.

XSHUT (updated 2026-11-23): the first version of this table had D4 on the left sensor and D5 on the right. The robot is wired the other way, found on the bench on 2026-11-22. The table, wiring sheet 01 and `firmware/include/pins.h` follow the robot.

Rules for the 3.3 V parts and other changes:
- **Display:**
  - CS, DC, MOSI and SCK each get a resistor divider: 1 kΩ in series and 2 kΩ to GND (R10–R17, wiring sheet 01).
  - The ILI9341 accepts a high level from 0.7 VDDI to VDDI, and at most VDDI + 0.3 V (ILI9341 V1.09, section 18). With VDDI = 3.3 V:
    - At 5.0 V the divider gives 3.33 V. That is inside the absolute maximum, but 0.03 V above the recommended high level.
    - At the 32U4 VOH minimum of 4.2 V it gives 2.8 V, which is still above the 2.31 V needed.
  - RESET keeps its 10 kΩ pull-up (R3), now to the Leonardo 3V3 pin. The LED input also takes 3.3 V from that pin.
  - VCC comes from the Leonardo 5V pin, with J1 open.
- **XSHUT:** the firmware drives it open-drain.
  - Shutdown: output low. Release: input with the port bit low, so the board's 10 kΩ pull-up raises it to 3.3 V.
  - The library drives XSHUT as a push-pull output (`pinMode(xshut, OUTPUT)` and `digitalWrite(xshut, HIGH)` in `vl53l4cd_class.h`, STM32duino VL53L4CD 1.0.5), so the firmware does not give it the XSHUT pin.
  - On AVR, an input with its port bit high has the internal pull-up on (the `INPUT_PULLUP` branch of `pinMode()` in [wiring_digital.c](https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/wiring_digital.c)), so the port bit must stay low.
- **I2C:** `twi_init()` turns on the internal pull-ups to 5 V ([twi.c](https://github.com/arduino/ArduinoCore-avr/blob/master/libraries/Wire/src/utility/twi.c)). The firmware turns them off after `Wire.begin()`, so only the 2.2 kΩ pull-ups to 3.3 V on the sensor boards remain.
- **Edge-sensor supply:** 3.3 V from the LM2596 (D021). The two sensors draw 2 × 24 mA on average while ranging, with peaks up to 40 mA each (DS13812 table 14). That is above the 50 mA of the Leonardo 3V3 pin.
- **Actuator pot:** its reference rail (J6 pin 4) moves from 3.3 V to the Leonardo 5V pin.
  - The PQ12 takes "any stable low and high reference voltage" on pins 1 and 4 ([PQ12 datasheet](https://cdn.robotshop.com/media/f/fir/rb-fir-42/pdf/actuonix-pq12-datasheet.pdf)).
  - The ADC references AVCC, so the reading covers the full range.
- **Encoders:** the internal pull-up holds phase A at 5 V (20–50 kΩ, ATmega32U4 datasheet table 29-1).
- **IMU: dropped for the demo.**
  - Its serial input would be Serial1 RX on D0, which now reads the left encoder.
  - Surprised is shown when both edge sensors see no surface at once.
- **Control:** both layers run in one fixed-rate loop on the single core.

### Reasons

- The map fits every part except the IMU. It uses 22 of the 23 signal pins (D0–D13, A0–A5 and three pins on the ICSP header), so D13 and ICSP-1 are free.
- The hardware SPI sits on the ICSP header, so the display takes no SPI pins from the side headers.
- It is lab equipment.

### Consequences

- **D004:** the microcontroller and the two-core split are replaced. One processor remains, so there is still no inter-board protocol.
- **D009 and D010:** the IMU is dropped (D010 superseded), and Surprised uses the edge sensors only. BOM item 9 has the status `optional`.
- **D013:** the ESP32 platform, the core and the GPIO pin numbering are replaced. The AVR platform version is chosen and pinned in the firmware port.
- **D015:** the display VCC moves from Nano VBUS to the Leonardo 5V pin.
- **Firmware port (separate task).** Until it is done, `firmware/include/pins.h` and `firmware/platformio.ini` still describe the Nano ESP32 and do not match the wiring sheets. Do not flash the robot until then. The checklist:
  - `platformio.ini`: platform `atmelavr` at an exact version, board `leonardo`, the ESP32 flags removed.
  - `pins.h`: the new map, with `static_assert` checks against the Leonardo variant numbers (A0–A5 are D18–D23).
  - Tracks: PWM on D6 (left) and D11 (right) only. Reverse is IN2 high and IN1 at PWM (1 − duty). Timer4 and Timer0 start at different PWM frequencies.
  - Tracks: read the 32-bit encoder counts with interrupts off, and remove `IRAM_ATTR`.
  - Servos: the Arduino Servo library, pinned in `platformio.ini`.
  - Actuator: `digitalWrite` on D8 and D10, and `analogRead` instead of `analogReadMilliVolts`.
  - Edge sensors: XSHUT open-drain in our own code, and the I2C pull-ups off.
  - Display: default SPI pins.
  - Display drawing: a full screen is 240 × 320 × 16 bits, and the fastest SPI clock is fosc/2 = 8 MHz (ATmega32U4 datasheet, table 17-5). So it takes at least 154 ms, and the edge check would wait that long. Draw the face in small pieces between loop passes.
  - Console: the AVR core has no `Serial.printf`.
  - Remove `imu_link`.
  - Check the flash and RAM use that `pio run` prints against 28,672 and 2,560 bytes.
- **Bench checklist:**
  - The check that the motors stay off through reset now measures D12. The port pins are tri-stated during reset (ATmega32U4 datasheet, section 10.2.1), so R2 holds MOTOR_ENABLE low.
  - The D0/D1 boot UART note of the ESP32-S3 no longer applies.
- **Open items:**
  - I2C pull-ups: in Martin's photo of 2026-11-22, one board has 0 Ω links fitted at SB1 and SB3, so R1 and R2 connect to SDA and SCL. To confirm on both boards with the power off, measure about 2.2 kΩ from VDD to SDA and from VDD to SCL. With both boards on the bus, each line has 1.1 kΩ to 3.3 V, so a low level sinks 3 mA. The VL53L4CD VOL is specified at 4 mA (DS13812 table 15).
  - Were the display or the sensors powered while wired straight to the Leonardo? If so, test them for damage.
  - Which resistor values does the lab have for the dividers?
- `CLAUDE.md`, BOM items 1, 8 and 9, the wiring sheets and `.claude/skills/wiring-diagram/SKILL.md` name the Leonardo.

## D021: Power rails: track motors on the BEC 5.2 V rail, LM2596 at 3.3 V for the edge sensors

- **Date:** 2026-11-22
- **Status:** Accepted
- **Decided by:** Martin and Yuki

### Context

- D014 feeds the track motors from the LM2596, set for about 4.5 V at the motors, and the servos from the BEC12S-PRO at 5.2 V.
- With the Leonardo (D020), the edge sensors need a 3.3 V supply that gives more than the 50 mA of the Leonardo 3V3 pin.
- The team ran the track motors on 5 V through the L293D, and they worked (reported on 2026-11-22).

### Options

1. Keep D014, and add a new 3.3 V regulator for the edge sensors.
2. Feed the track motors from the BEC 5.2 V rail, and set the LM2596 to 3.3 V for the edge sensors.

### Choice

Option 2:
- **Track motors:** U2 VCC2 takes +5V_SERVO, the BEC output. That rail already feeds the servos and VCC1 of both L293D chips. VM_TRACK is removed.
- **LM2596:** set to 3.3 V, PS2 on wiring sheet 03.
  - Its input is +12V_UNSWITCHED, before the cut-off switch, like the encoders.
  - Its output, +3V3_TOF, feeds only VDD of both edge sensors.
- **Leonardo 3V3 pin (+3V3_LOGIC):** keeps the small display loads (D020).
- The two 3.3 V nets are never joined.

### Reasons

- No new part is needed.
- The L293D allows VCC2 from VCC1 to 36 V ([TI SLRS008D](https://www.ti.com/lit/ds/symlink/l293d.pdf)), so VCC2 = VCC1 = 5.2 V is allowed.
- 3.3 V is inside the 2.6–3.5 V supply range of the VL53L4CD (DS13812 table 12). 12 V is more than 1.5 V above the LM2596 output (HTC LM2596 datasheet).
- Before the cut-off switch, the edge sensors stay powered when the switch cuts the motors. The firmware does not have to restart them, and the robot keeps seeing the edge.
- The BEC gives 5 A continuously. The worst case with every load stalled at once, in the format of D016:

| Load | Power | Source |
|---|---|---|
| 2 servos | 2 × 1 A × 5.2 V = 10.4 W | Stall current at 6 V ([DFRobot](https://www.dfrobot.com/product-255.html)) |
| 2 track motors | 2 × 0.6 A × 5.2 V ≈ 6.2 W | 600 mA continuous per L293D channel (TI SLRS008D 6.1) |
| Logic of both L293D chips | 2 × 60 mA × 5.2 V ≈ 0.6 W | ICC1 max (TI SLRS008D 6.5) |
| **BEC output** | **3.32 A, about 17.3 W; at 88 % efficiency about 19.6 W from 12 V** | Efficiency 88–96 % (Matek BEC12S-PRO manual) |
| Linear actuator | 0.21 A × 12 V ≈ 2.5 W | Stall current at 12 V ([Actuonix PQ12](https://cdn.robotshop.com/media/f/fir/rb-fir-42/pdf/actuonix-pq12-datasheet.pdf)) |
| **Total through SW1** | **≈ 22 W, about 1.8 A at 12 V** | |
| Edge sensors through the LM2596, before SW1 | 2 × 40 mA × 3.3 V ≈ 0.26 W at the output | Peak current (DS13812 table 14). The HTC datasheet gives no efficiency at this light load |

The motor line still uses the L293D limit, because the DG01D stall current is not measured yet (D014).

### Consequences

- **D014:** the LM2596 no longer feeds the motors. Its notes about setting it at the motor terminals no longer apply. The BEC part of D014 stands.
- **D016:** the worst-case current through SW1 drops from about 2.2 A to about 1.8 A, so the switch and the adapter keep more margin.
- **Set the LM2596 to 3.3 V and measure its output before the edge sensors are connected.** It was set for the motors, above 5.2 V (sheet 03), and that would destroy the sensors (3.6 V absolute maximum, DS13812 table 11).
- **Motor speed:** the motors get 5.2 V minus the L293D drop. That is less than their suggested 4.5 V at high current (about 2.6 V drop at 0.6 A, D016). Measure the top speed and the stopping distance in the edge tests.
- **Shared rail:** motors and servos now share the BEC rail. Motor starts and stalls can pull the rail down and make the servos twitch. Measure the rail during the first powered tests, together with the motor stall current (D014).
- **Power order: USB in first, and the 12 V adapter out first.** The edge sensors stay powered when only the USB cable is unplugged. Their pull-ups (2.2 kΩ on SDA and SCL, 10 kΩ on XSHUT) then drive current into the pins of the unpowered ATmega32U4, above its VCC + 0.5 V limit (datasheet 7766J, section 29.1).
- `CLAUDE.md`, BOM items 12 and 13, and wiring sheets 01–03 name the new rails.

## D022: Optimization task: squaring up to the table edge replaces the collection rate

- **Date:** 2026-11-22
- **Status:** Accepted; the measure and the controller are superseded by D025
- **Decided by:** Martin and Yuki

### Context

D003 made the collection rate the optimization task: the share of a weighed amount of spread debris, by mass, that ends up in the bucket. The target was at least 80 % by mass (`CLAUDE.md`, Should). Measuring it needs the full dump cycle to work: lower the brush, drive to the edge, square up, lower the bucket, push, stow and back away.

The switch to the Arduino Leonardo (D020) set the firmware back. The pin map, the power rails (D021) and the firmware all had to change, and the bench firmware had to be ported before any part of the dump cycle could run. The bench firmware for the Leonardo was finished on 2026-11-22, and the dump cycle is not written yet. Part 2 asks us to optimize the controller for a certain task (`docs/part_2/ASSIGNMENT.md`, section 1), and the demo is on 30 November.

### Options

1. Keep the collection rate (D003). It needs the full dump cycle to work reliably, and many runs with weighed debris.
2. Squaring up to the table edge: the robot drives to the edge at an angle and ends square to it. It is the first part of every dump cycle, and the Part 2 GIF shows it.
3. Stopping distance at the edge versus speed. It is already planned as a safety measurement (D003), and it leaves little to tune beyond the speed.

### Choice

Option 2: we tune the approach duty of the tracks for squaring up to the table edge.
- **Measured in each run:**
  - the gap between the front of each track and the table edge, measured with a ruler
  - the final angle to the edge, from the difference between the two gaps
  - the time from the start until both tracks have stopped
- **Tuned:** the approach duty, within the speed cap in the firmware. Each duty is tested from several approach angles, with several runs each.
- **Pass:** both gaps stay above zero in every run. Among the duties that pass, the best one gives the smallest final angle, and the shorter time breaks a tie.

### Reasons

- It needs only the first part of the dump cycle, which step 2 of the firmware builds (`README.md`, Firmware). It fits in the time left before the demo.
- Every dump cycle starts with it. D007 already asks the edge sensors to guide the approach so that the robot ends square to the edge.
- It can be measured with a ruler and the serial console. It needs no weighing and no test debris.
- It has a clear reality gap. In CAD, each sensor point is 63.25 mm ahead of the front of its track, and the robot turns about the middle of the stopped track. On the real robot:
  - The brush-arm mount is off center, so the two sensor points are not the same distance ahead of the tracks (`docs/part_2/README.md`, section 6).
  - Each sensor sees a cone, not a line: the detection volume is 22° at 100 mm ([VL53L4CD datasheet DS13812](https://www.st.com/resource/en/datasheet/vl53l4cd.pdf), field of view). The stop can come later than the point straight below the sensor.
  - The tracks slip, so the turn point moves.
- We prefer to report a smaller task that we can finish and measure over a larger one that we cannot finish before the demo.

### Consequences

- **D003:** the collection rate is no longer the optimization task. It becomes future work.
- **`CLAUDE.md`:** the Should goals, the MVP note and the list of experiments name the new task. The collection rate moves to the Could goals.
- **Firmware, step 2:**
  - A console command sets the approach duty without reflashing, limited by the speed cap.
  - Each run prints one result line: the time, the encoder counts and which side stopped first.
  - The edge stop and the dump interlock apply in every run. The optimization never overrides them (`CLAUDE.md`, priority order).
- **Experiments:**
  - `docs/experiments/001-edge-calibration.md` comes first. The edge threshold and the stopping distance set the safe range of duties.
  - `docs/experiments/002-square-up.md` holds the setup, the raw data and the result of the optimization.
- **Final angle:** angle = atan((gap_left − gap_right) / b), where b is the distance between the two points where the gaps are measured.
- **Mount offset:** until the brush-arm mount is fixed, part of the final angle comes from the mount and not from the controller. Measure how far each sensor point sits ahead of its track on the real robot before the runs, so this offset can be separated from the result.
- **Part 2 README and poster:** section 1 and the poster describe the new task, its results and the limits that the switch to the Leonardo put on it. Section 6 names the collection rate as future work.

## D023: Firmware step 2: edge gate, interlocks and the edge approach

- **Date:** 2026-11-22
- **Status:** Accepted; the handling of a hung loop is superseded by D024 (motor watchdog and one-try sensor reads)
- **Decided by:** Martin

### Context

Step 1 of the Leonardo firmware is a bench console with no edge reflex (`README.md`, Firmware). The optimization task (D022) and the Part 2 GIF need the robot to drive to a table edge and square up to it. The GIF shows only the approach and the square-up, with the brush down and its servo off. `CLAUDE.md` asks the low level for:
- an edge check in every loop iteration, before any motor command is applied
- a stop on invalid sensor readings, and when commands stop arriving
- a speed cap in the firmware
- forward motion of a track only while the sensor ahead of it sees the table
- reverse only along the path just driven, never farther than the encoders measured
- the dump interlock

Measured on the robot on 2026-11-22:
- `tof` with the brush down and its servo off: about 50 mm on the table, and more than 100 mm past the edge.
- The slowest duty that moves the robot reliably on the table with the brush down: 50 %.
- The direction flags in `firmware/include/config.h` are correct.
- From CAD: each sensor point (where the axis of the sensor meets the table) is 63.25 mm ahead of the front of its track. The track is 130.92 mm long, so the point is 128.71 mm ahead of the middle of the track.

On the ATmega32U4, the Caterina bootloader and the 1200-baud reset of the core both use the watchdog timer. Reports say that a sketch that turns the watchdog on can block USB uploads ([Arduino forum](https://forum.arduino.cc/t/atmega32u4-bootloader/386871), [arduino/Arduino#4492](https://github.com/arduino/Arduino/issues/4492)).

### Options

**Stop when commands stop arriving:**
1. The hardware watchdog (WDT), which resets the board when the loop stops.
2. A command lease and timeouts in software: every track command runs for a limited time, and the edge approach must renew it in every tick.

**Reverse:**
1. Allow reverse up to the encoder counts driven forward.
2. Refuse reverse in step 2.

### Choice

Option 2 in both cases. The firmware works like this:

- **Control tick:** every 5 ms (`LOOP_PERIOD_MS`, matching the sensor poll period), in this order: edge sensors, edge approach, track gate and timers, actuator timer. The console runs in every pass of the loop, and its commands go through the same checks.
- **Safety checks** (`firmware/src/safety.{h,cpp}`). The track, servo and actuator modules call them before they apply a command:
  - **Sees the table:** the sensor is running, its last reading is at most 150 ms old (`EDGE_STALE_MS`, three timing budgets), the range status is 0, and the distance is at most 75 mm (`EDGE_TABLE_MAX_MM`). Every other case counts as "no table".
  - **Edge gate:** `tracks::run()` refuses forward duty on a track whose sensor does not see the table, and `tracks::update()` stops a running track as soon as its sensor stops seeing it.
  - **Speed cap:** every track command is limited to ±70 % (`TRACK_MAX_DUTY_PCT`).
  - **Reverse:** refused.
  - **Bucket interlock:** the tracks are locked while the bucket servo gets pulses. The bucket servo gets pulses only while both tracks are stopped and neither sensor sees the table.
  - **Actuator interlock:** the tracks are locked while the actuator moves, and the actuator moves only while both tracks are stopped.
- **Edge approach** (`firmware/src/edge_approach.{h,cpp}`), started with `approach [duty]`. The default duty is 50 % (`APPROACH_DEFAULT_DUTY_PCT`).
  1. The display shows "Working", then the approach waits for a fresh reading from each sensor.
  2. Both tracks drive forward. Each track stops for the rest of the run when its sensor stops seeing the table, so the robot turns about the first track that stops.
  3. The run ends when both tracks have stopped. It aborts on the first of these:
     - 8 s from the start (`APPROACH_MAX_MS`)
     - 3 s after the first track stopped (`SQUARE_MAX_MS`)
     - a sensor that stops delivering readings
     - the interlock
     - `stop` or `en 0`
  4. The approach renews each track command with a run time of 50 ms (`TRACK_LEASE_MS`) in every tick.
  5. At the end, the console prints `result,<outcome>,<duty_pct>,<first_side>,<t_first_ms>,<t_total_ms>,<enc_l>,<enc_r>`, and the display shows "Happy" (done), "Scared" (aborted) or "Neutral" (stopped).
- **The shared states:** `firmware/include/emotion.h` holds the seven states of D009. Step 2 draws three of them as text.

### Reasons

- **The gate sits in the track module,** so every caller passes through it, and the console cannot drive around it.
- **The threshold is halfway** between the two measured values, 25 mm from each.
- **Three timing budgets** let one late result pass without a stop, while a sensor that stops delivering stops the robot within 150 ms.
- **A sensor fault aborts the run** instead of counting as an edge. A fault also reads as "no table", and counting it as an edge would give a false "done".
- **Reverse:** there are no rear sensors, and the rule "only along the path just driven" belongs to the back-away step, which is not written yet. The optimization runs do not need reverse, since the robot is put back by hand.
- **No hardware watchdog:** it can block USB uploads on this board (Context). The lease stops the tracks when the edge approach stops renewing its commands.
- **The display draws only while the tracks are stopped.** A full-screen draw blocks the loop for at least 154 ms (D020), and the robot would drive without an edge check in that time.

### Consequences

- **Reaction time:** a new reading arrives about every 50 ms (`TOF_TIMING_BUDGET_MS`), and the tick adds up to 5 ms. The track then still has to stop. Experiment 001 measures the resulting gap at the edge.
- **Margin:** of the 63.25 mm, the square-up turn uses d·(1 − cos α), with d = 128.71 mm. That is 7.8 mm at an approach angle of 20°. The rest must cover the stopping distance and the late trigger caused by the cone of the sensor (D022).
- **A loop that hangs is not caught in software.** The I2C timeout and the address probe in `edge_sensors.cpp` keep the sensor bus from hanging it. The cut-off switch and the pull-downs on the driver inputs remain.
- **Brush position:** the threshold holds for the brush down with its servo off. With the brush raised, the sensors tilt (`docs/part_2/README.md`, section 6). That case is not tested.
- **Interlocks use what is measured.**
  - The bucket check uses "pulses or none", because the stowed pulse is not measured.
  - The actuator check cannot tell whether the brush is back, because the feedback value at the retracted end is not measured. Retract the actuator fully before a run.
- **Flash:** step 2 uses 28,410 of 28,672 bytes, so 262 bytes are left. RAM: 1,047 of 2,560 bytes. The rest of the dump cycle needs space to be freed first, for example by removing the display test pattern or shortening console text.
- **Bench tests now need a surface** within 75 mm under both sensors, or the gate refuses forward commands.
- **D020:** the firmware port is done (step 1, 2026-11-22), so the note not to flash the robot no longer applies.
- **Next:** `README.md` describes the commands and the test ladder. `docs/experiments/001-edge-calibration.md` checks the threshold, the noise and the gap at the edge for each duty.

## D024: Motor watchdog in a timer interrupt and one-try edge-sensor reads

- **Date:** 2026-11-22
- **Status:** Accepted
- **Decided by:** Martin

### Context

The step 2 firmware (D023) failed on the robot on 2026-11-22:
- `approach` on the table kept driving after the sensors had passed the edge.
- On the bench, with the tracks off the ground, `drive B 60 2000` kept driving after the board was pulled away from a sensor.
- `drive B 50 500` kept driving after 500 ms, and the console did not answer `status` or `stop`.

The control loop had stopped while the motors ran, and the motors kept their last PWM. Only the cut-off switch could stop them.

The likely cause is in the sensor library: its register read repeats until a transfer succeeds (`VL53L4CD_I2CRead` in `platform.cpp`), so a stuck I2C bus hangs the loop for good. Motor noise on the I2C wires could make a sensor miss a clock edge and hold the bus. A second possibility is noise on the encoder wires, which have only the weak internal pull-ups, firing their interrupts so often that the processor does nothing else. The cause is not confirmed. D023 left 262 bytes of flash free.

### Options

1. A motor watchdog in a timer interrupt, outside the main loop.
2. Poll the sensors with our own one-try reads instead of the library calls.
3. Less noise, in hardware: a 100 nF capacitor across each track motor, sensor and encoder wires away from the motor wires, and I2C at 100 kHz as a test.
4. Our own reads for the sensor start-up as well, or a patched copy of the library.

### Choice

Options 1 and 2. Option 3 is up to the team.

- **Motor watchdog** (`firmware/src/safety.{h,cpp}`):
  - It runs in the Timer0 compare-B interrupt, once per Timer0 cycle (16 MHz / 64 / 256 = 1.024 ms).
  - It is armed while a track or the actuator is commanded. Every track and actuator command and every control tick kicks it, and a stop disarms it.
  - After 50 cycles (about 51 ms, `WATCHDOG_MS`) without a kick, it pulls MOTOR_ENABLE low. Both L293D chips turn off, and the motors coast to a stop.
  - The next control tick prints `watchdog: loop stalled, drivers off`, ends any approach run, and stops and disables the modules so they match the pin.
- **One-try sensor reads** (`firmware/src/edge_sensors.cpp`):
  - The polling reads the same registers as `VL53L4CD_CheckForDataReady()`, `VL53L4CD_ClearInterrupt()` and `VL53L4CD_GetResult()` (`vl53l4cd_api.cpp`): `0x0031` for data ready, `0x0086` to clear the interrupt, and `0x0089`, `0x008C` and `0x0096` for the range status, the SPAD count and the distance.
  - The status mapping is copied from `GetResult()`. The interrupt polarity (`0x0030`) is read once after the start.
  - Each transfer gets one try, limited by the 25 ms Wire timeout. A failed transfer marks the sensor `failed` until the next reset, so the edge gate stops the tracks and the console keeps working.
  - The library still starts the sensors at boot, while the motors are off.
- **`approach` is refused while a track is moving.** Found while adding the watchdog: the "Working" draw would have blocked the edge check while a `drive` command ran.
- **To make room,** the `status` command and the display test pattern (`face test`) are removed. Trial builds measured `status` at 432 bytes and the test pattern at 56 bytes.

### Reasons

- **The watchdog works outside the main loop,** so it stops the motors whatever hangs the loop, as long as its interrupt runs.
  - Timer0 already runs for `millis()`, and the core uses only its overflow interrupt (`wiring.c`).
  - OC0B stays disconnected (COM0B1:0 = 0), so D3 remains I2C SCL.
  - The Servo library uses only Timer1 on the ATmega32U4 (`ServoTimers.h`).
  - The hardware watchdog is still ruled out (D023).
- **Arming only while something moves** keeps the display draws, which block for at least 154 ms, from tripping it. They happen only while everything is stopped.
- **One-try reads** turn a bus fault into a stop with a working console, so the fault shows in `tof` and can be logged.
- **`stream` is kept over `status`:** it logs nearly the same values over time, which the experiments need.

### Consequences

- **The test that tells the causes apart is still open:** unplug the encoder signal wires and run `drive B 50 500` on the bench. If the cause is a flood of encoder interrupts, the timer interrupt may be held back as well, and the watchdog may not help.
- **A trip coasts the motors** instead of braking them, so the robot stops later than at the edge gate. The watchdog is a backstop, not the normal stop.
- **A bus fault still ends the run,** and the sensor stays `failed` until a reset. If the motors cause a bus fault on every run, the approach cannot finish, and option 3 is needed.
- **A hang during the sensor start-up at boot is not covered,** but the motors are off then.
- **Anything that blocks the console for 51 ms while a track runs,** such as `face <text>`, trips the watchdog. That is intended, and the test ladder uses it.
- **Flash:** 28,326 of 28,672 bytes (346 free). RAM: 1,011 of 2,560 bytes.
- **D023:** its consequence "A loop that hangs is not caught in software" no longer applies.

## D025: Optimization task: how far the brush stops past the table edge

- **Date:** 2026-11-22
- **Status:** Accepted
- **Decided by:** Martin

### Context

D022 made squaring up to the table edge the optimization task, judged by the final angle to the edge. The runs on 2026-11-22 used the edge test sketch in `firmware/yuki_src/edge_detection_test.ino`, not the step 2 firmware in `firmware/src/` (D023). Claude Code wrote the sketch, and we reviewed, tuned and revised it on the robot.

Update 2026-11-23: the sketch was removed, because it does not match the structure of `firmware/`. It was never committed, so the baseline below cannot be rerun from the repository. `firmware/src/` (D023, D024) is the code going forward. Its `approach` stops each track on its own and caps the duty at 70 % (`TRACK_MAX_DUTY_PCT`), so duties above 70 % need experiment 001 first.

The sketch worked like this:
- **Calibration (`c`):** each edge sensor takes 8 readings of the table, and their average is the baseline. The sketch refuses a baseline outside 20–180 mm or a spread of more than 20 mm.
- **Threshold:** the baseline plus 35 mm, or plus half the baseline if that is larger (`MIN_EDGE_INCREASE_MM`).
- **Drive (`g`):** both tracks drive forward at full duty. The sketch sets the enable pin of the driver high, with no PWM. When either sensor reading passes its threshold, both tracks stop. They also stop on an invalid reading, when a sensor has not delivered a reading for 180 ms, and after 2.5 s.
- **Sensors:** the Pololu VL53L4CD library 1.0.0, with a 20 ms timing budget.

Both tracks stop together, so the robot does not turn to square up, and the final angle says little about the controller. Three runs at full duty gave:
- a gap of 20–30 mm between the front of the tracks and the table edge
- the brush 30–40 mm past the edge, in every run

When the brush stops past the edge in a dump cycle, the debris in front of it falls to the floor before the bucket is lowered. There is no time for more runs before the Part 2 deadline.

### Options

1. Keep D022: the final angle to the edge.
2. How far the brush stops past the edge, with the track gaps as the constraint, tuned with the approach duty.
3. As option 2, but tuned with the threshold instead of the duty.

### Choice

Option 2:
- **Tuned:** the approach duty, from the slowest duty that moves the robot reliably with the brush down up to 100 %.
- **Measured in each run, with a ruler:** how far the brush stops past the edge, and the gap between the front of each track and the edge.
- **Constraint:** both gaps stay above 0 in every run.
- **Best:** the duty with the smallest brush overshoot. Among the duties whose results cannot be told apart from the best one (their ranges overlap), the fastest wins.
- **Baseline:** the three runs at full duty above.

### Reasons

- It fits the controller we have. Both tracks stop together, so the overshoot shows what the duty changes, and the angle does not.
- The overshoot decides whether the dump cycle works: debris pushed past the edge is lost before the bucket is lowered.
- A ruler is enough, and the baseline is already measured.
- At full duty, the robot moves the farthest between passing the edge and standing still, so the duty is the first thing to tune. A lower threshold (option 3) is limited by the noise in the table readings, so it comes second.

### Consequences

- **D022:** the measure and the controller change. Its reality-gap points (the cone of the sensor, the off-center brush-arm mount, track slip) still apply.
- **Sketch:** testing lower duties needs PWM on the motor driver inputs. Sheet 02 (`electronics/wiring/02_drivers.yml`) lists the pins with PWM.
- **Stop:** the sketch turns off the motor driver to stop, so the motors coast (D024). Braking may shorten the overshoot and is worth a test.
- **Part 2:** section 1 of `docs/part_2/README.md` and the poster report the task and the baseline. The duty sweep is future work.
- **`CLAUDE.md`:** the Should goal, the MVP note and the list of experiments name this task (updated 2026-11-23).

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
