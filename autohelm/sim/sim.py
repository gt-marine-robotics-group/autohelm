import typer
import signal

from autohelm.sim.mockhelm import MockHelm

app = typer.Typer(
    help='Run Autohelm Simulation'
)

@app.command('mock')
def mock():
    mh = MockHelm()
    mh.start()

    try:
        signal.pause()
    except KeyboardInterrupt:
        mh.close()