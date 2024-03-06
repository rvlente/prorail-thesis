#include "experiments/geos/strtree.h"
#include "experiments/geos/quadtree.h"
#include "experiments/s2/pointindex.h"

int main(int argc, char **argv)
{
    std::string data_file = "../data/synthetic/delhi/delhi-10m.bin";

    std::vector<std::string> distance_query_files = {
        "../data/synthetic/delhi/queries/synthetic_distance_0.1.csv",
    };
    std::vector<std::string> range_query_files = {
        "../data/synthetic/delhi/queries/synthetic_range_0.1.csv",
    };

    auto strtree_runner = STRtreeExperimentRunner("23__geos_strtree", "EPSG:24378", argv[0]);
    strtree_runner.run("synthetic-delhi-10m", data_file, distance_query_files, range_query_files);

    auto quadtree_runner = QuadtreeExperimentRunner("23__geos_quadtree", "EPSG:24378", argv[0]);
    quadtree_runner.run("synthetic-delhi-10m", data_file, distance_query_files, range_query_files);

    auto s2pointindex_runner = S2PointIndexExperimentRunner("23__s2_pointindex", argv[0]);
    s2pointindex_runner.run("synthetic-delhi-10m", data_file, distance_query_files, range_query_files);

    return 0;
}
