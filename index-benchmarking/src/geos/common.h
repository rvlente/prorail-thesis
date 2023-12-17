#pragma once
#include <vector>
#include <memory>
#include "geos/geom/GeometryFactory.h"
#include "../utils/progress.h"
#include "../utils/proj.h"
#include "../utils/data.h"

template <typename TIndex, typename TGeom>
class IndexExperiment
{
public:
    static IndexExperiment load(const char *geomFile, const char *);

private:
    IndexExperiment()
    {
    }
};

std::vector<std::unique_ptr<geos::geom::Point>> create_geos_points(const std::vector<Coord> &points, const char *crs)
{
    std::cout << "Creating GEOS Geometry from coordinates..." << std::endl;

    std::vector<std::unique_ptr<geos::geom::Point>> geos_points;
    auto factory = geos::geom::GeometryFactory::create();
    ProjWrapper transformer("EPSG:4326", crs);

    ProgressBar progress(points.size());
    progress.start();

    for (int i = 0; i < points.size(); i++)
    {
        auto xy = transformer.transform(points[i].lat, points[i].lon);
        geos_points.push_back(factory->createPoint(geos::geom::Coordinate(std::get<0>(xy), std::get<1>(xy))));
        progress.update(i);
    }

    progress.finish();

    return geos_points;
}
