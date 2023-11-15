#include <memory>
#include <chrono>
#include "s2/s2point.h"
#include "s2/s2point_vector_shape.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "s2/mutable_s2shape_index.h"
#include "s2/s2contains_point_query.h"
#include "shp-utils.h"

std::vector<S2Point> getPoints(SHPObject_ptr obj)
{
    std::vector<S2Point> points;
    for (int i = 0; i < obj->nVertices; i++)
    {
        points.push_back(S2LatLng::FromDegrees(obj->padfY[i], obj->padfX[i]).ToPoint());
    }
    return points;
}

std::unique_ptr<S2Shape> makeS2Point(SHPObject_ptr obj)
{
    auto points = getPoints(std::move(obj));
    return std::make_unique<S2PointVectorShape>(points);
}

std::unique_ptr<S2Shape> makeS2Polyline(SHPObject_ptr obj)
{
    auto points = getPoints(std::move(obj));
    auto polyline = new S2Polyline(points);
    return std::make_unique<S2Polyline::Shape>(polyline);
}

std::unique_ptr<S2Shape> makeS2Polygon(SHPObject_ptr obj)
{
    auto points = getPoints(std::move(obj));
    points.pop_back();
    auto polygon = new S2Polygon(std::make_unique<S2Loop>(points));
    return std::make_unique<S2Polygon::Shape>(polygon);
}

int main(int argc, char **argv)
{
    std::cout << "Loading shapes... " << std::flush;

    // Read shapes from shapefile.
    ShapeType shapeType;
    auto shapes = loadShapefile("../../notebooks/shape/lines/lines", &shapeType);

    // Convert shapes to S2 shapes.
    std::vector<std::unique_ptr<S2Shape>> s2shapes;

    std::function<std::unique_ptr<S2Shape>(SHPObject_ptr)> makeS2Shape;
    std::string shapeName;

    switch (shapeType)
    {
    case ShapeType::Point:
        makeS2Shape = makeS2Point;
        shapeName = "points";
        break;
    case ShapeType::Line:
        makeS2Shape = makeS2Polyline;
        shapeName = "lines";
        break;
    case ShapeType::Polygon:
        makeS2Shape = makeS2Polygon;
        shapeName = "polygons";
        break;
    }

    for (auto &shape : shapes)
    {
        s2shapes.push_back(makeS2Shape(std::move(shape)));
    }

    std::cout << "Finished. Loaded " << shapes.size() << " " << shapeName << "." << std::endl;

    // Build index and measure indexing time.
    std::cout << "Building S2ShapeIndex... " << std::flush;

    MutableS2ShapeIndex index;

    S2ContainsPointQueryOptions options(S2VertexModel::CLOSED);

    auto start = std::chrono::high_resolution_clock::now();

    for (auto &shape : s2shapes)
    {
        index.Add(std::move(shape));
    }

    // MutableS2ShapeIndex is lazy, and will only build the index when a query is performed.
    MakeS2ContainsPointQuery(&index, options).Contains(S2LatLng::FromDegrees(0, 0).ToPoint());

    auto end = std::chrono::high_resolution_clock::now();

    // Report.
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Finished. Built index of size " << index.SpaceUsed() << " bytes in " << duration << " ms." << std::endl;

    return 0;
}
