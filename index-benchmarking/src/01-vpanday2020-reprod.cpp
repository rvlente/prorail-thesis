#include <chrono>
#include <memory>
#include <tuple>
#include <iostream>
#include "geos/quadtree.h"
#include "geos/strtree.h"
#include "s2/s2pointindex.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <data_file>" << std::endl;
        return 0;
    }

    const char *data_file = argv[1];

    // Load nyc-taxi points from file.
    std::cout << "Loading nyc-taxi dataset... " << std::flush;
    auto points = load_coordinates(data_file);
    std::cout << "Done." << std::endl;

    // Run benchmarks.
    auto s2_points = create_s2_points(points);
    benchmark_s2pointindex(s2_points);
    s2_points.clear();

    auto geos_points = create_geos_points(points);
    benchmark_strtree(geos_points);
    benchmark_quadtree(geos_points);
    geos_points.clear();

    return 0;
}
