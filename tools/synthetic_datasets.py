import osmnx  # as antigravity (https://xkcd.com/353/)
import struct
import numpy as np
import geopandas as gpd
from shapely import Point
from pathlib import Path
from pyproj import Transformer
from nyctaxi_datasets import create_queries

# Center of NYC in EPSG:32118, generated using _get_transformation_settings
NYC_CENTER_CART = (302170.38872842223, 64903.128892935325)


def create_binary(target_file, n_points, radius, settings, to_nyc=False):
    # Generate road graph.
    G = osmnx.graph_from_point(settings['center'], network_type="drive", dist=radius)
    G = osmnx.projection.project_graph(G, to_crs=settings['crs'])

    # First, uniformly sample a set of points.
    points = osmnx.utils_geo.sample_points(G, n_points)

    # Compute center point.
    x, y = Transformer.from_crs(4326, settings['crs']).transform(*settings['center'])
    center_point = Point(y, x) if settings.get('y_is_easting', False) else Point(x, y)

    # Compute sampling weights based on squared distance to center.
    d = points.distance(center_point)
    p = d ** -2
    p = p / np.sum(p)

    # Move the dataset to NYC if desired.
    if to_nyc:
        points = points.translate(-center_point.x + NYC_CENTER_CART[0], -center_point.y + NYC_CENTER_CART[1])

    # Then, resample using the weights points. This will lead to duplicate points, but it should not be a problem.
    points = np.random.choice(points, n_points, p=p)
    
    # Project to EPSG:4326.
    points = gpd.GeoDataFrame(geometry=points, crs=32118 if to_nyc else settings['crs'])
    points = osmnx.projection.project_gdf(points, to_latlong=True)

    # Write to file.
    with open(target_file, "wb") as f:
        for p in points.geometry:
            f.write(struct.pack("d", p.y)) # lat
            f.write(struct.pack("d", p.x)) # lon


def get_transformation_settings_compat(center, crs, y_is_easting=False):
    return {
        'crs': crs,
        'center': NYC_CENTER_CART,
        'scale': 1,
        'transformed_center': Transformer.from_crs(4326, crs).transform(*center),
        'y_is_easting': y_is_easting
    }


if __name__ == '__main__':
    DATA_FOLDER = Path('data/synthetic')

    N_POINTS = 25_000_000
    RADIUS = 10_000

    nyc_settings = {
        'center': (40.747659, -73.986230),
        'crs': 32118,
    }

    tokyo_settings = {
        'center': (35.681471, 139.765617),
        'crs': 6684,
        'y_is_easting': True
    }

    delhi_settings = {
        'center': (28.642832, 77.218273),
        'crs': 24378
    }

    saopaolo_settings = {
        'center': (-23.546118, -46.635005),
        'crs': 5641
    }

    # create_binary(DATA_FOLDER / 'nyc' / 'nyc-25m.bin', N_POINTS, RADIUS, nyc_settings)
    # create_binary(DATA_FOLDER / 'tokyo' / 'tokyo-25m.bin', N_POINTS, RADIUS, tokyo_settings)
    # create_binary(DATA_FOLDER / 'delhi' / 'delhi-25m.bin', N_POINTS, RADIUS, delhi_settings)
    # create_binary(DATA_FOLDER / 'saopaolo' / 'saopaolo-25m.bin', N_POINTS, RADIUS, saopaolo_settings)

    # create_queries(DATA_FOLDER / 'nyc' / 'queries', DATA_FOLDER / 'tokyo' / 'queries', get_transformation_settings_compat(**tokyo_settings))
    # create_queries(DATA_FOLDER / 'nyc' / 'queries', DATA_FOLDER / 'delhi' / 'queries', get_transformation_settings_compat(**delhi_settings))
    # create_queries(DATA_FOLDER / 'nyc' / 'queries', DATA_FOLDER / 'saopaolo' / 'queries', get_transformation_settings_compat(**saopaolo_settings))

    # create_binary(DATA_FOLDER / 'tokyo' / 'tokyo-nyc-25m.bin', N_POINTS, RADIUS, tokyo_settings, True)
    create_binary(DATA_FOLDER / 'delhi' / 'delhi-nyc-25m.bin', N_POINTS, RADIUS, delhi_settings, True)
    create_binary(DATA_FOLDER / 'saopaolo' / 'saopaolo-nyc-25m.bin', N_POINTS, RADIUS, saopaolo_settings, True)
