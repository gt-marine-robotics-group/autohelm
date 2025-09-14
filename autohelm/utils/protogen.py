from pathlib import Path
from subprocess import run
import sys

try:
    from autohelm import PACKAGE_DIR
except ImportError:
    PACKAGE_DIR = Path(__file__).resolve().parents[1].parent # Needs to be defined for build


PB_DIR = PACKAGE_DIR / 'proto'
PBGEN_PY = PACKAGE_DIR / 'autohelm' / 'pb'
PBGEN_MC = PACKAGE_DIR / 'firmware' / 'lib' / 'pb'

NANOPB_PLUGIN = Path(sys.executable).parent / 'protoc-gen-nanopb'

if sys.platform.startswith("win"):
    NANOPB_PLUGIN = NANOPB_PLUGIN.with_suffix(".exe")

def build_protobuf():
    """Build Protobuf"""
    PBGEN_PY.mkdir(parents=True, exist_ok=True)
    PBGEN_MC.mkdir(parents=True, exist_ok=True)

    for pb_file in PB_DIR.glob('*.proto'):
        try:
            run([
                sys.executable,
                '-m', 'grpc_tools.protoc',
                f'-I={PB_DIR}',
                f'--python_out={PBGEN_PY}',
                str(pb_file)
            ], check=True)
        except Exception as e:
            print(e)

        try:
            run([
                sys.executable,
                '-m', 'grpc_tools.protoc',
                f'-I={PB_DIR}',
                f'--plugin=protoc-gen-nanopb={NANOPB_PLUGIN}',
                f'--nanopb_out={PBGEN_MC}',
                str(pb_file)
            ], check=True)
        except Exception as e:
            print(e)

if __name__ == "__main__":
    build_protobuf()