#pragma once
#include <vector>
#include <memory>
#include <gperftools/heap-profiler.h>
#include "geos/index/strtree/STRtree.h"
#include "common.h"
#include "../utils/progress.h"

std::shared_ptr<geos::index::strtree::STRtree> build_strtree(const std::vector<std::unique_ptr<geos::geom::Point>> &points)
{
    auto index = new geos::index::strtree::STRtree();

    for (int i = 0; i < points.size(); i++)
    {
        index->insert(points[i]->getEnvelopeInternal(), nullptr);
        progress.update(i);
    }

    progress.finish();

    return std::make_unique<geos::index::strtree::STRtree>(index);
}

void run_distance_queries(std::shared_ptr<geos::index::strtree::STRtree> index, std::vector<DistanceQuery> &dqueries)
{
    ProgressTracker progress(dqueries.size());
    progress.start();

    for (size_t i = 0; i < dqueries.size(); i++)
    {
        auto coord = dqueries[i].coord;
        auto distance = dqueries[i].distance;
        geos::geom::Envelope rectangle(geos::geom::Coordinate(coord.lat - distance, y - distance),
                                       geos::geom::Coordinate(x + distance, y + distance));

        std::vector<geos::geom::Geometry *> result;
        result.reserve(1e8);
        std::vector<void *> candidates;
        index.query(target_range->getEnvelopeInternal(), candidates);

        for (auto &candidate : candidates)
        {
            const auto &point = static_cast<geos::geom::Geometry *>(candidate);
            if (point->isWithinDistance(target_point, distance))
                result.push_back(point);
        }

        uint64_t res_size = result.size();
        result.clear();

        progress.update(i);
    }

    progress.finish();
}

void run_range_queries(std::shared_ptr<geos::index::strtree::STRtree> index, std::vector<RangeQuery> &rqueries)
{
}

void benchmark_strtree(std::vector<std::unique_ptr<geos::geom::Point>> &points, std::vector<DistanceQuery> &dqueries, std::vector<RangeQuery> &rqueries)
{
    std::cout << "Running STRtree benchmark..." << std::endl;

    HeapProfilerStart("heapprofile/strtree");
    auto index = build_strtree(points);
    HeapProfilerStop();

    std::cout << "Finished. Running " << dqueries.size() << " distance queries..." << std::endl;

    run_distance_queries(index, dqueries);

    std::cout << "Finished. Running " << rqueries.size() << " range queries..." << std::endl;

    run_range_queries(index, rqueries);

    std::cout << "Finished." << std::endl;
}