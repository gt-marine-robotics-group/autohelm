# Main app

import typer

from autohelm.build import app as build_app


cli = typer.Typer()

cli.add_typer(build_app)