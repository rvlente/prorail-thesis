"""
generate-nyctaxi.py

This script provides functions to download the 2009-2010 NYC TLC Taxi dataset and write 
it to either GeoPackage or binary format.

Example usage:

    python -i generate-nyctaxi.py
    >>> DATA_FOLDER = Path('data/nyc-taxi')
    >>> download_files(DATA_FOLDER / 'raw')
    >>> create_binary(DATA_FOLDER / 'raw', DATA_FOLDER / 'nyc-taxi-30m.bin', 30_000_000)
"""

import duckdb
import struct
from functools import partial
from pyproj import Transformer
import numpy as np
import time
from pathlib import Path
import multiprocessing


def setup_duckdb(input_folder):
    create_data_query = f"""
    DROP TABLE IF EXISTS nyctaxi;

    CREATE TABLE nyctaxi AS
    SELECT * FROM '{input_folder}/nyc-taxi-rides.parquet';
    """

    conn = duckdb.connect(database=":memory:", read_only=False)
    conn.execute(create_data_query)
    return conn


def _get_transformation_settings(conn, crs, center=None, radius=None, y_is_easting=False):
    tf_cart = Transformer.from_crs(4326, 32118)
    tf_center = Transformer.from_crs(4326, crs)

    coords = conn.sql('SELECT lat, lon FROM nyctaxi USING SAMPLE 100000;').fetchnumpy()
    
    xx, yy = tf_cart.transform(coords['lat'], coords['lon'])
    x_mean = np.mean(xx)
    y_mean = np.mean(yy)

    if radius is None:
        scale = 1
    else:
        # Compute the radius of the original dataset.
        r = np.max(np.sqrt(np.square(xx - x_mean) + np.square(yy - y_mean)))
        scale = radius / r

    transformed_x_mean, transformed_y_mean = tf_center.transform(*center)

    return {
        'crs': crs,
        'center': (x_mean, y_mean),
        'transformed_center': (transformed_x_mean, transformed_y_mean),
        'scale': scale,
        'y_is_easting': y_is_easting
    }


def _transform_job(coords, crs, center, transformed_center, scale, y_is_easting):
    lats, lons = coords

    # Transform to cartesian coordinates.
    tf_cart = Transformer.from_crs(4326, 32118)
    xx, yy = tf_cart.transform(lats, lons)

    # Normalize to center 0, 0.
    x_center, y_center = center
    xx -= x_center
    yy -= y_center

    # Rescale x and y.
    xx *= scale
    yy *= scale

    # Reposition in new CRS.
    if y_is_easting:
        xx, yy = yy, xx
    
    transformed_x_center, transformed_y_center = transformed_center
    xx += transformed_x_center
    yy += transformed_y_center

    # Transform back to lat, lon.
    tf_latlon = Transformer.from_crs(crs, 4326)
    return tf_latlon.transform(xx, yy)


def transform_efficient(lats, lons, transform_settings):
    """
    Transform latitude, longitude arrays using the given settings. Uses multiprocessing
    to significantly speed up computation.
    """
    batches = np.stack([np.array_split(lats, 1000), np.array_split(lons, 1000)], axis=1)

    with multiprocessing.get_context("fork").Pool() as pool:
        result = pool.map(partial(_transform_job, **transform_settings), batches)
    
    return np.concatenate(result, axis=1)


def create_binary(conn, target_file, n_points, transform_settings=None):
    """
    Create a binary file of points from the nyc-taxi dataset.
    """

    if target_file.exists():
        print(f'File <{target_file}> exists, skipping...')
        return

    print(f'Creating data file <{target_file}>...')
    start = time.time()

    query = f"SELECT lat, lon FROM nyctaxi{'' if n_points is None else f' USING SAMPLE {n_points}'};"
    coords = conn.sql(query).fetchnumpy()
    lats = coords['lat']
    lons = coords['lon']

    
    if transform_settings is not None:
        lats, lons = transform_efficient(lats, lons, transform_settings)

    with open(target_file, "wb") as f:
        for lat, lon in zip(lats, lons):
            f.write(struct.pack("d", lat))
            f.write(struct.pack("d", lon))
    
    end = time.time()
    print(f'Done. {end - start:.2f} seconds elapsed.')


