from platformio.run.cli import cli as run_cli
from platformio.device.list.command import device_list_cmd



from autohelm import PACKAGE_DIR

def build():
    """Build firmware"""
    args = ['--project-dir', PACKAGE_DIR]
    run_cli(args)

def list():
    """List serial devices"""
    device_list_cmd([])


def flash(port=''):
    """Flash"""
    args = ['--project-dir', PACKAGE_DIR,
            '-t', 'upload']
            # '--upload-port', 'COM7']
    run_cli(args)