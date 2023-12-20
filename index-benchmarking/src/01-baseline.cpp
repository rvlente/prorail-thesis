#include "experiments/geos/strtree.h"
#include "experiments/geos/quadtree.h"
#include "experiments/s2/pointindex.h"
#include "experiments/s2/shapeindex.h"

int main(int argc, char **argv)
{
    const char *data_file_30m = "../data/nyc-taxi/nyc-taxi-30m.bin";
    const char *data_file_100m = "../data/nyc-taxi/nyc-taxi-100m.bin";
    const char *data_file_full = "../data/nyc-taxi/nyc-taxi-full.bin";
    const char *distance_query_file = "../data/nyc-taxi/queries/taxi_distance_0.0001.csv";
    const char *range_query_file = "../data/nyc-taxi/queries/taxi_range_0.0001.csv";

    auto strtree_runner = STRtreeExperimentRunner("geos_strtree", "EPSG:32118");
    strtree_runner.run("nyc-taxi-30m", data_file_30m, distance_query_file, range_query_file, argv[0]);
    strtree_runner.run("nyc-taxi-100m", data_file_100m, distance_query_file, range_query_file, argv[0]);
    strtree_runner.run("nyc-taxi-full", data_file_30m, distance_query_file, range_query_file, argv[0]);

    auto quadtree_runner = QuadtreeExperimentRunner("geos_quadtree", "EPSG:32118");
    quadtree_runner.run("nyc-taxi-30m", data_file_30m, distance_query_file, range_query_file, argv[0]);
    quadtree_runner.run("nyc-taxi-100m", data_file_100m, distance_query_file, range_query_file, argv[0]);
    quadtree_runner.run("nyc-taxi-full", data_file_full, distance_query_file, range_query_file, argv[0]);

    auto s2pointindex_runner = S2PointIndexExperimentRunner("s2_pointindex");
    s2pointindex_runner.run("nyc-taxi-30m", data_file_30m, distance_query_file, range_query_file, argv[0]);
    s2pointindex_runner.run("nyc-taxi-100m", data_file_100m, distance_query_file, range_query_file, argv[0]);
    s2pointindex_runner.run("nyc-taxi-full", data_file_full, distance_query_file, range_query_file, argv[0]);

    return 0;
}
