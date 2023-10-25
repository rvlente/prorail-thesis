from pathlib import Path
import geopandas as gpd
import pandas as pd
from shapely.geometry import LineString
import numpy as np

import time

DATA_FOLDER = Path("../data/prorail")
CRS = "EPSG:28992"

df_mtps = pd.read_parquet(DATA_FOLDER / "mtps_2023_10_02_12.parquet")


def to_point_gpd(df):
    return gpd.GeoDataFrame(
        df[["longitude", "latitude"]],
        geometry=gpd.points_from_xy(df.longitude, df.latitude),
        crs=CRS,
    )


df_spoortak = (
    gpd.read_parquet(DATA_FOLDER / "spoortakken.parquet").to_crs(CRS).drop_duplicates()
)


# Further split up the tracks in 100m sections.
def segmentize(line, d=100):
    return LineString(
        [line.interpolate(l) for l in np.arange(0, line.length, d)]
        + [line.boundary.geoms[1]]
    )


def segments(curve):
    return list(map(LineString, zip(curve.coords[:-1], curve.coords[1:])))


df_spoortak["segments"] = df_spoortak.geometry.apply(lambda x: segments(segmentize(x)))

# Create DataFrame for track segments.
df_segments = df_spoortak.explode("segments")

# Add incrementing index.
df_segments["segment_id"] = (
    df_segments.groupby("geometry").cumcount().astype(str).values
)

# Set segment as row geometry and select relevant rows.
df_segments.drop(columns=["geometry"], inplace=True)
df_segments.rename(
    columns={"segments": "geometry", "NAAM": "branch_name"}, inplace=True
)
df_segments.set_geometry("geometry", drop=True, inplace=True, crs=CRS)
df_segments = df_segments[["branch_name", "geometry", "segment_id"]]

df_segments.head()

for i in [1, 6, 24, 72, 168]:
    print(f"Performing join on {i}x data...")
    df_points = to_point_gpd(pd.concat([df_mtps] * i, ignore_index=True))
    l = len(df_points)

    st = time.time()
    x = df_points.sjoin_nearest(
        df_segments, how="left", max_distance=4, distance_col="distance"
    )
    et = time.time()
    del x
    del df_points

    td = et - st
    print(f"Done: joining {l} points took {td} seconds")
