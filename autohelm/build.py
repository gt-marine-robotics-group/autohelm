from pathlib import Path
from subprocess import run
import sys

import typer

from autohelm.protogen import build_protobuf
from autohelm.pio import pio_build

app = typer.Typer(
    help='Build firmware'
)

@app.command('build')
def build():
    """Build"""
    typer.echo('Building Protobuf Messages')
    build_protobuf()
    typer.echo('Building PlatformIO Project')
    pio_build()

