from pathlib import Path
from subprocess import run
import sys


PACKAGE_DIR = Path(__file__).parent.parent # Needs to be defined for build

PB_DIR = PACKAGE_DIR / 'proto'
PBGEN_PY = PACKAGE_DIR / 'autohelm' / 'pb'
PBGEN_MC = PACKAGE_DIR / 'firmware' / 'lib' / 'pb'

NANOPB_PLUGIN = Path(sys.executable).parent / 'protoc-gen-nanopb'

def build_protobuf():
    """Build Protobuf"""
    PBGEN_PY.mkdir(parents=True, exist_ok=True)
    PBGEN_MC.mkdir(parents=True, exist_ok=True)

    for pb_file in PB_DIR.glob('*.proto'):
        run([
            'protoc',
            f'-I={PB_DIR}',
            f'--python_out={PBGEN_PY}',
            str(pb_file)
        ], check=True)

        run([
            'protoc',
            f'-I={PB_DIR}',
            f'--plugin=protoc-gen-nanopb={NANOPB_PLUGIN}',
            f'--nanopb_out={PBGEN_MC}',
            str(pb_file)
        ], check=True)

if __name__ == "__main__":
    build_protobuf()