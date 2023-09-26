import subprocess
import geopandas as gpd
from threading import Thread
from queue import Queue

def get_omgevingsvergunning_areas(folder_path) -> gpd.GeoDataFrame:
    """
    Get the areas for which an omgevingsvergunning is required.
    These areas are defined in railmaps, in the "omgevingsvergunning milieu" layer.
    Data for this was exported from railmaps directly (by selecting the layer in a rectangle around
    Netherlands and exporting it as a shapefile)

    Returns:
        A geodataframe with the areas
    """

    df = (
        gpd.read_file(folder_path)
        .assign(geometry=lambda x: x.geometry.buffer(10))
        # remove all non alphanumerical characters from the name
        .assign(
            EMPLACEMEN=lambda x: x["EMPLACEMEN"].str.replace(r"[^a-zA-Z0-9\s]", ""),
            regex=True,
        )
        .to_crs("EPSG:4326")
    )

    return df


def _wrap_process(command, stdin_queue, log_file):
    with open(log_file, "w") as f:
        p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=f, stderr=f)

        while True:
            item = stdin_queue.get()

            if item is None:
                stdin_queue.task_done()
                break

            p.stdin.write(item)
            p.stdin.flush()
            stdin_queue.task_done()


def run_experiment(data: [bytes], command="cat", log_file="cat.txt"):
    stdin_queue = Queue()
    t = Thread(target=_wrap_process, args=(command, stdin_queue, log_file))
    t.start()

    for d in data:
        stdin_queue.put(d)

    stdin_queue.put(None)
    stdin_queue.join()
    t.join()
