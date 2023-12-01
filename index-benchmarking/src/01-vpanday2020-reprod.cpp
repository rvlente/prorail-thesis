#include <chrono>
#include <memory>
#include <tuple>
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "geos/index/strtree/STRtree.h"
#include "geos/index/quadtree/Quadtree.h"
#include "geos/geom/GeometryFactory.h"
#include "utils/bin.h"
#include "utils/progress.h"
#include "utils/proj.h"

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
    std::cout << "Finished. Building S2PointIndex..." << std::endl;

    S2PointIndex<int> index;
    ProgressBar progress2(s2points.size());
    progress2.start();

    for (int i = 0; i < s2points.size(); i++)
    {
        index.Add(s2points[i], i);
        progress2.update(i);
    }

    progress2.finish();
    std::cout << "Finished. Built index of size " << index.SpaceUsed() << " bytes." << std::endl
              << std::endl;
}

std::vector<std::unique_ptr<geos::geom::Point>> _get_geos_points(const std::vector<Coord> &points)
{
    std::vector<std::unique_ptr<geos::geom::Point>> geos_points;
    auto factory = geos::geom::GeometryFactory::create();
    ProjWrapper transformer("EPSG:4326", "EPSG:3857");

    ProgressBar progress1(points.size());
    progress1.start();

    for (int i = 0; i < points.size(); i++)
    {
        auto xy = transformer.transform(points[i].lat, points[i].lon);
        geos_points.push_back(factory->createPoint(geos::geom::Coordinate(std::get<0>(xy), std::get<1>(xy))));
        progress1.update(i);
    }

    progress1.finish();

    return geos_points;
}

void benchmarkGeosSTRtree(const std::vector<Coord> &points)
{
    std::cout << "Creating GEOS Geometry from coordinates..." << std::endl;
    auto geos_points = _get_geos_points(points);

    std::cout << "Finished. Building STRtree..." << std::endl;

    geos::index::strtree::STRtree index;
    ProgressBar progress2(geos_points.size());
    progress2.start();

    for (int i = 0; i < geos_points.size(); i++)
    {
        index.insert(geos_points[i]->getEnvelopeInternal(), nullptr);
        progress2.update(i);
    }

    index.build();
    progress2.finish();

    std::cout << "Finished. Built index of size " << sizeof(index) << " bytes." << std::endl
              << std::endl;
}

void benchmarkGeosQuadtree(const std::vector<Coord> &points)
{
    std::cout << "Creating GEOS Geometry from coordinates..." << std::endl;
    auto geos_points = _get_geos_points(points);

    std::cout << "Finished. Building Quadtree..." << std::endl;

    geos::index::quadtree::Quadtree index;

    ProgressBar progress2(geos_points.size());
    progress2.start();

    for (int i = 0; i < geos_points.size(); i++)
    {
        index.insert(geos_points[i]->getEnvelopeInternal(), reinterpret_cast<void *>(i));
        progress2.update(i);
    }

    progress2.finish();

    std::cout << "Finished. Built index of size " << sizeof(index) << " bytes." << std::endl
              << std::endl;
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
    benchmarkGeosSTRtree(points);
    benchmarkGeosQuadtree(points);

    return 0;
}
