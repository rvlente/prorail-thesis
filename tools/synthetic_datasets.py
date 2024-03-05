import osmnx  # as antigravity (https://xkcd.com/353/)
import struct
import numpy as np
import geopandas as gpd
from shapely import Point
from pathlib import Path
from pyproj import Transformer
import multiprocessing
from functools import partial


def create_binary(bin_file, n_points, radius, settings):
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
    points_gdf = gpd.GeoDataFrame(geometry=points, crs=settings['crs'])
    points_gdf_proj = osmnx.projection.project_gdf(points_gdf, to_latlong=True)

    # Write to file.
    print(f"Writing points to {bin_file}...")
    with open(bin_file, "wb") as f:
        for p in points_gdf_proj.geometry:
            f.write(struct.pack("d", p.y)) # lat
            f.write(struct.pack("d", p.x)) # lon
    
    return points_gdf.geometry


def _create_distance_query(points, target_n_points):
    # Sample point from dataset.
    query_point = np.random.choice(points)

    # Compute distances to query point.
    point_distances = points.distance(query_point)
    point_distances = np.sort(point_distances)

    # To get a distance that includes N points, take the Nth distance from the array. 
    query_distance = point_distances[target_n_points]

    return query_point.x, query_point.y, query_distance


def _create_range_query(points, xx, yy, target_n_points):
    # Sample point from dataset.
    query_center = np.random.choice(points)
    xc, yc = query_center.x, query_center.y

     # Compute distances to query center.
    point_distances = points.distance(query_center)
    point_distances = np.sort(point_distances)

    # Create a rectangle from opposite points on the circle.
    query_point_a = points[target_n_points]
    xa, ya = query_point_a.x, query_point_a.y
    xb, yb = xc - (xa - xc),  yc - (ya - yc)

    xa, xb = np.sort([xa, xb])
    ya, yb = np.sort([ya, yb])

    # Refine query rectangle until the target number of points are contained.
    n_points = np.sum((xa <= xx) & (xx <= xb) & (ya <= yy) & (yy <= yb))

    if n_points <= target_n_points:
        while n_points < target_n_points:
            xa -= 100
            ya -= 100
            xb += 100
            yb += 100
            n_points = np.sum((xa <= xx) & (xx <= xb) & (ya <= yy) & (yy <= yb))
    else:
        while n_points > target_n_points:
            xa += 100
            ya += 100
            xb -= 100
            yb -= 100
            n_points = np.sum((xa <= xx) & (xx <= xb) & (ya <= yy) & (yy <= yb))
    
    return xa, ya, xb, yb


def create_distance_queries(points, target_file, n_queries, selectivity, settings):
    print("Generating distance queries...")

    target_n_points = np.ceil(selectivity * len(points)).astype(int)

    with multiprocessing.get_context("fork").Pool() as pool:
        dqueries = pool.starmap(partial(_create_distance_query, points, target_n_points), [() for _ in range(n_queries)])
    
    # Project query points and write to file.
    print(f"Writing distance queries to {target_file}...")
    t = Transformer.from_crs(settings['crs'], 4326)

    with open(target_file, "w") as f:
        for x, y, d in dqueries:
            x, y = t.transform(x, y)
            f.write(f"{x},{y},{d}\n")


def create_range_queries(points, target_file, n_queries, selectivity, settings):
    print("Generating range queries...")

    target_n_points = np.ceil(selectivity * len(points)).astype(int)
    
    xx = np.array([p.x for p in points])
    yy = np.array([p.y for p in points])

    with multiprocessing.get_context("fork").Pool() as pool:
        rqueries = pool.starmap(partial(_create_range_query, points, xx, yy, target_n_points), [() for _ in range(n_queries)])

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

    N_POINTS = 25_000_000
    RADIUS = 10_000
    N_QUERIES = 10_000
    QUERY_SELECTIVITY = 0.1

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

    points = create_binary(DATA_FOLDER / 'nyc' / 'nyc-25m.bin', N_POINTS, RADIUS, nyc_settings)
    create_distance_queries(points, DATA_FOLDER / 'nyc'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, nyc_settings)
    create_range_queries(points, DATA_FOLDER / 'nyc'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, nyc_settings)

    points = create_binary(DATA_FOLDER / 'tokyo' / 'tokyo-25m.bin', N_POINTS, RADIUS, tokyo_settings)
    create_distance_queries(points, DATA_FOLDER / 'tokyo'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, tokyo_settings)
    create_range_queries(points, DATA_FOLDER / 'tokyo'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, tokyo_settings)

    points = create_binary(DATA_FOLDER / 'delhi' / 'delhi-25m.bin', N_POINTS, RADIUS, delhi_settings)
    create_distance_queries(points, DATA_FOLDER / 'delhi'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, delhi_settings)
    create_range_queries(points, DATA_FOLDER / 'delhi'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, delhi_settings)

    points = create_binary(DATA_FOLDER / 'saopaolo' / 'saopaolo-25m.bin', N_POINTS, RADIUS, saopaolo_settings)
    create_distance_queries(points, DATA_FOLDER / 'saopaolo'/ 'queries' / 'synthetic_distance_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, saopaolo_settings)
    create_range_queries(points, DATA_FOLDER / 'saopaolo'/ 'queries' / 'synthetic_range_0.1.csv', N_QUERIES, QUERY_SELECTIVITY, saopaolo_settings)
