# Main app

import typer

from autohelm.build import app as build_app
from autohelm.udev import app as udev_app

cli = typer.Typer()

cli.add_typer(build_app)
cli.add_typer(udev_app, 
              name="udev")