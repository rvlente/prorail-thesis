#include "experiments/geos/strtree.h"
#include "experiments/geos/quadtree.h"
#include "experiments/s2/pointindex.h"

int main(int argc, char **argv)
{
    std::string data_file_30m = "../data/nyc-taxi/aogaki-taxi-30m.bin";
    std::string data_file_100m = "../data/nyc-taxi/aogaki-taxi-100m.bin";
    std::string data_file_180m = "../data/nyc-taxi/aogaki-taxi-180m.bin";
    std::string data_file_full = "../data/nyc-taxi/aogaki-taxi-full.bin";
    std::string distance_query_file = "../data/nyc-taxi/queries/taxi_distance_0.001.csv";
    std::string range_query_file = "../data/nyc-taxi/queries/taxi_range_0.001.csv";

    auto strtree_runner = STRtreeExperimentRunner("geos_strtree", "EPSG:6684");
    strtree_runner.run("aogaki-taxi-30m", data_file_30m, distance_query_file, range_query_file, argv[0]);
    strtree_runner.run("aogaki-taxi-100m", data_file_100m, distance_query_file, range_query_file, argv[0]);
    strtree_runner.run("aogaki-taxi-180m", data_file_180m, distance_query_file, range_query_file, argv[0]);
    strtree_runner.run("aogaki-taxi-full", data_file_full, distance_query_file, range_query_file, argv[0]);

    auto quadtree_runner = QuadtreeExperimentRunner("geos_quadtree", "EPSG:6684");
    quadtree_runner.run("aogaki-taxi-30m", data_file_30m, distance_query_file, range_query_file, argv[0]);
    quadtree_runner.run("aogaki-taxi-100m", data_file_100m, distance_query_file, range_query_file, argv[0]);
    quadtree_runner.run("aogaki-taxi-180m", data_file_180m, distance_query_file, range_query_file, argv[0]);
    quadtree_runner.run("aogaki-taxi-full", data_file_full, distance_query_file, range_query_file, argv[0]);

    auto s2pointindex_runner = S2PointIndexExperimentRunner("s2_pointindex");
    s2pointindex_runner.run("aogaki-taxi-30m", data_file_30m, distance_query_file, range_query_file, argv[0]);
    s2pointindex_runner.run("aogaki-taxi-100m", data_file_100m, distance_query_file, range_query_file, argv[0]);
    s2pointindex_runner.run("aogaki-taxi-180m", data_file_180m, distance_query_file, range_query_file, argv[0]);
    s2pointindex_runner.run("aogaki-taxi-full", data_file_full, distance_query_file, range_query_file, argv[0]);

    return 0;
}