def create_queries(raw_folder, target_folder, transform_settings):
    for qf in raw_folder.glob('*_distance_*.csv'):
        target_file = target_folder / qf.name
        if target_file.exists():
            print(f'File <{target_file}> exists, skipping...')
            continue

        print(f'Creating query file <{target_file}>...')
        lats = []
        lons = []
        ds = []

        with qf.open() as f:
            for line in f:
                lat, lon, d = line.strip().split(',')
                lats.append(float(lat))
                lons.append(float(lon))
                ds.append(float(d))
        
        lats, lons = transform_efficient(np.array(lats), np.array(lons), transform_settings)
        ds = np.array(ds)
        ds *= transform_settings['scale']

        with open(target_file, 'w') as f:
            for lat, lon, d in zip(lats, lons, ds):
                f.write(f'{lat:.6f},{lon:.6f},{d:.2f}\n')
    
    for rf in raw_folder.glob('*_range_*.csv'):
        target_file = target_folder / rf.name
        if target_file.exists():
            print(f'File <{target_file}> exists, skipping...')
            continue

        print(f'Creating query file <{target_file}>...')
        latas = []
        lonas = []
        latbs = []
        lonbs = []

        with rf.open() as f:
            for line in f:
                lata, lona, latb, lonb = line.strip().split(',')
                latas.append(float(lata))
                lonas.append(float(lona))
                latbs.append(float(latb))
                lonbs.append(float(lonb))

        latas, lonas = transform_efficient(np.array(latas), np.array(lonas), transform_settings)
        latbs, lonbs = transform_efficient(np.array(latbs), np.array(lonbs), transform_settings)

        with open(target_file, 'w') as f:
            for lata, lona, latb, lonb in zip(latas, lonas, latbs, lonbs):
                f.write(f'{lata:.6f},{lona:.6f},{latb:.6f},{lonb:.6f}\n')


def generate_datasets(conn, raw_query_folder, data_folder, name, **settings):
    if not settings:
        transform_settings = None
    else:
        transform_settings = _get_transformation_settings(conn, **settings)

    create_binary(conn, data_folder / name / f'{name}-0_25m.bin', 250_000, transform_settings)
    # create_binary(conn, data_folder / name / f'{name}-2_5m.bin', 2_500_000, transform_settings)
    # create_binary(conn, data_folder / name / f'{name}-25m.bin', 25_000_000, transform_settings)
    # create_binary(conn, data_folder / name / f'{name}-250m.bin', 250_000_000, transform_settings)

    if raw_query_folder is not None:
        create_queries(raw_query_folder, data_folder / name / 'queries', transform_settings)


if __name__ == '__main__':
    DATA_FOLDER = Path('data/taxi')

    shippensburg_settings = {
        'center': (40.112385, -77.519584),
        'crs': 32118,
    }

    aogaki_settings = {
        'center': (35.263921, 134.999230),
        'crs': 6673,
        'y_is_easting': True
    }

    germany_settings = {
        'center': (50.940709, 6.9575126),
        'radius': 300_000,
        'crs': 4839,
        'y_is_easting': True 
    }

    japan_settings = {
        'center': (35.263921, 134.999230),
        'radius': 300_000,
        'crs': 6673,
        'y_is_easting': True
    }

    conn = setup_duckdb(DATA_FOLDER / 'nyc-taxi' / 'raw')
    query_folder = DATA_FOLDER / 'nyc-taxi' / 'queries'

    # generate_datasets(conn, None, DATA_FOLDER, 'nyc-taxi')
    # generate_datasets(conn, query_folder, DATA_FOLDER, 'shippensburg-taxi', **shippensburg_settings)
    # generate_datasets(conn, query_folder, DATA_FOLDER, 'aogaki-taxi', **aogaki_settings)
    generate_datasets(conn, query_folder, DATA_FOLDER, 'germany-taxi', **germany_settings)
    # generate_datasets(conn, query_folder, DATA_FOLDER, 'japan-taxi', **japan_settings)
