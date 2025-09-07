# /// script
# requires-python = ">=3.11"
# dependencies = [
#   "protobuf",
# ]
# ///

# Generate Protobuf for Python

from pathlib import Path
from subprocess import run

PACKAGE_DIR = Path(__file__).parent.parent

PB_DIR = PACKAGE_DIR / 'proto'
PBGEN_PY = PACKAGE_DIR / 'gen' / 'pbpy'

def build_protobuf():
    """Build Protobuf"""
    PBGEN_PY.mkdir(parents=True, exist_ok=True)

    for pb_file in PB_DIR.glob('*.proto'):
        run([
            'protoc',
            f'-I={PB_DIR}',
            f'--python_out={PBGEN_PY}',
            str(pb_file)
        ], check=True)

build_protobuf()
