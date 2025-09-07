from pathlib import Path
from subprocess import run
import sys

import typer

from autohelm.utils.protogen import build_protobuf
from autohelm.utils.pio import pio_build

app = typer.Typer(
    help='Build firmware'
)

@app.command('build')
def build():
    """Build"""
    typer.echo(typer.style(
        '[AUTOHELM] Building Protobuf Messages',
        fg=typer.colors.BLUE, bold=True))
    build_protobuf()
    typer.echo(typer.style(
        '[AUTOHELM] Building PlatformIO Project',
         fg=typer.colors.BLUE, bold=True))
    pio_build()

