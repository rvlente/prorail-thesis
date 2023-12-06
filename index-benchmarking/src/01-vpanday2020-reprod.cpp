#include <chrono>
#include <memory>
#include <tuple>
#include <gperftools/heap-profiler.h>
#include "s2/s2latlng.h"
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "geos/index/strtree/STRtree.h"
#include "geos/index/quadtree/Quadtree.h"
#include "geos/geom/GeometryFactory.h"
#include "utils/bin.h"
#include "utils/progress.h"
#include "utils/proj.h"

void benchmark_s2pointindex(const std::vector<Coord> &points)
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

std::vector<std::unique_ptr<geos::geom::Point>> get_geos_points(const std::vector<Coord> &points)
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

void build_strtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    ProgressBar progress(points.size() * 2); // Geometry insertion takes up less than half of the total progress.
    progress.start();

    auto index = new geos::index::strtree::STRtree();

    for (int i = 0; i < points.size(); i++)
    {
        index->insert(points[i]->getEnvelopeInternal(), nullptr);
        progress.update(i);
    }

    progress.finish();
}

void benchmark_strtree(const std::vector<Coord> &points)
{
    std::cout << "Creating GEOS Geometry from coordinates..." << std::endl;
    auto geos_points = get_geos_points(points);

    std::cout << "Finished. Building STRtree..." << std::endl;

    HeapProfilerStart("heapprofile/strtree");

    build_strtree(geos_points);

    HeapProfilerStop();

    std::cout << "Finished. Check strtree heap profile for memory usage." << std::endl
              << std::endl;
}

void build_quadtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    ProgressBar progress(points.size() * 2); // Geometry insertion takes up less than half of the total progress.
    progress.start();

    geos::index::quadtree::Quadtree index;

    for (int i = 0; i < points.size(); i++)
    {
        index.insert(points[i]->getEnvelopeInternal(), reinterpret_cast<void *>(i));
        progress.update(i);
    }

    progress.finish();
}

void benchmark_quadtree(const std::vector<Coord> &points)
{
    std::cout << "Creating GEOS Geometry from coordinates..." << std::endl;
    auto geos_points = get_geos_points(points);

    std::cout << "Finished. Building Quadtree..." << std::endl;

    HeapProfilerStart("heapprofile/quadtree");

    build_quadtree(geos_points);

    HeapProfilerStop();

    std::cout << "Finished. Check quadtree heap profile for memory usage." << std::endl
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
    std::cout << "Loading nyc-taxi dataset... " << std::flush;
    auto points = load_coordinates(data_file);
    std::cout << "Done." << std::endl;

    // Run benchmarks.
    benchmark_s2pointindex(points);
    benchmark_strtree(points);
    benchmark_quadtree(points);

    return 0;
}
