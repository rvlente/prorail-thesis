#pragma once
#include <vector>
#include <gperftools/heap-profiler.h>
#include "s2/s2point.h"
#include "s2/s2point_index.h"
#include "s2/s2closest_point_query.h"
#include "s2/s2earth.h"
#include "../utils/progress.h"
#include "common.h"

std::shared_ptr<S2PointIndex<int>> build_s2pointindex(const S2PointVectorShape *points)
{
    auto index = std::make_shared<S2PointIndex<int>>();
    ProgressBar progress(points->num_points());
    progress.start();

    for (int i = 0; i < points->num_points(); i++)
    {
        index->Add(points->point(i), i);
        progress.update(i);
    }

    progress.finish();

    return index;
}

void run_distance_queries(std::shared_ptr<S2PointIndex<int>> index, std::vector<DistanceQuery> &dqueries)
{
    ProgressBar progress(dqueries.size());
    progress.start();

    for (size_t i = 0; i < dqueries.size(); i++)
    {
        auto coord = dqueries[i].coord;
        auto point = S2LatLng::FromDegrees(coord.lat, coord.lon).ToPoint();

        S2ClosestPointQuery<int> query(index.get());
        S2ClosestPointQueryPointTarget target(point);

        query.mutable_options()->set_max_distance(S2Earth::ToAngle(util::units::Meters(dqueries[i].distance)));
        query.FindClosestPoints(&target);

        progress.update(i);
    }

    progress.finish();
}

void run_range_queries(std::shared_ptr<S2PointIndex<int>> index, std::vector<RangeQuery> &rqueries)
{
    ProgressBar progress(rqueries.size());
    progress.start();

    for (size_t i = 0; i < rqueries.size(); i++)
    {
        auto coordA = rqueries[i].a;
        auto coordB = rqueries[i].b;

        S2LatLngRect rect(S2LatLng::FromDegrees(coordA.lat, coordA.lon), S2LatLng::FromDegrees(coordB.lat, coordB.lon));

        S2ClosestPointQuery<int> query(index.get());
        S2ClosestPointQueryPointTarget target(rect.GetCenter().ToPoint());

        query.mutable_options()->set_region(&rect);
        query.FindClosestPoints(&target);

        progress.update(i);
    }

    progress.finish();
}

void benchmark_s2pointindex(const S2PointVectorShape *points, std::vector<DistanceQuery> &dqueries, std::vector<RangeQuery> &rqueries)
{
    std::cout << "================" << std::endl
              << "= S2PointIndex =" << std::endl
              << "================" << std::endl
              << std::endl;

    std::cout << "Building index..." << std::endl;

    HeapProfilerStart("heapprofile/strtree");
    auto index = build_s2pointindex(points);
    HeapProfilerStop();

    std::cout << "Finished. Running " << dqueries.size() << " distance queries..." << std::endl;

    run_distance_queries(index, dqueries);

    std::cout << "Finished. Running " << rqueries.size() << " range queries..." << std::endl;

    run_range_queries(index, rqueries);

    std::cout << "Finished." << std::endl;
}