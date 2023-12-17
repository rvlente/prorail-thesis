#include <chrono>
#include <memory>
#include <tuple>
#include <iostream>
#include "geos/quadtree.h"
#include "geos/strtree.h"
#include "s2/pointindex.h"
#include "s2/shapeindex.h"
#include "utils/data.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <data_file> <dq_file> <rq_file>" << std::endl;
        return 1;
    }

    const char *data_file = argv[1];
    const char *distance_query_file = argv[2];
    const char *range_query_file = argv[3];

    // Load nyc-taxi points from file.
    std::cout << "Loading aogaki-taxi dataset... " << std::flush;
    auto points = load_coordinates(data_file);
    std::cout << "Done." << std::endl;

    // Load queries.
    std::cout << "Loading queries... " << std::flush;
    auto distance_queries = load_distance_queries(distance_query_file);
    auto range_queries = load_range_queries(range_query_file);
    std::cout << "Done." << std::endl
              << std::endl
              << "Running benchmarks..." << std::endl
              << std::endl;

    // Run benchmarks.
    auto s2_points = create_s2_points(points);
    benchmark_s2pointindex(s2_points, distance_queries, range_queries);
    benchmark_s2shapeindex(s2_points, distance_queries, range_queries);

    auto geos_points = create_geos_points(points, "EPSG:6684"); // Japan projection

    benchmark_strtree(geos_points);
    benchmark_quadtree(geos_points);

    geos_points.clear();

    return 0;
}
