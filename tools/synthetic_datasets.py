import osmnx  # as antigravity (https://xkcd.com/353/)
import struct
import numpy as np
import geopandas as gpd
from shapely import Point
from pathlib import Path
from pyproj import Transformer
import multiprocessing
from functools import partial
from itertools import islice
from tqdm import tqdm


def create_binary(bin_file, n_points, radius, settings):
    bin_file.parent.mkdir(parents=True, exist_ok=True)

    # Generate road graph.
    print("Generating graph...") 
    G = osmnx.graph_from_point(settings['center'], network_type="drive", dist=radius)
    G = osmnx.projection.project_graph(G, to_crs=settings['crs'])

    # First, uniformly sample a set of points.
    print("Sampling points...")
    points = osmnx.utils_geo.sample_points(G, n_points)

    # Compute center point.
    x, y = Transformer.from_crs(4326, settings['crs']).transform(*settings['center'])
    center_point = Point(y, x) if settings.get('y_is_easting', False) else Point(x, y)

    # Compute sampling weights based on squared distance to center.
    d = points.distance(center_point)
    d /= np.max(d)
    p = 1 - np.tanh(d * 10 - 2)
    p = p / np.sum(p)

    # Then, resample using the weights points. This will lead to duplicate points.
    points = np.random.choice(points, n_points, p=p)
    
    # Project to EPSG:4326.
    print("Projecting points to EPSG:4326...")
    points = gpd.GeoDataFrame(geometry=points, crs=settings['crs'])
    points = osmnx.projection.project_gdf(points, to_latlong=True)

    # Write to file.
    print(f"Writing points to {bin_file}...")
    with open(bin_file, "wb") as f:
        for p in points.geometry:
            f.write(struct.pack("d", p.y)) # lat
            f.write(struct.pack("d", p.x)) # lon


def _parse_points(file, limit):
    DSIZE = struct.calcsize("d")

    def _yield_doubles():
        with open(file, 'rb') as f:
            while chunk := f.read(DSIZE):
                yield struct.unpack("d", chunk)[0]

    coords = list(islice(_yield_doubles(), limit))
    lats = coords[::2]
    lons = coords[1::2]
    return lats, lons


def _create_distance_query(xx, yy, n, q):
    points = np.stack([xx, yy], 1)

    # Compute distances to query point.
    point_distances = np.linalg.norm(q - points, axis=1)
    point_distances = np.sort(point_distances)

    # To get a distance that includes N points, take the Nth distance from the array. 
    return q[0], q[1], point_distances[n]


def create_distance_queries(bin_file, target_file, n_queries, selectivity, settings):
    target_file.parent.mkdir(parents=True, exist_ok=True)

    print(f"Reading points from {bin_file}...")
    xx, yy = _parse_points(bin_file, 100_000)

    print(f"Projecting points to CRS {settings['crs']}...")
    xx, yy = Transformer.from_crs(4326, settings['crs']).transform(xx, yy)

    print("Generating distance queries...")
    n = np.ceil(selectivity * len(xx)).astype(int)

    points = np.stack([xx, yy], 1)
    qi = np.random.choice(len(points), n_queries)
    qs = points[qi]

    with multiprocessing.get_context("spawn").Pool() as pool:
        dqueries = list(tqdm(pool.imap(partial(_create_distance_query, xx, yy, n), qs, 10), total=len(qs)))

    # Project query points and write to file.
    print(f"Writing distance queries to {target_file}...")
    t = Transformer.from_crs(settings['crs'], 4326)

    with open(target_file, "w") as f:
        for x, y, d in dqueries:
            x, y = t.transform(x, y)
            f.write(f"{x},{y},{d}\n")


def _create_range_query(xx, yy, n, q):
    xa, ya = q
    xb, yb = xa, ya

    while np.sum((xa <= xx) & (xx <= xb) & (ya <= yy) & (yy <= yb)) < n:
        xa -= 10
        ya -= 10
        xb += 10
        yb += 10
    
    return xa, ya, xb, yb


