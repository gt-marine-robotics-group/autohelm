from pathlib import Path

from typing_extensions import Annotated

import typer

PACKAGE_DIR = Path(__file__).parent.parent

PB_DIR = PACKAGE_DIR / 'proto'

CACHE_DIR = PACKAGE_DIR / 'cache'
CACHE_DIR.mkdir(parents=True, exist_ok=True)