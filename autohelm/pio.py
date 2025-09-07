from platformio.run.cli import cli as run_cli

from autohelm import PACKAGE_DIR

args = ['--project-dir', PACKAGE_DIR]

def pio_build():
    """HI"""
    run_cli(args)
