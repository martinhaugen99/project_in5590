from hydra import compose, initialize_config_dir

from mats.experiment import CONFIG_DIR, git_state


def test_experiment_config_composes_with_override() -> None:
    # Arrange
    overrides = ["name=smoke"]

    # Act
    with initialize_config_dir(version_base="1.3", config_dir=str(CONFIG_DIR)):
        cfg = compose(config_name="experiment", overrides=overrides)

    # Assert
    assert cfg.name == "smoke"
    assert cfg.notes == ""


def test_git_state_returns_commit_and_dirty_flag() -> None:
    # Act
    state = git_state()

    # Assert
    assert isinstance(state["commit"], str)
    assert isinstance(state["dirty"], bool)
