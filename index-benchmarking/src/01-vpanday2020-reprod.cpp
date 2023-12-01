#include <chrono>
#include <memory>
#include <tuple>
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "utils/bin.h"
#include "utils/progress.h"

void benchmarkS2PointIndex(const std::vector<Coord> &points)
{
    std::cout << "Creating S2Points from coordinates..." << std::endl;

    std::vector<S2Point> s2points;
    ProgressBar progress1(points.size());
    progress1.start();

    for (int i = 0; i < points.size(); i++)
    {
        s2points.push_back(S2LatLng::FromDegrees(points[i].lat, points[i].lon).ToPoint());
        progress1.update(i);
    }

    progress1.finish();
    std::cout << "Finished. Building index..." << std::endl;

    S2PointIndex<int> index;
    ProgressBar progress2(s2points.size());
    progress2.start();

    for (int i = 0; i < s2points.size(); i++)
    {
        index.Add(s2points[i], i);
        progress2.update(i);
    }

    progress2.finish();
    std::cout << "Finished. Built index of size " << index.SpaceUsed() << " bytes." << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <data_file>" << std::endl;
        return 0;
    }

    const char *data_file = argv[1];

    // Load nyc-taxi points from file.
    std::cout << "Loading nyc-taxi dataset..." << std::flush;
    auto points = loadCoordinatesFromFile(data_file);
    std::cout << "Done." << std::endl;

    // Run benchmarks.
    benchmarkS2PointIndex(points);

    return 0;
}
