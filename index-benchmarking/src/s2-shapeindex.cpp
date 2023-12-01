#include <memory>
#include <chrono>
#include "s2/s2point.h"
#include "s2/s2point_vector_shape.h"
#include "s2/s2polyline.h"
#include "s2/s2polygon.h"
#include "s2/mutable_s2shape_index.h"
#include "s2/s2contains_point_query.h"
#include "gdal-utils.h"
#include "s2/s2closest_point_query.h"
#include "s2/s2earth.h"

// std::vector<S2Point> getPoints(SHPObject_ptr obj)
// {
//     std::vector<S2Point> points;
//     for (int i = 0; i < obj->nVertices; i++)
//     {
//         points.push_back(S2LatLng::FromDegrees(obj->padfY[i], obj->padfX[i]).ToPoint());
//     }
//     return points;
// }

// std::unique_ptr<S2Shape> makeS2Point(SHPObject_ptr obj)
// {
//     auto points = getPoints(std::move(obj));
//     return std::make_unique<S2PointVectorShape>(points);
// }

// std::unique_ptr<S2Shape> makeS2Polyline(SHPObject_ptr obj)
// {
//     auto points = getPoints(std::move(obj));
//     auto polyline = new S2Polyline(points);
//     return std::make_unique<S2Polyline::Shape>(polyline);
// }

// std::unique_ptr<S2Shape> makeS2Polygon(SHPObject_ptr obj)
// {
//     auto points = getPoints(std::move(obj));
//     points.pop_back();
//     auto polygon = new S2Polygon(std::make_unique<S2Loop>(points));
//     return std::make_unique<S2Polygon::Shape>(polygon);
// }

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

    // MutableS2ShapeIndex is lazy, and will only build the index when a query is performed.
    // S2ContainsPointQueryOptions options(S2VertexModel::CLOSED);
    // MakeS2ContainsPointQuery(&index, options).Contains(S2LatLng::FromDegrees(0, 0).ToPoint());
    S2ClosestPointQuery<int> query(&index);
    query.mutable_options()->set_max_distance(
        S1Angle::Radians(S2Earth::KmToRadians(1)));

    auto end = std::chrono::high_resolution_clock::now();

    // Report.
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Finished. Built index of size " << index.SpaceUsed() << " bytes in " << duration << " ms." << std::endl;

    return 0;
}
