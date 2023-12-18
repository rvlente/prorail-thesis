#include <chrono>
#include <memory>
#include <tuple>
#include <iostream>
#include "experiments/s2/pointindex.h"
#include "experiments/geos/strtree.h"

int main(int argc, char **argv)
{
    const char *data_file = "../data/nyc-taxi/nyc-taxi-30m.bin";
    const char *distance_query_file = "../data/nyc-taxi/queries/taxi_distance_0.0001.csv";
    const char *range_query_file = "../data/nyc-taxi/queries/taxi_range_0.0001.csv";

    auto s2pointindex_runner = S2PointIndexExperimentRunner("s2pointindex");
    s2pointindex_runner.run("nyc-taxi-30m", data_file, distance_query_file, range_query_file, argv[0]);

    auto strtree_runner = STRtreeExperimentRunner("geos_strtree", "EPSG:32118");
    strtree_runner.run("nyc-taxi-30m", data_file, distance_query_file, range_query_file, argv[0]);

    return 0;
}
