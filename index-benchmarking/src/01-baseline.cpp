#include "experiments/geos/strtree.h"
#include "experiments/geos/quadtree.h"
#include "experiments/s2/pointindex.h"

int main(int argc, char **argv)
{
    std::string data_file_30m = "../data/nyc-taxi/nyc-taxi-30m.bin";
    std::string data_file_100m = "../data/nyc-taxi/nyc-taxi-100m.bin";
    std::string data_file_180m = "../data/nyc-taxi/nyc-taxi-180m.bin";
    std::string data_file_300m = "../data/nyc-taxi/nyc-taxi-300m.bin";
    std::vector<std::string> distance_query_files = {
        "../data/nyc-taxi/queries/taxi_distance_0.0001.csv",
        "../data/nyc-taxi/queries/taxi_distance_0.001.csv",
        "../data/nyc-taxi/queries/taxi_distance_0.01.csv",
        "../data/nyc-taxi/queries/taxi_distance_0.1.csv",
        "../data/nyc-taxi/queries/taxi_distance_1.csv",
        "../data/nyc-taxi/queries/taxi_distance_10.csv",
        "../data/nyc-taxi/queries/taxi_distance_100.csv"};
    std::vector<std::string> range_query_files = {
        "../data/nyc-taxi/queries/taxi_range_0.0001.csv",
        "../data/nyc-taxi/queries/taxi_range_0.001.csv",
        "../data/nyc-taxi/queries/taxi_range_0.01.csv",
        "../data/nyc-taxi/queries/taxi_range_0.1.csv",
        "../data/nyc-taxi/queries/taxi_range_1.csv",
        "../data/nyc-taxi/queries/taxi_range_10.csv",
        "../data/nyc-taxi/queries/taxi_range_100.csv"};

    auto strtree_runner = STRtreeExperimentRunner("geos_strtree", "EPSG:32118");
    strtree_runner.run("nyc-taxi-30m", data_file_30m, distance_query_files, range_query_files, argv[0]);
    strtree_runner.run("nyc-taxi-100m", data_file_100m, distance_query_files, range_query_files, argv[0]);
    strtree_runner.run("nyc-taxi-180m", data_file_180m, distance_query_files, range_query_files, argv[0]);
    strtree_runner.run("nyc-taxi-300m", data_file_300m, distance_query_files, range_query_files, argv[0]);

    auto quadtree_runner = QuadtreeExperimentRunner("geos_quadtree", "EPSG:32118");
    quadtree_runner.run("nyc-taxi-30m", data_file_30m, distance_query_files, range_query_files, argv[0]);
    quadtree_runner.run("nyc-taxi-100m", data_file_100m, distance_query_files, range_query_files, argv[0]);
    quadtree_runner.run("nyc-taxi-180m", data_file_180m, distance_query_files, range_query_files, argv[0]);
    quadtree_runner.run("nyc-taxi-300m", data_file_300m, distance_query_files, range_query_files, argv[0]);

    auto s2pointindex_runner = S2PointIndexExperimentRunner("s2_pointindex");
    s2pointindex_runner.run("nyc-taxi-30m", data_file_30m, distance_query_files, range_query_files, argv[0]);
    s2pointindex_runner.run("nyc-taxi-100m", data_file_100m, distance_query_files, range_query_files, argv[0]);
    s2pointindex_runner.run("nyc-taxi-180m", data_file_180m, distance_query_files, range_query_files, argv[0]);
    s2pointindex_runner.run("nyc-taxi-300m", data_file_300m, distance_query_files, range_query_files, argv[0]);

    return 0;
}
