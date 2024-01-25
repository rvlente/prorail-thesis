import osmnx  # as antigravity (https://xkcd.com/353/)
import struct
from pathlib import Path


def create_binary(target_file, n_points, radius, settings):
    G = osmnx.graph_from_point(settings['center'], network_type="drive", dist=radius)
    G = osmnx.projection.project_graph(G, to_crs=settings['crs'])
    points = osmnx.utils_geo.sample_points(G, n_points)
    points = osmnx.projection.project_gdf(points, to_latlong=True)

    with open(target_file, "wb") as f:
        for p in points:
            f.write(struct.pack("d", p.x))
            f.write(struct.pack("d", p.y))



if __name__ == '__main__':
    DATA_FOLDER = Path('data/synthetic')

    N_POINTS = 25_000_000
    RADIUS = 10_000

    nyc_settings = {
        'center': (40.712214, -74.004800),
        'crs': 32118,
    }

    delhi_settings = {
        'center': (43.054477, -76.144178),
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
    create_binary(DATA_FOLDER / 'delhi' / 'delhi-25m.bin', N_POINTS, RADIUS, delhi_settings)
    create_binary(DATA_FOLDER / 'tokyo' / 'tokyo-25m.bin', N_POINTS, RADIUS, tokyo_settings)
    create_binary(DATA_FOLDER / 'sao-paolo' / 'sao-paolo-25m.bin', N_POINTS, RADIUS, sao_paolo_settings)