def create_range_queries(bin_file, target_file, n_queries, selectivity, settings):
    target_file.parent.mkdir(parents=True, exist_ok=True)
    
    print(f"Reading points from {bin_file}...")
    xx, yy = _parse_points(bin_file, 100_000)

    print(f"Projecting points to CRS {settings['crs']}...")
    xx, yy = Transformer.from_crs(4326, settings['crs']).transform(xx, yy)

    print("Generating range queries...")
    n = np.ceil(selectivity * len(xx)).astype(int)

    points = np.stack([xx, yy], 1)
    qi = np.random.choice(len(points), n_queries)
    qs = points[qi]

    with multiprocessing.get_context("spawn").Pool() as pool:
        rqueries = list(tqdm(pool.imap(partial(_create_range_query, xx, yy, n), qs, 10), total=len(qs)))

    # Project query points and write to file.
    print(f"Writing range queries to {target_file}...")
    t = Transformer.from_crs(settings['crs'], 4326)

    with open(target_file, "w") as f:
        for xa, ya, xb, yb in rqueries:
            xa, ya = t.transform(xa, ya)
            xb, yb = t.transform(xb, yb)
            f.write(f"{xa},{ya},{xb},{yb}\n")


if __name__ == '__main__':
    DATA_FOLDER = Path('data/synthetic')

    N_POINTS = 10_000_000
    RADIUS = 10_000
    N_QUERIES = 10_000
    QUERY_SELECTIVITY = 0.001 # 0.1%

    nyc_settings = {
        'center': (40.747659, -73.986230),
        'crs': 32118,
    }

    tokyo_settings = {
        'center': (35.681471, 139.765617),
        'crs': 6677,
        'y_is_easting': True
    }

    delhi_settings = {
        'center': (28.642832, 77.218273),
        'crs': 24378
    }

    saopaolo_settings = {
        'center': (-23.546118, -46.635005),
        'crs': 29101
    }

    create_binary(DATA_FOLDER / 'nyc' / 'nyc-10m.bin', N_POINTS, RADIUS, nyc_settings)
    create_distance_queries(DATA_FOLDER / 'nyc' / 'nyc-10m.bin', DATA_FOLDER / 'nyc'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, nyc_settings)
    create_range_queries(DATA_FOLDER / 'nyc' / 'nyc-10m.bin', DATA_FOLDER / 'nyc'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, nyc_settings)

    create_binary(DATA_FOLDER / 'tokyo' / 'tokyo-10m.bin', N_POINTS, RADIUS, tokyo_settings)
    create_distance_queries(DATA_FOLDER / 'tokyo' / 'tokyo-10m.bin', DATA_FOLDER / 'tokyo'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, tokyo_settings)
    create_range_queries(DATA_FOLDER / 'tokyo' / 'tokyo-10m.bin', DATA_FOLDER / 'tokyo'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, tokyo_settings)

    create_binary(DATA_FOLDER / 'delhi' / 'delhi-10m.bin', N_POINTS, RADIUS, delhi_settings)
    create_distance_queries(DATA_FOLDER / 'delhi' / 'delhi-10m.bin', DATA_FOLDER / 'delhi'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, delhi_settings)
    create_range_queries(DATA_FOLDER / 'delhi' / 'delhi-10m.bin', DATA_FOLDER / 'delhi'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, delhi_settings)

    create_binary(DATA_FOLDER / 'saopaolo' / 'saopaolo-10m.bin', N_POINTS, RADIUS, saopaolo_settings)
    create_distance_queries(DATA_FOLDER / 'saopaolo' / 'saopaolo-10m.bin', DATA_FOLDER / 'saopaolo'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, saopaolo_settings)
    create_range_queries(DATA_FOLDER / 'saopaolo' / 'saopaolo-10m.bin', DATA_FOLDER / 'saopaolo'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, saopaolo_settings)
