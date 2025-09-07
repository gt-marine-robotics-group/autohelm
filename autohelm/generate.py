from pathlib import Path
from subprocess import run
from urllib.request import urlopen

import typer

from autohelm import CACHE_DIR

TEENSY_UDEV_URL = 'https://www.pjrc.com/teensy/00-teensy.rules'
TEENSY_UDEV_FILE = CACHE_DIR / '00-teensy.rules'

app = typer.Typer(
    help='Generate utility files'
)

@app.callback(invoke_without_command=True)
def generate(ctx: typer.Context):
    """Generate"""
    if ctx.invoked_subcommand is None:
        typer.echo(ctx.get_help())
        raise typer.Exit()

@app.command('udev-teensy')
def udev_teensy():
    """
    Show udev rules to export

    Recommended usage:
      `sudo sh -c 'autohelm generate udev-teensy >> /etc/udev/rules.d/00-teensy.rules`
    """
    fetch_teensy_udev()
    udev_text = get_teensy_udev()
    for line in udev_text:
        print(line, end='')

def fetch_teensy_udev():
    with urlopen(TEENSY_UDEV_URL) as response:
        text = response.read().decode('utf-8')
        with open(TEENSY_UDEV_FILE, 'w', encoding='utf-8') as f:
            f.write(text)
        return True
    return False

def get_teensy_udev():
    with open(TEENSY_UDEV_FILE, 'r', encoding='utf-8') as f:
        lines = f.readlines()
        return lines
    return []
            