import osmnx  # as antigravity (https://xkcd.com/353/)
import struct
from pathlib import Path
from pyproj import Transformer
from nyctaxi_datasets import create_queries


def create_binary(target_file, n_points, radius, settings):
    G = osmnx.graph_from_point(settings['center'], network_type="drive", dist=radius)
    G = osmnx.projection.project_graph(G, to_crs=settings['crs'])
    points = osmnx.utils_geo.sample_points(G, n_points)
    points = osmnx.projection.project_gdf(points, to_latlong=True)

    with open(target_file, "wb") as f:
        for p in points:
            f.write(struct.pack("d", p.y)) # lat
            f.write(struct.pack("d", p.x)) # lon


def get_transformation_settings(center, crs):
    return {
        'crs': crs,
        'center': (302170.38872842223, 64903.128892935325), # generated using _get_transformation_settings
        'scale': 1,
        'transformed_center': Transformer.from_crs(4326, crs).transform(*center),
        'y_is_easting': False
    }


if __name__ == '__main__':
    DATA_FOLDER = Path('data/synthetic')

    N_POINTS = 25_000_000
    RADIUS = 10_000

    nyc_settings = {
        'center': (40.747659, -73.986230),
        'crs': 32118,
    }

    delhi_settings = {
        'center': (28.642832, 77.218273),
        'crs': 24378
    }

    tokyo_settings = {
        'center': (35.681471, 139.765617),
        'crs': 6684
    }

    sao_paolo_settings = {
        'center': (-23.546118, -46.635005),
        'crs': 5641
    }

    create_binary(DATA_FOLDER / 'nyc' / 'nyc-25m.bin', N_POINTS, RADIUS, nyc_settings)
    create_binary(DATA_FOLDER / 'tokyo' / 'tokyo-25m.bin', N_POINTS, RADIUS, tokyo_settings)
    create_binary(DATA_FOLDER / 'delhi' / 'delhi-25m.bin', N_POINTS, RADIUS, delhi_settings)
    create_binary(DATA_FOLDER / 'sao-paolo' / 'sao-paolo-25m.bin', N_POINTS, RADIUS, sao_paolo_settings)

    create_queries(DATA_FOLDER / 'nyc' / 'queries', DATA_FOLDER / 'tokyo' / 'queries', get_transformation_settings(**tokyo_settings))
    create_queries(DATA_FOLDER / 'nyc' / 'queries', DATA_FOLDER / 'delhi' / 'queries', get_transformation_settings(**delhi_settings))
    create_queries(DATA_FOLDER / 'nyc' / 'queries', DATA_FOLDER / 'saopaolo' / 'queries', get_transformation_settings(**sao_paolo_settings))
