#pragma once
#include <vector>
#include <gperftools/heap-profiler.h>
#include "s2/s2point.h"
#include "s2/s2point_vector_shape.h"
#include "s2/mutable_s2shape_index.h"
#include "common.h"
#include "../utils/progress.h"

std::shared_ptr<MutableS2ShapeIndex> build_s2shapeindex(std::unique_ptr<S2PointVectorShape> points)
{
    auto index = std::make_unique<MutableS2ShapeIndex>();

    ProgressBar progress(2);
    progress.start();

    index->Add(std::move(points));

    progress.update(1);

    index->ForceBuild();

    progress.finish();

    return index;
}

void run_distance_queries(std::shared_ptr<MutableS2ShapeIndex> index, const std::vector<DistanceQuery> &dqueries)
{
    ProgressBar progress(dqueries.size());
    progress.start();

    S2ClosestEdgeQuery query(index.get());

    for (size_t i = 0; i < dqueries.size(); i++)
    {
        auto coord = dqueries[i].coord;
        auto point = S2LatLng::FromDegrees(coord.lat, coord.lon);

        S2Cell cell(point);
        S2ClosestEdgeQuery::CellTarget target(cell);

        query.mutable_options()->set_max_distance(S2Earth::ToAngle(util::units::Meters(dqueries[i].distance)));
        query.FindClosestEdges(&target);

        progress.update(i);
    }

    progress.finish();
}

void run_range_queries(std::shared_ptr<MutableS2ShapeIndex> index, std::vector<RangeQuery> &rqueries)
{
    // ProgressBar progress(rqueries.size());
    // progress.start();

    // S2BooleanOperation query(S2BooleanOperation::OpType::INTERSECTION);
    // auto query = MakeS2ContainsPointQuery(index.get());

    // for (size_t i = 0; i < rqueries.size(); i++)
    // {
    //     auto coordA = rqueries[i].a;
    //     auto coordB = rqueries[i].b;

    //     S2LatLngRect rect(S2LatLng::FromDegrees(coordA.lat, coordA.lon), S2LatLng::FromDegrees(coordB.lat, coordB.lon));

    // }

    // progress.finish();
}

void benchmark_s2shapeindex(std::unique_ptr<S2PointVectorShape> points, std::vector<DistanceQuery> &dqueries, std::vector<RangeQuery> &rqueries)
{
    std::cout << "================" << std::endl
              << "= S2ShapeIndex =" << std::endl
              << "================" << std::endl
              << std::endl;

    std::cout << "Building index..." << std::endl;

    HeapProfilerStart("heapprofile/s2shapeindex");
    auto index = build_s2shapeindex(std::move(points));
    HeapProfilerStop();

    std::cout << "Finished. Running " << dqueries.size() << " distance queries..." << std::endl;

    run_distance_queries(index, dqueries);

    std::cout << "Finished. Running " << rqueries.size() << " range queries..." << std::endl;

    run_range_queries(index, rqueries);

    std::cout << "Finished." << std::endl;
}