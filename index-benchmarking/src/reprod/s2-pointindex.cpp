#include <memory>
#include <chrono>
#include "s2/s2point.h"
#include "s2/s2closest_point_query.h"
#include "s2/s2earth.h"
#include "utils/gdal.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <geometry_file>" << std::endl;
        return 0;
    }

    const char *geometry_file = argv[1];
    std::cout << "Building index with shapes from [" << geometry_file << "]... " << std::flush;

    // Build index.
    S2PointIndex<int> index;

    auto iter = GDALGeometryIterator::fromGPKG(geometry_file, "nyctaxi");
    auto start = std::chrono::high_resolution_clock::now();

    while (iter->hasNext())
    {
        auto geom = iter->next();
        auto point = geom->getPoint();

        if (point == nullptr)
        {
            std::cout << "Failed to get point." << std::endl;
            return 0;
        }

        index.Add(S2LatLng::FromDegrees(point->getX(), point->getY()).ToPoint(), 0);
    }

    // S2PointIndex is lazy, and will only build the index when a query is performed.
    S2ClosestPointQuery<int> query(&index);
    query.mutable_options()->set_max_distance(
        S1Angle::Radians(S2Earth::KmToRadians(0.001)));

    auto end = std::chrono::high_resolution_clock::now();

    // Report.
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Finished. Built index of size " << index.SpaceUsed() << " bytes in " << duration << " ms." << std::endl;

    return 0;
}
