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
import polars as pl
from pyproj import Transformer
import numpy as np
from pathlib import Path # imported for convenience


def create_parquet(input_folder):
    copy_query = f"""
    COPY (
        SELECT * FROM read_csv('{input_folder}/*_rides.csv', header=False, columns={{"lat": "DOUBLE", "lon": "DOUBLE"}})
        WHERE lat BETWEEN 40.50 AND 40.95 AND lon BETWEEN -74.25 AND -73.65
    ) TO '{input_folder}/nyc-taxi-rides.parquet' (FORMAT 'parquet');
    """

    conn = duckdb.connect(database=":memory:", read_only=False)
    conn.execute(copy_query)
    conn.close()


def _setup_duckdb(input_folder):
    create_data_query = f"""
    DROP TABLE IF EXISTS nyctaxi;

    CREATE TABLE nyctaxi AS
    SELECT * FROM '{input_folder}/nyc-taxi-rides.parquet';
    """

    conn = duckdb.connect(database=":memory:", read_only=False)
    conn.execute(create_data_query)
    return conn


def create_binary(conn, target_file, n_rows, transform_func=None):
    """
    Create a binary file of points from the nyc-taxi dataset.
    """

    print(f'Creating data file <{target_file}>...')

    query = f"SELECT lat, lon FROM nyctaxi{'' if n_rows is None else f' USING SAMPLE {n_rows}'};"
    df = conn.sql(query).pl()
    
    if transform_func is not None:
        df = df.with_columns(
            pl.struct(['lat', 'lon']).map_batches(
                lambda st: pl.Series(zip(*transform_func(st.struct.field('lat').to_numpy(), st.struct.field('lon').to_numpy())))
            ).alias('latlon')
        ).with_columns(pl.col('latlon').list.to_struct()).unnest('latlon').rename({'field_0': 'lat_new', 'field_1': 'lon_new'}) \
        .select([pl.col('lat_new'), pl.col('lon_new')])

    with open(target_file, "wb") as f:
        for lat, lon in df.iter_rows():
            f.write(struct.pack("d", lat))
            f.write(struct.pack("d", lon))


def create_queries(raw_folder, target_folder, scale=1, transform_func=None):
    for qf in raw_folder.glob('*_distance_*.csv'):
        print(f'Creating query file {target_folder / qf.name}...')
        lats = []
        lons = []
        ds = []

        with qf.open() as f:
            for line in f:
                lat, lon, d = line.strip().split(',')
                lats.append(float(lat))
                lons.append(float(lon))
                ds.append(float(d))
        
        lats, lons = transform_func(np.array(lats), np.array(lons))
        ds = np.array(ds)
        ds *= scale

        with open(target_folder / qf.name, 'w') as f:
            for lat, lon, d in zip(lats, lons, ds):
                f.write(f'{lat:.6f},{lon:.6f},{d:.2f}\n')
    
    for rf in raw_folder.glob('*_range_*.csv'):
        print(f'Creating query file {target_folder / rf.name}...')
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

        latas, lonas = transform_func(np.array(latas), np.array(lonas))
        latbs, lonbs = transform_func(np.array(latbs), np.array(lonbs))

        with open(target_folder / rf.name, 'w') as f:
            for lata, lona, latb, lonb in zip(latas, lonas, latbs, lonbs):
                f.write(f'{lata:.6f},{lona:.6f},{latb:.6f},{lonb:.6f}\n')


def _get_transformation_parameters(conn, crs, center=None, size=None, y_is_easting=False):
    tf_cart = Transformer.from_crs(4326, 32118)
    tf_center = Transformer.from_crs(4326, crs)
    tf_latlon = Transformer.from_crs(crs, 4326)

    df = conn.sql('SELECT lat, lon FROM nyctaxi USING SAMPLE 100000;').pl()
    df = df.with_columns(
            pl.struct(['lat', 'lon']) \
              .map_batches(
                  lambda st: pl.Series(
                      zip(*tf_cart.transform(st.struct.field('lat').to_numpy(), st.struct.field('lon').to_numpy()))
                  )
              ) \
              .alias('xy')
            ) \
           .with_columns(pl.col('xy').list.to_struct()).unnest('xy').rename({'field_0': 'x', 'field_1': 'y'})

    x_size, y_size, x_mean, y_mean = df.select([
        pl.max('x') - pl.min('x'), pl.max('y') - pl.min('y'),
        pl.mean('x').alias('x_mean'), pl.mean('y').alias('y_mean')
    ]).row(0)

    max_size = max(x_size, y_size)

    if size is None:
        scale = 1
    else:
        scale = size / max_size

    transformed_x_mean, transformed_y_mean = tf_center.transform(*center)

    def func(lats, lons):
        # Transform to cartesian coordinates.
        xx, yy = tf_cart.transform(lats, lons)

        # Normalize to center 0, 0.
        xx -= x_mean
        yy -= y_mean

        # Rescale x and y.
        xx *= scale
        yy *= scale

        # Reposition in new CRS.
        if y_is_easting:
            xx, yy = yy, xx
        
        xx += transformed_x_mean
        yy += transformed_y_mean

        # Transform back to lat, lon.        
        return tf_latlon.transform(xx, yy)

    return scale, func


def generate_datasets(conn, raw_query_folder, data_folder, name, **settings):
    if not settings:
        scale, pipeline = 1, None
    else:
        scale, pipeline = _get_transformation_parameters(conn, **settings)

    create_binary(conn, data_folder / name / f'{name}-0_25m.bin', 250_000, pipeline)
    create_binary(conn, data_folder / name / f'{name}-2_5m.bin', 2_500_000, pipeline)
    create_binary(conn, data_folder / name / f'{name}-25m.bin', 25_000_000, pipeline)
    create_binary(conn, data_folder / name / f'{name}-250m.bin', 250_000_000, pipeline)

    if raw_query_folder is not None:
        create_queries(raw_query_folder, data_folder / name / 'queries', scale, pipeline)


if __name__ == '__main__':
    DATA_FOLDER = Path('data')

    syracuse_settings = {
        'center': (43.054477, -76.144178),
        'crs': 32118,
    }

    aogaki_settings = {
        'center': (35.263921, 134.999230),
        'crs': 6684,
        'y_is_easting': True
    }

    germany_settings = {
        'center': (50.940709, 6.9575126),
        'size': 600_000,
        'crs': 4839,
        'y_is_easting': True 
    }

    japan_settings = {
        'center': (35.263921, 134.999230),
        'size': 600_000,
        'crs': 6684,
        'y_is_easting': True 
    }

    conn = _setup_duckdb(DATA_FOLDER / 'nyc-taxi' / 'raw')
    query_folder = DATA_FOLDER / 'nyc-taxi' / 'queries'

    generate_datasets(conn, None, DATA_FOLDER, 'nyc-taxi')
    generate_datasets(conn, query_folder, DATA_FOLDER, 'syracuse-taxi', **syracuse_settings)
    generate_datasets(conn, query_folder, DATA_FOLDER, 'aogaki-taxi', **aogaki_settings)
    generate_datasets(conn, query_folder, DATA_FOLDER, 'germany-taxi', **germany_settings)
    generate_datasets(conn, query_folder, DATA_FOLDER, 'japan-taxi', **japan_settings)
