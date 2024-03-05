#include "experiments/geos/strtree.h"
#include "experiments/geos/quadtree.h"
#include "experiments/s2/pointindex.h"

int main(int argc, char **argv)
{
    std::string data_file_25m = "../data/synthetic/saopaolo/saopaolo-25m.bin";

    std::vector<std::string> distance_query_files = {
        "../data/synthetic/saopaolo/queries/synthetic_distance_0.1.csv",
    };
    std::vector<std::string> range_query_files = {
        "../data/synthetic/saopaolo/queries/synthetic_range_0.1.csv",
    };

    auto strtree_runner = STRtreeExperimentRunner("24__geos_strtree", "EPSG:32118", argv[0]);
    strtree_runner.run("synthetic-saopaolo-25m", data_file_25m, distance_query_files, range_query_files);

    auto quadtree_runner = QuadtreeExperimentRunner("24__geos_quadtree", "EPSG:32118", argv[0]);
    quadtree_runner.run("synthetic-saopaolo-25m", data_file_25m, distance_query_files, range_query_files);

    auto s2pointindex_runner = S2PointIndexExperimentRunner("24__s2_pointindex", argv[0]);
    s2pointindex_runner.run("synthetic-saopaolo-25m", data_file_25m, distance_query_files, range_query_files);

    return 0;
}
