#include "experiments/geos/strtree.h"
#include "experiments/geos/quadtree.h"
#include "experiments/s2/pointindex.h"

int main(int argc, char **argv)
{
    std::string data_file_0_25m = "../data/nyc-taxi/nyc-taxi-0_25m.bin";
    std::string data_file_2_5m = "../data/nyc-taxi/nyc-taxi-2_5m.bin";
    std::string data_file_25m = "../data/nyc-taxi/nyc-taxi-25m.bin";
    std::string data_file_250m = "../data/nyc-taxi/nyc-taxi-250m.bin";

    std::vector<std::string> distance_query_files = {
        "../data/nyc-taxi/queries/taxi_distance_0.0001.csv",
        "../data/nyc-taxi/queries/taxi_distance_0.001.csv",
        "../data/nyc-taxi/queries/taxi_distance_0.01.csv",
        "../data/nyc-taxi/queries/taxi_distance_0.1.csv",
        "../data/nyc-taxi/queries/taxi_distance_1.csv",
    };
    std::vector<std::string> range_query_files = {
        "../data/nyc-taxi/queries/taxi_range_0.0001.csv",
        "../data/nyc-taxi/queries/taxi_range_0.001.csv",
        "../data/nyc-taxi/queries/taxi_range_0.01.csv",
        "../data/nyc-taxi/queries/taxi_range_0.1.csv",
        "../data/nyc-taxi/queries/taxi_range_1.csv",
    };

    const std::vector<std::string> fixed_distance_query_file = {distance_query_files[3]};
    const std::vector<std::string> fixed_range_query_file = {range_query_files[3]};

    auto strtree_runner = STRtreeExperimentRunner("geos_strtree", "EPSG:32118", argv[0]);
    strtree_runner.run("nyc-taxi-0_25m", data_file_0_25m, fixed_distance_query_file, fixed_range_query_file);
    strtree_runner.run("nyc-taxi-2_5m", data_file_2_5m, fixed_distance_query_file, fixed_range_query_file);
    strtree_runner.run("nyc-taxi-25m", data_file_25m, fixed_distance_query_file, fixed_range_query_file);
    strtree_runner.run("nyc-taxi-250m", data_file_250m, distance_query_files, range_query_files);

    auto quadtree_runner = QuadtreeExperimentRunner("geos_quadtree", "EPSG:32118", argv[0]);
    quadtree_runner.run("nyc-taxi-0_25m", data_file_0_25m, fixed_distance_query_file, fixed_range_query_file);
    quadtree_runner.run("nyc-taxi-2_5m", data_file_2_5m, fixed_distance_query_file, fixed_range_query_file);
    quadtree_runner.run("nyc-taxi-25m", data_file_25m, fixed_distance_query_file, fixed_range_query_file);
    quadtree_runner.run("nyc-taxi-250m", data_file_250m, distance_query_files, range_query_files);

    auto s2pointindex_runner = S2PointIndexExperimentRunner("s2_pointindex", argv[0]);
    s2pointindex_runner.run("nyc-taxi-0_25m", data_file_0_25m, fixed_distance_query_file, fixed_range_query_file);
    s2pointindex_runner.run("nyc-taxi-2_5m", data_file_2_5m, fixed_distance_query_file, fixed_range_query_file);
    s2pointindex_runner.run("nyc-taxi-25m", data_file_25m, fixed_distance_query_file, fixed_range_query_file);
    s2pointindex_runner.run("nyc-taxi-250m", data_file_250m, distance_query_files, range_query_files);

    return 0;
}
