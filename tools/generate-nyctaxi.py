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
import requests
import struct
from pathlib import Path # imported for convenience


def download_files():
    for color in ["yellow"]:
        for yyyy in [2009, 2010]:
            for mm in range(1, 13):
                url = (
                    "https://d37ci6vzurychx.cloudfront.net/trip-data/"
                    f"{color}_tripdata_{yyyy}-{mm:02d}.parquet"
                )

                print(url)
                result = requests.get(url)

                with open(
                    f"data/nyc-taxi/{color}_tripdata_{yyyy}-{mm:02d}.parquet", "wb"
                ) as f:
                    f.write(result.content)


def _setup_duckdb(input_folder):
    create_data_query = f"""
    DROP TABLE IF EXISTS nyctaxi_2009;
    DROP TABLE IF EXISTS nyctaxi_2010;
    DROP TABLE IF EXISTS nyctaxi;

    CREATE TABLE nyctaxi_2009 AS
    SELECT Start_Lon AS lon, Start_Lat AS lat
    FROM '{input_folder}/yellow_tripdata_2009-*.parquet';

    CREATE TABLE nyctaxi_2010 AS
    SELECT pickup_longitude AS lon, pickup_latitude AS lat
    FROM '{input_folder}/yellow_tripdata_2010-*.parquet';

    CREATE TABLE nyctaxi AS
    SELECT lat, lon FROM (
        SELECT lat, lon FROM nyctaxi_2009
        UNION ALL BY NAME
        SELECT lat, lon FROM nyctaxi_2010
    )
    WHERE lat BETWEEN 30 AND 50 AND lon BETWEEN -80 AND -70;
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


def create_binary(raw_folder, target_file, limit=None, translate=None):
    conn = _setup_duckdb(raw_folder)
    query = f"SELECT lat{'' if translate is None else f' + {translate[0]}'}, lon{'' if translate is None else f' + {translate[0]}'} FROM nyctaxi{'' if limit is None else f' USING SAMPLE {limit}'};"
    df = conn.sql(query).pl()

    with open(target_file, "wb") as f:
        for lat, lon in df.iter_rows():
            f.write(struct.pack("d", lat))
            f.write(struct.pack("d", lon))
