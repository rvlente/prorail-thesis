#include <memory>
#include <chrono>
#include "s2/s2point.h"
#include "s2/s2point_vector_shape.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "s2/mutable_s2shape_index.h"
#include "shp-utils.h"

void addToIndex(MutableS2ShapeIndex *index, const SHPObject *obj, ShapeType shapeType)
{
    std::vector<S2Point> points;

    for (int i = 0; i < obj->nVertices; i++)
    {
        points.push_back(S2LatLng::FromDegrees(obj->padfY[i], obj->padfX[i]).ToPoint());
    }

    switch (shapeType)
    {
    case ShapeType::Point:
    {
        index->Add(std::make_unique<S2PointVectorShape>(points));
        break;
    }
    case ShapeType::Line:
    {
        auto polyline = new S2Polyline(points);
        index->Add(std::make_unique<S2Polyline::Shape>(polyline));
        break;
    }
    case ShapeType::Polygon:
    {
        points.pop_back();
        auto polygon = new S2Polygon(std::make_unique<S2Loop>(points));
        index->Add(std::make_unique<S2Polygon::Shape>(polygon));
        break;
    }
    }
}

std::unique_ptr<MutableS2ShapeIndex> buildIndex(const char *shapeFile)
{
    auto index = std::make_unique<MutableS2ShapeIndex>();
    processShapes(shapeFile, std::bind(addToIndex, index.get(), std::placeholders::_1, std::placeholders::_2));
    return index;
}

int main(int argc, char **argv)
{
    // Build index and measure indexing time.
    std::cout << "Building S2ShapeIndex... " << std::flush;

    auto start = std::chrono::high_resolution_clock::now();
    auto index = buildIndex("../../notebooks/shape/polygons/polygons");
    auto end = std::chrono::high_resolution_clock::now();

    // Report.
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Finished. Built index of size " << index->SpaceUsed() << " bytes in " << duration << " ms." << std::endl;

    return 0;
}
