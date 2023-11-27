import requests
import duckdb


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


def create_gpkg():
    create_data_query = """
    DROP TABLE IF EXISTS nyctaxi_2009;
    DROP TABLE IF EXISTS nyctaxi_2010;
    DROP TABLE IF EXISTS nyctaxi;

    CREATE TABLE nyctaxi_2009 AS
    SELECT Start_Lon AS lon, Start_Lat AS lat
    FROM 'data/nyc-taxi/yellow_tripdata_2009-*.parquet';

    CREATE TABLE nyctaxi_2010 AS
    SELECT pickup_longitude AS lon, pickup_latitude AS lat
    FROM 'data/nyc-taxi/yellow_tripdata_2010-*.parquet';

    CREATE TABLE nyctaxi AS
    SELECT ST_Point(lat, lon) FROM (
        SELECT lat, lon FROM nyctaxi_2009
        UNION ALL BY NAME
        SELECT lat, lon FROM nyctaxi_2010
    );
    """

    write_data_query = """
    COPY nyctaxi TO 'data/nyctaxi.gpkg' WITH (
        FORMAT GDAL,
        DRIVER 'GPKG',
        LAYER_CREATION_OPTIONS 'SPATIAL_INDEX=NO'
    );
    """

    conn = duckdb.connect(database=":memory:", read_only=False)
    conn.execute("INSTALL spatial; LOAD spatial;")
    conn.execute(create_data_query)
    conn.execute(write_data_query)
    conn.close()


if __name__ == "__main__":
    download_files()
    create_gpkg()
