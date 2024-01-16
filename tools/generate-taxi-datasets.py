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


# def create_gpkg(raw_folder, target_file, limit=None):
#     write_data_query = f"""
#     COPY (SELECT ST_Point(lon, lat) FROM nyctaxi{'' if limit is None else f' USING SAMPLE {limit}'}) 
#     TO '{target_file}' WITH (
#         FORMAT GDAL,
#         DRIVER 'GPKG',
#         LAYER_CREATION_OPTIONS 'SPATIAL_INDEX=NO'
#     );
#     """

#     conn = _setup_duckdb(raw_folder)
#     conn.execute("INSTALL spatial; LOAD spatial;")
#     conn.execute(write_data_query)
#     conn.close()


def create_binary(conn, target_file, limit=None, size=None, center=None, crs=None, y_is_easting=False):
    """
    Create a binary file of points from the nyc-taxi dataset.
    """

    if size is not None:
        assert isinstance(size, int)
        assert isinstance(crs, int)

    if center is not None:
        assert len(center) == 2
        assert isinstance(crs, int) 

    query = f"SELECT lat, lon FROM nyctaxi{'' if limit is None else f' USING SAMPLE {limit}'};"
    df = conn.sql(query).pl()

    if size is not None or center is not None:
        # If resizing or translating, we first convert to cartesian coordinates.
        print("Converting to cartesian...")

        tf_cart = Transformer.from_crs(4326, 32118)
        
        df = df.with_columns(
            pl.struct(['lat', 'lon']) \
            .map_batches(lambda st: pl.Series(zip(*tf_cart.transform(st.struct.field('lat'), st.struct.field('lon'))))) \
            .alias('xy'))
        
        df = df.with_columns(pl.col('xy').list.to_struct()).unnest('xy').rename({'field_0': 'x', 'field_1': 'y'})

        # Next, we normalize the coordinates, preserving the ratio.
        print("Normalizing...")

        x_size = df.select(pl.max('x') - pl.min('x')).item()
        y_size = df.select(pl.max('y') - pl.min('y')).item()
        x_center = df.select(pl.mean('x')).item()
        y_center = df.select(pl.mean('y')).item()

        max_size = max(x_size, y_size)

        df = df.with_columns([
            ((pl.col('x') - pl.mean('x')) / max_size).alias('x_norm'),
            ((pl.col('y') - pl.mean('y')) / max_size).alias('y_norm'),
        ])
    
        # If not resizing, we set the new size to the original.
        if size is None:
            size = max_size
        
        # If translating, we compute the new center point within the given CRS.
        if center is not None:
            tf = Transformer.from_crs(4326, crs)
            x_center, y_center = tf.transform(*center)

        # Finally, we apply the transformations.
        print('Applying...')
        tf_latlon = Transformer.from_crs(crs, 4326)

        if y_is_easting:
            df = df.rename({'x_norm': 'y_norm', 'y_norm': 'x_norm'})

        df = df.with_columns([
                   (pl.col('x_norm') * size + x_center).alias('x_new'),
                   (pl.col('y_norm') * size + y_center).alias('y_new'),
               ]) \
               .with_columns(pl.struct(['x_new', 'y_new']) \
                    .map_batches(lambda st: 
                                 pl.Series(zip(*tf_latlon.transform(st.struct.field('x_new'), st.struct.field('y_new'))))
                                 ).alias('latlon')) \
               .with_columns(pl.col('latlon').list.to_struct()).unnest('latlon').rename({'field_0': 'lat_new', 'field_1': 'lon_new'})
        
        df = df.select(pl.col('lat_new'), pl.col('lon_new'))

    with open(target_file, "wb") as f:
        print('Writing output...')
        for lat, lon in df.iter_rows():
            f.write(struct.pack("d", lat))
            f.write(struct.pack("d", lon))


def translate_queries(raw_folder, target_folder, translate):
    for qf in raw_folder.glob('*_distance_*.csv'):
        with qf.open() as in_f, open(target_folder / qf.name, 'w') as out_f:
            while in_l := in_f.readline():
                lat, lon, d = in_l.strip().split(',')
                out_l = f'{float(lat) + translate[0]},{float(lon) + translate[1]},{d}\n'
                out_f.write(out_l)
    
    for rf in raw_folder.glob('*_range_*.csv'):
        with rf.open() as in_f, open(target_folder / rf.name, 'w') as out_f:
            while in_l := in_f.readline():
                lata, lona, latb, lonb = in_l.strip().split(',')
                out_l = f'{float(lata) + translate[0]},{float(lona) + translate[1]},{float(latb) + translate[0]},{float(lonb) + translate[1]}\n'
                out_f.write(out_l)


def generate_datasets(conn, data_folder, name, settings={}):
    create_binary(conn, data_folder / name / f'{name}-0_25m.bin', 250_000, **settings)
    create_binary(conn, data_folder / name / f'{name}-2_5m.bin', 2_500_000, **settings)
    create_binary(conn, data_folder / name / f'{name}-25m.bin', 25_000_000, **settings)
    create_binary(conn, data_folder / name / f'{name}-250m.bin', 250_000_000, **settings)


if __name__ == '__main__':
    DATA_FOLDER = Path('data')

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
    generate_datasets(conn, DATA_FOLDER, 'nyc-taxi')
    generate_datasets(conn, DATA_FOLDER, 'aogaki-taxi', aogaki_settings)
    generate_datasets(conn, DATA_FOLDER, 'germany-taxi', germany_settings)
    generate_datasets(conn, DATA_FOLDER, 'japan-taxi', japan_settings)
