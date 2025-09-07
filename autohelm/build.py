from pathlib import Path
from subprocess import run
import sys

import typer

from autohelm.protogen import build_protobuf

app = typer.Typer(
    help='Build firmware'
)

@app.command('build')
def build():
    """Build"""
    build_protobuf()
