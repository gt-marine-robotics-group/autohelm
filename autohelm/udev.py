from pathlib import Path
from subprocess import run
from urllib.request import urlopen

import typer

from autohelm import CACHE_DIR

TEENSY_UDEV_URL = 'https://www.pjrc.com/teensy/00-teensy.rules'
TEENSY_UDEV_FILE = CACHE_DIR / '00-teensy.rules'

app = typer.Typer(
    help='Generate udev files'
)

@app.callback(invoke_without_command=True)
def udev(ctx: typer.Context):
    """Generate"""
    if ctx.invoked_subcommand is None:
        typer.echo(ctx.get_help())
        typer.echo(
            'Recommended usage: \n' \
            "sudo sh -c 'autohelm udev [COMMAND] >> /etc/udev/rules.d/<NAME>.rules"
        )
        raise typer.Exit()

@app.command('teensy')
def teensy():
    """
    Teensy udev rules to export to `00-teensy.rules`
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
            