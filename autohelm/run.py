import typer

from autohelm.comm.serial import send as ser_send

app = typer.Typer(
    help='Run Autohelm Driver'
)

@app.command('send')
def send():
    ser_send()