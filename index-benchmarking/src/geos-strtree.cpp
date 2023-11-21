#include <memory>
#include <chrono>
#include "geos/index/strtree/STRtree.h"
#include "geos/geom/GeometryFactory.h"
#include "shp-utils.h"
#include "proj-utils.h"

using namespace geos;
using index::strtree::STRtree;

geom::CoordinateSequence getCoordinateSequence(const ProjWrapper *coordinateTransformer, SHPObject_ptr obj)
{
    geom::CoordinateSequence seq;

    for (int i = 0; i < obj->nVertices; i++)
    {
        double x, y;
        std::tie(x, y) = coordinateTransformer->transform(std::make_tuple(obj->padfX[i], obj->padfY[i]));
        seq.add(x, y);
    }

    return seq;
}

std::unique_ptr<geom::Geometry> makeGeosPoint(const ProjWrapper *coordinateTransformer, const geom::GeometryFactory *factory, SHPObject_ptr obj)
{
    auto seq = getCoordinateSequence(coordinateTransformer, std::move(obj));
    return factory->createPoint(seq[0]);
}

std::unique_ptr<geom::Geometry> makeGeosLine(const ProjWrapper *coordinateTransformer, const geom::GeometryFactory *factory, SHPObject_ptr obj)
{
    auto seq = getCoordinateSequence(coordinateTransformer, std::move(obj));
    return factory->createLineString(seq);
}

std::unique_ptr<geom::Geometry> makeGeosPolygon(const ProjWrapper *coordinateTransformer, const geom::GeometryFactory *factory, SHPObject_ptr obj)
{
    auto seq = getCoordinateSequence(coordinateTransformer, std::move(obj));
    auto geosLinearRing = factory->createLinearRing(seq);
    return factory->createPolygon(std::move(geosLinearRing));
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <shapefile>" << std::endl;
        return 0;
    }

    const char *shapefile = argv[1];
    std::cout << "Loading shapes from shapefile [" << shapefile << "]... " << std::flush;

    // Read shapes from shapefile.
    ShapeType shapeType;
    auto shapes = loadShapefile(shapefile, &shapeType);

    if (shapes.empty())
    {
        std::cout << "Failed to load shapes from shapefile [" << shapefile << "]." << std::endl;
        return 0;
    }

    // Convert shapes to Geometry shapes.
    std::vector<std::unique_ptr<geom::Geometry>> geosShapes;

    std::function<std::unique_ptr<geom::Geometry>(const ProjWrapper *, const geom::GeometryFactory *, SHPObject_ptr)> makeGeosGeometry;
    std::string shapeName;

    switch (shapeType)
    {
    case ShapeType::Point:
        makeGeosGeometry = makeGeosPoint;
        shapeName = "points";
        break;
    case ShapeType::Line:
        makeGeosGeometry = makeGeosLine;
        shapeName = "lines";
        break;
    case ShapeType::Polygon:
        makeGeosGeometry = makeGeosPolygon;
        shapeName = "polygons";
        break;
    }

    auto factory = geom::GeometryFactory::create();
    ProjWrapper transformer("EPSG:4326", "EPSG:3857");

    for (auto &shape : shapes)
    {
        geosShapes.push_back(makeGeosGeometry(&transformer, factory.get(), std::move(shape)));
    }

    std::cout << "Finished. Loaded " << shapes.size() << " " << shapeName << "." << std::endl;

    // Build index and measure indexing time.
    std::cout << "Building STRtree... " << std::flush;

    STRtree index;

    auto start = std::chrono::high_resolution_clock::now();

    for (auto &shape : geosShapes)
    {
        index.insert(shape->getEnvelopeInternal(), nullptr);
    }

    index.build();

    auto end = std::chrono::high_resolution_clock::now();

    // Report.
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Finished. Built index of size " << 0 << " bytes in " << duration << " ms." << std::endl;

    return 0;
}