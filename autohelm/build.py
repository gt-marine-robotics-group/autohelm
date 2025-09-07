from pathlib import Path
from subprocess import run
import sys

import typer

from autohelm.proto import build_protobuf

app = typer.Typer()

@app.command('build')
def build():
    """Build"""
    build_protobuf()
