# Decisions

Numbered record of project decisions (D001, D002, …). Each entry gives the context, the options, the choice and the reasons. To change a decision, add a new entry that supersedes the old one, and set the old entry's status to "Superseded by D###". Update `CLAUDE.md` whenever a decision changes the stack or the hardware.

| ID | Date | Decision | Status |
|---|---|---|---|
| D001 | 2026-10-10 | Host-side Python tooling: uv, ruff, mypy, pytest, pre-commit | Accepted |
| D002 | 2026-10-10 | Hydra 1.3 for experiment configuration | Accepted |

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

### Options

1. **Version:**
   - hydra-core 1.3.7: the latest stable release, 14 Sep 2026.
   - 1.4.0rc1: a prerelease from 10 Oct 2026 with breaking changes. It also deprecates `version_base` ([release notes](https://github.com/facebookresearch/hydra/releases)).
2. **Where the configs live.** Hydra reads a relative `config_path` as a Python package name when the script runs as an installed command:
   - (a) `configs/` at the repository root, passed to Hydra as an absolute path.
   - (b) `src/deskpal/configs/` inside the package.
   - (c) Root `configs/` with a relative path, run as `uv run python src/deskpal/experiment.py` with no named command.

### Choice

- `hydra-core>=1.3.7,<1.4`, used as `@hydra.main(version_base="1.3", ...)` ([version_base](https://hydra.cc/docs/1.3/upgrades/version_base/)).
- Option 2a: the configs are in `configs/`, and `CONFIG_DIR` in `src/deskpal/experiment.py` passes the absolute path.
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
