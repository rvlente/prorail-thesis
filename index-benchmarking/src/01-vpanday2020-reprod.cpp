#include <chrono>
#include <memory>
#include <tuple>
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "utils/bin.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <data_file>" << std::endl;
        return 0;
    }

    const char *data_file = argv[1];

    // Load coordinates from file.
    std::cout << "Loading nyc-taxi dataset..." << std::endl;
    auto coordinates = loadCoordinatesFromFile(data_file);

    // Create S2 points.
    std::vector<S2Point> s2points;

    for (const auto &coord : coordinates)
    {
        s2points.push_back(S2LatLng::FromDegrees(coord.lat, coord.lon).ToPoint());
    }

    std::cout << "Finished." << std::endl
              << "Building index..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    typedef u_char _T;
    S2PointIndex<_T> index;

    for (const auto &point : s2points)
    {
        index.Add(point, 0);
    }

    auto end = std::chrono::high_resolution_clock::now();

    // Report.
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Finished. Built index of size " << index.SpaceUsed() << " bytes in " << duration << " ms." << std::endl;

    return 0;
}
