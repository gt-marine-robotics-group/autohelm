# Main app

import typer

from autohelm.build import app as build_app
from autohelm.generate import app as gen_app

cli = typer.Typer()

cli.add_typer(build_app)
cli.add_typer(gen_app, 
              name="generate")