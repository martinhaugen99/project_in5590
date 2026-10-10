"""Example Hydra entry point for robot experiments.

Run it with ``uv run deskpal-experiment`` and override config values on the
command line, for example ``uv run deskpal-experiment name=stopping_distance``.
Each run gets its own folder under ``outputs/`` with the resolved config, the
command-line overrides, the git state and the log.
"""

import logging
import subprocess
from pathlib import Path

import hydra
from hydra.core.hydra_config import HydraConfig
from omegaconf import DictConfig, OmegaConf

log = logging.getLogger(__name__)

# Absolute, because Hydra reads a relative path as a Python package name when the
# script runs as an installed command. Assumes the editable install that uv uses.
CONFIG_DIR = Path(__file__).resolve().parents[2] / "configs"


def git_state() -> dict[str, str | bool]:
    """Return the current git commit and whether the working tree has changes.

    Returns:
        A dict with ``commit`` (the full hash, or ``"unknown"`` when there is no
        commit yet) and ``dirty`` (``True`` when there are uncommitted changes).
    """
    head = subprocess.run(
        ["git", "rev-parse", "HEAD"], capture_output=True, text=True, check=False
    )
    status = subprocess.run(
        ["git", "status", "--porcelain"], capture_output=True, text=True, check=False
    )
    commit = head.stdout.strip() if head.returncode == 0 else "unknown"
    return {"commit": commit, "dirty": bool(status.stdout.strip())}


@hydra.main(version_base="1.3", config_path=str(CONFIG_DIR), config_name="experiment")
def main(cfg: DictConfig) -> None:
    """Record the git state and log the config of one experiment run."""
    output_dir = Path(HydraConfig.get().runtime.output_dir)
    (output_dir / "git.yaml").write_text(OmegaConf.to_yaml(git_state()))
    log.info("Config:\n%s", OmegaConf.to_yaml(cfg))
    log.info("Run folder: %s", output_dir)
