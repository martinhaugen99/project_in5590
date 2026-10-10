# DeskPal

DeskPal is a small animatronic desk robot on tracks, built for IN5590 (Rapid Prototyping of Robotic Systems) at UiO. It shows emotions on a display, follows its person, and must never drive off the table. This repository holds the CAD, electronics, firmware, host-side tools and documentation.

## Repository layout

```
project_in5590/
├── configs/            # Hydra configs for experiments
├── docs/               # Deliverables, decisions, experiment reports
├── src/deskpal/        # Host-side Python package (experiments, analysis, tools)
├── tests/              # Host-side tests (pytest)
├── bom.csv             # Bill of materials
├── pyproject.toml      # Python project, dependencies and tool settings
├── uv.lock             # Exact versions of every Python package (committed)
├── .python-version     # Python version used by uv (3.13)
├── .pre-commit-config.yaml
└── CLAUDE.md           # Project rules for coding agents
```

More folders (`cad/`, `print/`, `electronics/`, `firmware/`, `data/`) are added when they are needed.

## Setup

1. Install [uv](https://docs.astral.sh/uv/getting-started/installation/) once. It is the only tool installed outside the project.
2. From the repository root:

   ```bash
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

## Running experiments (Hydra)

Experiments are configured with [Hydra](https://hydra.cc/docs/1.3/intro/). The example entry point is `src/deskpal/experiment.py`, with its config in `configs/experiment.yaml`.

```bash
uv run deskpal-experiment                                   # default config
uv run deskpal-experiment name=stopping_distance notes="first try"
uv run deskpal-experiment --multirun notes=a,b              # one run per value
```

Each run gets its own folder, `outputs/<name>/<date>_<time>/` (or `multirun/<name>/<date>_<time>/<job>/`), containing:

| File | Content |
|---|---|
| `.hydra/config.yaml` | The full config used for the run |
| `.hydra/overrides.yaml` | The command-line overrides |
| `git.yaml` | Git commit and whether there were uncommitted changes (`dirty`) |
| `experiment.log` | The run log |

`outputs/` and `multirun/` are not committed. Each experiment report in `docs/experiments/` names its run folder and copies the key results. Sweep over parameters, not over `name`: `name` is part of the folder path, so sweeping it splits the runs across folders.
